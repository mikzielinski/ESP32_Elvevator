#!/usr/bin/env python3
"""
Test bezpośredni komend PWM do sterownika DRI0050
"""

import serial
import time
import sys

def test_motor_direct():
    print("🔍 Test bezpośredni komend PWM do sterownika DRI0050...")
    
    # Znajdź port sterownika
    ports = ['/dev/cu.usbserial-210', '/dev/cu.usbmodem5A671675611']
    
    for port in ports:
        try:
            print(f"📡 Testowanie portu: {port}")
            
            # Otwórz port sterownika
            ser = serial.Serial(port, 9600, timeout=1)
            time.sleep(2)
            
            print("✅ Połączono ze sterownikiem!")
            print("🚀 Test 1: Ustawienie częstotliwości PWM (rejestr 0x0007)")
            
            # Test 1: Ustawienie częstotliwości PWM
            freq_cmd = bytes([0x32, 0x06, 0x00, 0x07, 0x00, 0x0A, 0x48, 0x2C])  # Freq = 10
            ser.write(freq_cmd)
            print(f"📤 Wysłano: {' '.join(f'{b:02x}' for b in freq_cmd)}")
            time.sleep(0.5)
            
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting)
                print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            
            print("\n🚀 Test 2: Ustawienie PWM Duty na 0 (rejestr 0x0006)")
            
            # Test 2: Ustawienie PWM Duty na 0
            duty_cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0x00, 0x6C, 0x08])  # Duty = 0
            ser.write(duty_cmd)
            print(f"📤 Wysłano: {' '.join(f'{b:02x}' for b in duty_cmd)}")
            time.sleep(0.5)
            
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting)
                print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            
            print("\n🚀 Test 3: Włączenie PWM (rejestr 0x0008)")
            
            # Test 3: Włączenie PWM
            enable_cmd = bytes([0x32, 0x06, 0x00, 0x08, 0x00, 0x01, 0x48, 0x2C])  # Enable = 1
            ser.write(enable_cmd)
            print(f"📤 Wysłano: {' '.join(f'{b:02x}' for b in enable_cmd)}")
            time.sleep(0.5)
            
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting)
                print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            
            print("\n🚀 Test 4: Ustawienie PWM Duty na 255 (100%)")
            
            # Test 4: Ustawienie PWM Duty na 255 (100%)
            duty_cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0xFF, 0x48, 0x2C])  # Duty = 255
            ser.write(duty_cmd)
            print(f"📤 Wysłano: {' '.join(f'{b:02x}' for b in duty_cmd)}")
            print("🎯 Sprawdź czy silnik się obraca!")
            time.sleep(2)
            
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting)
                print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            
            print("\n🚀 Test 5: Zatrzymanie silnika (Duty = 0)")
            
            # Test 5: Zatrzymanie silnika
            stop_cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0x00, 0x6C, 0x08])  # Duty = 0
            ser.write(stop_cmd)
            print(f"📤 Wysłano: {' '.join(f'{b:02x}' for b in stop_cmd)}")
            print("🛑 Silnik powinien się zatrzymać!")
            time.sleep(1)
            
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting)
                print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            
            ser.close()
            print(f"\n✅ Test zakończony na porcie {port}")
            return True
            
        except Exception as e:
            print(f"❌ Błąd na porcie {port}: {e}")
            continue
    
    print("❌ Żaden port nie działa")
    return False

if __name__ == "__main__":
    test_motor_direct()