#!/usr/bin/env python3
"""
Test czy sterownik DRI0050 odpowiada na komendy ModBus RTU
"""

import serial
import time
import sys

def test_motor_response():
    print("🔍 Test odpowiedzi sterownika DRI0050...")
    
    # Znajdź port sterownika (nie ESP32)
    ports = ['/dev/cu.usbmodem5A671675611', '/dev/cu.usbserial-210']
    
    for port in ports:
        try:
            print(f"📡 Testowanie portu: {port}")
            
            # Otwórz port sterownika
            ser = serial.Serial(port, 9600, timeout=1)
            time.sleep(2)
            
            # Wyczyść bufor
            ser.flushInput()
            ser.flushOutput()
            
            print("📤 Wysyłanie komendy ModBus RTU...")
            
            # Komenda ModBus RTU: Set PWM Duty to 255 (100%)
            cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0xFF, 0x2C, 0x48])
            ser.write(cmd)
            print(f"📤 Wysłano: {' '.join(f'{b:02x}' for b in cmd)}")
            
            # Czekaj na odpowiedź
            time.sleep(0.5)
            
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting)
                print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
                
                if len(response) >= 8:
                    print("✅ Sterownik odpowiada!")
                    return True
                else:
                    print("⚠️  Krótka odpowiedź")
            else:
                print("❌ Brak odpowiedzi")
            
            ser.close()
            
        except Exception as e:
            print(f"❌ Błąd na porcie {port}: {e}")
            continue
    
    print("❌ Żaden port nie odpowiada")
    return False

if __name__ == "__main__":
    test_motor_response()
