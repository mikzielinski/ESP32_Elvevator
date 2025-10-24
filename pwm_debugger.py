#!/usr/bin/env python3
"""
PWM Debugger - Wizualizator sygnału PWM z ESP32
Sprawdza czy ESP32 wysyła prawidłowe sygnały PWM
"""

import serial
import time
import sys
import matplotlib.pyplot as plt
import numpy as np
from collections import deque
import threading

class PWMDebugger:
    def __init__(self, port='/dev/cu.usbmodem11101', baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.ser = None
        self.running = False
        self.data_buffer = deque(maxlen=1000)
        self.pwm_data = []
        
    def connect(self):
        """Połącz z ESP32"""
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=1)
            time.sleep(2)
            print(f"✅ Połączono z ESP32 na porcie {self.port}")
            return True
        except Exception as e:
            print(f"❌ Błąd połączenia: {e}")
            return False
    
    def read_serial_data(self):
        """Czytaj dane z ESP32"""
        if not self.ser:
            return
            
        try:
            while self.running:
                if self.ser.in_waiting > 0:
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        self.data_buffer.append({
                            'timestamp': time.time(),
                            'data': line
                        })
                        print(f"📡 {line}")
                time.sleep(0.01)
        except Exception as e:
            print(f"❌ Błąd czytania: {e}")
    
    def analyze_pwm_commands(self):
        """Analizuj komendy PWM z logów"""
        pwm_commands = []
        
        for entry in self.data_buffer:
            if 'MOTOR_CONTROL' in entry['data'] and 'Sending' in entry['data']:
                # Parsuj komendę ModBus RTU
                parts = entry['data'].split(':')
                if len(parts) > 1:
                    hex_data = parts[1].strip()
                    hex_bytes = hex_data.split()
                    if len(hex_bytes) >= 8:
                        try:
                            cmd = [int(b, 16) for b in hex_bytes[:8]]
                            pwm_commands.append({
                                'timestamp': entry['timestamp'],
                                'command': cmd,
                                'hex': hex_data
                            })
                        except ValueError:
                            continue
        
        return pwm_commands
    
    def visualize_pwm_signal(self):
        """Wizualizuj sygnał PWM"""
        pwm_commands = self.analyze_pwm_commands()
        
        if not pwm_commands:
            print("❌ Brak komend PWM do wizualizacji")
            return
        
        print(f"📊 Znaleziono {len(pwm_commands)} komend PWM")
        
        # Analizuj komendy
        for i, cmd in enumerate(pwm_commands):
            print(f"\n🔍 Komenda {i+1}:")
            print(f"   Czas: {time.ctime(cmd['timestamp'])}")
            print(f"   Hex: {cmd['hex']}")
            print(f"   Bajty: {[f'0x{b:02x}' for b in cmd['command']]}")
            
            # Analizuj strukturę ModBus RTU
            if len(cmd['command']) >= 8:
                addr = cmd['command'][0]
                func = cmd['command'][1]
                reg_hi = cmd['command'][2]
                reg_lo = cmd['command'][3]
                val_hi = cmd['command'][4]
                val_lo = cmd['command'][5]
                crc_hi = cmd['command'][6]
                crc_lo = cmd['command'][7]
                
                print(f"   Adres: 0x{addr:02x} ({addr})")
                print(f"   Funkcja: 0x{func:02x} ({func})")
                print(f"   Rejestr: 0x{reg_hi:02x}{reg_lo:02x} ({reg_hi*256 + reg_lo})")
                print(f"   Wartość: 0x{val_hi:02x}{val_lo:02x} ({val_hi*256 + val_lo})")
                print(f"   CRC: 0x{crc_hi:02x}{crc_lo:02x}")
                
                # Sprawdź czy to komenda PWM
                if reg_hi == 0x00 and reg_lo == 0x06:  # PWM Duty register
                    duty = val_lo
                    print(f"   🎯 PWM Duty: {duty}/255 ({duty/255*100:.1f}%)")
                elif reg_hi == 0x00 and reg_lo == 0x08:  # PWM Enable register
                    enable = val_lo
                    print(f"   🎯 PWM Enable: {enable}")
    
    def test_motor_commands(self):
        """Test komend silnika"""
        print("\n🚀 Test komend silnika...")
        print("Kliknij przyciski na dashboard i obserwuj logi!")
        
        # Czekaj na komendy
        start_time = time.time()
        while time.time() - start_time < 30:  # 30 sekund
            pwm_commands = self.analyze_pwm_commands()
            if pwm_commands:
                print(f"📊 Znaleziono {len(pwm_commands)} komend PWM")
                self.visualize_pwm_signal()
                break
            time.sleep(1)
        
        if not pwm_commands:
            print("❌ Brak komend PWM w ciągu 30 sekund!")
            print("💡 Sprawdź czy dashboard działa i czy przyciski są klikane!")
    
    def run(self):
        """Uruchom debugger"""
        if not self.connect():
            return
        
        self.running = True
        
        # Uruchom wątek czytania danych
        read_thread = threading.Thread(target=self.read_serial_data)
        read_thread.daemon = True
        read_thread.start()
        
        print("\n🔍 PWM Debugger uruchomiony!")
        print("📡 Czytam dane z ESP32...")
        print("💡 Kliknij przyciski na dashboard i obserwuj logi!")
        print("⏰ Czekam 30 sekund na komendy PWM...")
        
        try:
            self.test_motor_commands()
        except KeyboardInterrupt:
            print("\n⏹️  Zatrzymywanie debuggera...")
        finally:
            self.running = False
            if self.ser:
                self.ser.close()
            print("✅ Debugger zatrzymany")

if __name__ == "__main__":
    debugger = PWMDebugger()
    debugger.run()
