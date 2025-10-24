#include "web_server.h"
#include "wifi_config.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "hx711.h"
#include "motor_control.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

static const char *TAG = "WEB_SERVER";
static httpd_handle_t server = NULL;
static float current_weight = 0.0;
static long current_raw = 0;
static hx711_t* hx711_scale = NULL;

// Smart weight measurement system
static float stable_weight = 0.0;
static bool is_calculating = false;
static float weight_samples[50]; // 5 seconds * 10 samples per second
static int sample_count = 0;
static float last_stable_weight = 0.0;

// Motor control system
static bool motor_auto_mode = false;
static float weight_threshold = 3.0; // kg
static bool motor_was_triggered = false;

// Motor control API handlers
static esp_err_t motor_forward_handler(httpd_req_t *req)
{
    motor_start_forward();
    char json[64];
    snprintf(json, sizeof(json), "{\"status\":\"forward\",\"success\":true}");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t motor_backward_handler(httpd_req_t *req)
{
    motor_start_backward();
    char json[64];
    snprintf(json, sizeof(json), "{\"status\":\"backward\",\"success\":true}");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t motor_stop_handler(httpd_req_t *req)
{
    motor_stop();
    char json[64];
    snprintf(json, sizeof(json), "{\"status\":\"stopped\",\"success\":true}");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t motor_auto_handler(httpd_req_t *req)
{
    motor_auto_mode = !motor_auto_mode;
    char json[64];
    snprintf(json, sizeof(json), "{\"auto_mode\":%s,\"success\":true}", 
             motor_auto_mode ? "true" : "false");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t motor_threshold_handler(httpd_req_t *req)
{
    char buf[100];
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret > 0) {
        buf[ret] = '\0';
        // Simple JSON parsing for threshold
        char *threshold_str = strstr(buf, "\"threshold\":");
        if (threshold_str) {
            threshold_str += 12; // Skip "threshold":
            weight_threshold = atof(threshold_str);
            ESP_LOGI(TAG, "Weight threshold set to %.2f kg", weight_threshold);
        }
    }
    
    char json[64];
    snprintf(json, sizeof(json), "{\"threshold\":%.2f,\"success\":true}", weight_threshold);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// HTML Dashboard - embedded in firmware
static const char *html_dashboard = 
"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='UTF-8'>"
"<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
"<title>HX711 Load Cell Monitor</title>"
"<style>"
"body{font-family:Arial,sans-serif;margin:0;padding:20px;background:#f5f5f5}"
".container{max-width:800px;margin:0 auto;background:white;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);padding:20px}"
"h1{color:#333;text-align:center;margin-bottom:30px}"
".status{display:flex;justify-content:space-around;margin-bottom:30px}"
".metric{text-align:center;padding:20px;background:#f8f9fa;border-radius:8px;border:2px solid #e9ecef}"
".metric h3{margin:0 0 10px 0;color:#666;font-size:14px;text-transform:uppercase}"
".metric .value{font-size:32px;font-weight:bold;color:#007bff;margin:0}"
".chart-container{background:#f8f9fa;border-radius:8px;padding:20px;margin-bottom:20px}"
"canvas{border:1px solid #ddd;border-radius:4px;background:white}"
".controls{text-align:center}"
"button{background:#007bff;color:white;border:none;padding:12px 24px;border-radius:6px;cursor:pointer;margin:0 10px;font-size:16px}"
"button:hover{background:#0056b3}"
"button:disabled{background:#6c757d;cursor:not-allowed}"
"</style>"
"</head>"
"<body>"
"<div class='container'>"
"<h1>🔧 HX711 Load Cell Monitor</h1>"
"<div class='status'>"
"<div class='metric'>"
"<h3>Real-time Weight</h3>"
"<div class='value' id='weight'>0.00</div>"
"</div>"
"<div class='metric'>"
"<h3>Stable Weight</h3>"
"<div class='value' id='stable-weight'>0.00</div>"
"<div class='status' id='status' style='font-size:12px;color:#666;margin-top:5px;'></div>"
"</div>"
"<div class='metric'>"
"<h3>Raw Value</h3>"
"<div class='value' id='raw'>0</div>"
"</div>"
"</div>"
"<div class='status' style='margin-top:20px;'>"
"<div class='metric' style='background:#f8f9fa;border:2px solid #e9ecef;'>"
"<h3>🔧 HX711 Sensor</h3>"
"<div class='value' id='sensor-status' style='font-size:18px;color:#28a745;'>Ready</div>"
"</div>"
"<div class='metric' style='background:#f8f9fa;border:2px solid #e9ecef;'>"
"<h3>📡 ESP32 Status</h3>"
"<div class='value' id='esp-status' style='font-size:18px;color:#28a745;'>Online</div>"
"</div>"
"<div class='metric' style='background:#f8f9fa;border:2px solid #e9ecef;'>"
"<h3>🌐 WiFi</h3>"
"<div class='value' id='wifi-status' style='font-size:18px;color:#28a745;'>Connected</div>"
"</div>"
"</div>"
"<div class='chart-container'>"
"<canvas id='chart' width='760' height='200'></canvas>"
"</div>"
"<div class='controls'>"
"<button onclick='zeroScale()'>Zero Scale</button>"
"<button onclick='clearChart()'>Clear Chart</button>"
"</div>"
"<div class='status' style='margin-top:20px;'>"
"<div class='metric' style='background:#fff3cd;border:2px solid #ffeaa7;'>"
"<h3>🚀 Motor Control</h3>"
"<div class='value' id='motor-status' style='font-size:18px;color:#856404;'>Stopped</div>"
"<div style='margin-top:10px;'>"
"<button onclick='motorForward()' id='btn-forward'>Forward</button>"
"<button onclick='motorBackward()' id='btn-backward'>Backward</button>"
"<button onclick='motorStop()' id='btn-stop'>Stop</button>"
"</div>"
"</div>"
"<div class='metric' style='background:#d1ecf1;border:2px solid #bee5eb;'>"
"<h3>⚙️ Auto Control</h3>"
"<div class='value' id='auto-status' style='font-size:18px;color:#0c5460;'>Manual</div>"
"<div style='margin-top:10px;'>"
"<button onclick='toggleAutoMode()' id='btn-auto'>Enable Auto</button>"
"<div style='margin-top:10px;'>"
"<label>Threshold: <input type='number' id='threshold' value='3.0' step='0.1' style='width:60px;'> kg</label>"
"<button onclick='setThreshold()'>Set</button>"
"</div>"
"</div>"
"</div>"
"</div>"
"</div>"
"<script>"
"let data=[];"
"let zeroOffset=0;"
"const canvas=document.getElementById('chart');"
"const ctx=canvas.getContext('2d');"
"function updateData(){"
"  fetch('/api/weight')"
"    .then(response=>response.json())"
"    .then(result=>{"
"      document.getElementById('weight').textContent=result.weight.toFixed(2);"
"      document.getElementById('raw').textContent=result.raw;"
"      if(result.status==='Stable'){"
"        document.getElementById('stable-weight').textContent=result.weight.toFixed(2);"
"        document.getElementById('status').textContent='Stable';"
"        document.getElementById('status').style.color='#28a745';"
"      }else if(result.status==='Calculating...'){"
"        document.getElementById('status').textContent='Calculating...';"
"        document.getElementById('status').style.color='#ff6b35';"
"      }"
"      if(result.sensor_ready){"
"        document.getElementById('sensor-status').textContent='Ready';"
"        document.getElementById('sensor-status').style.color='#28a745';"
"      }else{"
"        document.getElementById('sensor-status').textContent='Not Ready';"
"        document.getElementById('sensor-status').style.color='#dc3545';"
"      }"
"      document.getElementById('esp-status').textContent='Online';"
"      document.getElementById('esp-status').style.color='#28a745';"
"      document.getElementById('wifi-status').textContent='Connected';"
"      document.getElementById('wifi-status').style.color='#28a745';"
"      data.push({weight:result.weight,time:Date.now()});"
"      if(data.length>100)data.shift();"
"      drawChart();"
"    })"
"    .catch(error=>{"
"      console.error('Error:',error);"
"      document.getElementById('esp-status').textContent='Offline';"
"      document.getElementById('esp-status').style.color='#dc3545';"
"      document.getElementById('wifi-status').textContent='Disconnected';"
"      document.getElementById('wifi-status').style.color='#dc3545';"
"    });"
"}"
"function drawChart(){"
"const w=canvas.width;const h=canvas.height;"
"ctx.clearRect(0,0,w,h);"
"if(data.length<2)return;"
"const maxW=Math.max(...data.map(d=>d.weight),1);"
"const minW=Math.min(...data.map(d=>d.weight),0);"
"const range=maxW-minW||1;"
"ctx.beginPath();"
"ctx.strokeStyle='#007bff';"
"ctx.lineWidth=2;"
"data.forEach((d,i)=>{"
"const x=(i/(data.length-1))*w;"
"const y=h-(((d.weight-minW)/range)*h*0.8+h*0.1);"
"i===0?ctx.moveTo(x,y):ctx.lineTo(x,y);"
"});"
"ctx.stroke();"
"ctx.fillStyle='#666';"
"ctx.font='12px Arial';"
"ctx.fillText(maxW.toFixed(2)+' kg',5,15);"
"ctx.fillText(minW.toFixed(2)+' kg',5,h-5);"
"}"
"function clearChart(){data.length=0;drawChart();}"
"function zeroScale(){"
"  fetch('/api/zero',{method:'POST'})"
"    .then(response=>response.json())"
"    .then(result=>{"
"      console.log('Scale zeroed:',result);"
"      alert('Scale zeroed successfully!');"
"    })"
"    .catch(error=>{"
"      console.error('Error zeroing scale:',error);"
"      alert('Error zeroing scale');"
"    });"
"}"
"function motorForward(){"
"  fetch('/api/motor/forward',{method:'POST'})"
"    .then(response=>response.json())"
"    .then(result=>{"
"      console.log('Motor forward:',result);"
"      document.getElementById('motor-status').textContent='Forward';"
"      document.getElementById('motor-status').style.color='#28a745';"
"    })"
"    .catch(error=>{"
"      console.error('Error starting motor:',error);"
"    });"
"}"
"function motorBackward(){"
"  fetch('/api/motor/backward',{method:'POST'})"
"    .then(response=>response.json())"
"    .then(result=>{"
"      console.log('Motor backward:',result);"
"      document.getElementById('motor-status').textContent='Backward';"
"      document.getElementById('motor-status').style.color='#ffc107';"
"    })"
"    .catch(error=>{"
"      console.error('Error starting motor:',error);"
"    });"
"}"
"function motorStop(){"
"  fetch('/api/motor/stop',{method:'POST'})"
"    .then(response=>response.json())"
"    .then(result=>{"
"      console.log('Motor stop:',result);"
"      document.getElementById('motor-status').textContent='Stopped';"
"      document.getElementById('motor-status').style.color='#dc3545';"
"    })"
"    .catch(error=>{"
"      console.error('Error stopping motor:',error);"
"    });"
"}"
"function toggleAutoMode(){"
"  fetch('/api/motor/auto',{method:'POST'})"
"    .then(response=>response.json())"
"    .then(result=>{"
"      console.log('Auto mode:',result);"
"      if(result.auto_mode){"
"        document.getElementById('auto-status').textContent='Auto ON';"
"        document.getElementById('auto-status').style.color='#28a745';"
"        document.getElementById('btn-auto').textContent='Disable Auto';"
"      }else{"
"        document.getElementById('auto-status').textContent='Manual';"
"        document.getElementById('auto-status').style.color='#0c5460';"
"        document.getElementById('btn-auto').textContent='Enable Auto';"
"      }"
"    })"
"    .catch(error=>{"
"      console.error('Error toggling auto mode:',error);"
"    });"
"}"
"function setThreshold(){"
"  const threshold=document.getElementById('threshold').value;"
"  fetch('/api/motor/threshold',{"
"    method:'POST',"
"    headers:{'Content-Type':'application/json'},"
"    body:JSON.stringify({threshold:parseFloat(threshold)})"
"  })"
"    .then(response=>response.json())"
"    .then(result=>{"
"      console.log('Threshold set:',result);"
"      alert('Threshold set to '+threshold+' kg');"
"    })"
"    .catch(error=>{"
"      console.error('Error setting threshold:',error);"
"    });"
"}"
"window.onload=function(){"
"  updateData();"
"  setInterval(updateData,2000);"
"};"
"</script>"
"</body>"
"</html>";

// API handler for weight data
static esp_err_t weight_api_handler(httpd_req_t *req)
{
    // Check if HX711 is ready
    bool sensor_ready = false;
    if (hx711_scale != NULL) {
        sensor_ready = hx711_is_ready(hx711_scale);
    }
    
    // Get current weight and status
    char json[256];
    if (is_calculating) {
        snprintf(json, sizeof(json), "{\"weight\":%.2f,\"raw\":%ld,\"status\":\"Calculating...\",\"sensor_ready\":%s}", 
                 current_weight, current_raw, sensor_ready ? "true" : "false");
    } else {
        snprintf(json, sizeof(json), "{\"weight\":%.2f,\"raw\":%ld,\"status\":\"Stable\",\"sensor_ready\":%s}", 
                 current_weight, current_raw, sensor_ready ? "true" : "false");
    }
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// API handler for zeroing the scale
static esp_err_t zero_api_handler(httpd_req_t *req)
{
    if (req->method == HTTP_POST) {
        if (hx711_scale != NULL) {
            // Call the HX711 zero function
            hx711_zero_scale(hx711_scale);
            
            char json[64];
            snprintf(json, sizeof(json), "{\"status\":\"success\",\"message\":\"Scale zeroed\"}");
            
            httpd_resp_set_type(req, "application/json");
            httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
            
            ESP_LOGI(TAG, "Scale zeroed successfully");
            return ESP_OK;
        } else {
            char json[64];
            snprintf(json, sizeof(json), "{\"status\":\"error\",\"message\":\"HX711 not initialized\"}");
            
            httpd_resp_set_type(req, "application/json");
            httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
            
            ESP_LOGE(TAG, "HX711 not initialized for zeroing");
            return ESP_FAIL;
        }
    }
    
    httpd_resp_send_404(req);
    return ESP_FAIL;
}

// Root page handler
static esp_err_t root_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_dashboard, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void web_server_init(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = WEB_SERVER_PORT;
    
    ESP_LOGI(TAG, "Starting web server on port %d", config.server_port);
    
    if (httpd_start(&server, &config) == ESP_OK) {
        // Root page
        httpd_uri_t root = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &root);
        
        // API endpoint for weight data
        httpd_uri_t api = {
            .uri = "/api/weight",
            .method = HTTP_GET,
            .handler = weight_api_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &api);
        
        // API endpoint for zeroing the scale
        httpd_uri_t zero_api = {
            .uri = "/api/zero",
            .method = HTTP_POST,
            .handler = zero_api_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &zero_api);
        
        // Motor control API endpoints
        httpd_uri_t motor_forward = {
            .uri = "/api/motor/forward",
            .method = HTTP_POST,
            .handler = motor_forward_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &motor_forward);
        
        httpd_uri_t motor_backward = {
            .uri = "/api/motor/backward",
            .method = HTTP_POST,
            .handler = motor_backward_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &motor_backward);
        
        httpd_uri_t motor_stop = {
            .uri = "/api/motor/stop",
            .method = HTTP_POST,
            .handler = motor_stop_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &motor_stop);
        
        httpd_uri_t motor_auto = {
            .uri = "/api/motor/auto",
            .method = HTTP_POST,
            .handler = motor_auto_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &motor_auto);
        
        httpd_uri_t motor_threshold = {
            .uri = "/api/motor/threshold",
            .method = HTTP_POST,
            .handler = motor_threshold_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &motor_threshold);
        
        ESP_LOGI(TAG, "Web server started successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start web server");
    }
}

void web_server_send_weight(float weight_kg, long raw_value)
{
    // Store current values for API endpoint
    current_weight = weight_kg;
    current_raw = raw_value;
}

void web_server_set_hx711(hx711_t* hx711)
{
    hx711_scale = hx711;
    ESP_LOGI(TAG, "HX711 pointer set for web server");
}

// Smart weight measurement function
void web_server_process_weight(float weight_kg, long raw_value)
{
    static int64_t calculation_start_ms = 0;
    static bool first_run = true;
    
    // Update current values
    current_weight = weight_kg;
    current_raw = raw_value;
    
    // Initialize on first run
    if (first_run) {
        last_stable_weight = weight_kg;
        stable_weight = weight_kg;
        first_run = false;
        ESP_LOGI(TAG, "✓ Initial weight set: %.2f kg", weight_kg);
        return;
    }
    
    // Check for significant weight change (threshold: 0.13 kg = 130g)
    float weight_change = fabs(weight_kg - last_stable_weight);
    
    if (weight_change > 0.13 && !is_calculating) {
        // Start calculation process
        is_calculating = true;
        sample_count = 0;
        calculation_start_ms = esp_timer_get_time() / 1000; // Get current time in milliseconds
        ESP_LOGI(TAG, "⚡ Weight change detected: %.2f kg → %.2f kg (Δ=%.2f kg)", 
                 last_stable_weight, weight_kg, weight_change);
        ESP_LOGI(TAG, "⏱️  Starting 5-second measurement...");
    }
    
    if (is_calculating) {
        // Collect samples
        if (sample_count < 50) {
            weight_samples[sample_count] = weight_kg;
            sample_count++;
        }
        
        // Check if 5 seconds have passed
        int64_t current_ms = esp_timer_get_time() / 1000;
        int64_t elapsed_ms = current_ms - calculation_start_ms;
        
        if (elapsed_ms >= 5000) { // 5000 ms = 5 seconds
            // Calculate average weight
            float sum = 0.0;
            for (int i = 0; i < sample_count; i++) {
                sum += weight_samples[i];
            }
            stable_weight = sum / sample_count;
            last_stable_weight = stable_weight;
            
            // Reset calculation state
            is_calculating = false;
            int final_sample_count = sample_count;
            sample_count = 0;
            
            ESP_LOGI(TAG, "✅ Measurement complete: %.2f kg (average of %d samples over %.1f seconds)", 
                     stable_weight, final_sample_count, elapsed_ms / 1000.0);
            
            // Check motor auto control
            if (motor_auto_mode) {
                if (stable_weight >= weight_threshold && !motor_was_triggered) {
                    motor_start_forward();
                    motor_was_triggered = true;
                    ESP_LOGI(TAG, "🚀 Motor started - weight %.2f kg >= threshold %.2f kg", 
                             stable_weight, weight_threshold);
                } else if (stable_weight < weight_threshold && motor_was_triggered) {
                    motor_stop();
                    motor_was_triggered = false;
                    ESP_LOGI(TAG, "🛑 Motor stopped - weight %.2f kg < threshold %.2f kg", 
                             stable_weight, weight_threshold);
                }
            }
        } else {
            // Log progress every 10 samples
            if (sample_count % 10 == 0) {
                ESP_LOGI(TAG, "📊 Measuring... %d samples collected (%.1f seconds elapsed)", 
                         sample_count, elapsed_ms / 1000.0);
            }
        }
    }
}

// Motor control functions
void web_server_set_motor_auto_mode(bool enabled)
{
    motor_auto_mode = enabled;
}

bool web_server_get_motor_auto_mode(void)
{
    return motor_auto_mode;
}

float web_server_get_weight_threshold(void)
{
    return weight_threshold;
}

void web_server_set_weight_threshold(float threshold)
{
    weight_threshold = threshold;
}