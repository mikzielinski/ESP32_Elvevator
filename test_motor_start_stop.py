#!/usr/bin/env python3
"""
Test sygnału START/STOP dla sterownika DRI0050
"""

import serial
import time
import sys

def test_motor_start_stop():
    print("🔍 Test sygnału START/STOP dla sterownika DRI0050...")
    
    try:
        # Otwórz port sterownika
        ser = serial.Serial('/dev/cu.usbmodem5A671675611', 9600, timeout=1)
        time.sleep(2)
        
        print("✅ Połączono ze sterownikiem!")
        
        print("\n🚀 Test 1: Inicjalizacja PWM")
        
        # Test 1: Ustawienie częstotliwości PWM
        freq_cmd = bytes([0x32, 0x06, 0x00, 0x07, 0x00, 0x0A, 0x48, 0x2C])  # Freq = 10
        ser.write(freq_cmd)
        print(f"📤 Częstotliwość PWM: {' '.join(f'{b:02x}' for b in freq_cmd)}")
        time.sleep(0.5)
        
        # Test 2: Ustawienie PWM Duty na 0
        duty_cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0x00, 0x6C, 0x08])  # Duty = 0
        ser.write(duty_cmd)
        print(f"📤 PWM Duty = 0: {' '.join(f'{b:02x}' for b in duty_cmd)}")
        time.sleep(0.5)
        
        # Test 3: Włączenie PWM
        enable_cmd = bytes([0x32, 0x06, 0x00, 0x08, 0x00, 0x01, 0x48, 0x2C])  # Enable = 1
        ser.write(enable_cmd)
        print(f"📤 PWM Enable = 1: {' '.join(f'{b:02x}' for b in enable_cmd)}")
        time.sleep(0.5)
        
        print("\n🚀 Test 2: Sprawdzenie rejestrów sterownika")
        
        # Test 4: Odczyt rejestru PWM Duty
        read_duty_cmd = bytes([0x32, 0x03, 0x00, 0x06, 0x00, 0x01, 0x64, 0x0B])  # Read register 0x0006
        ser.write(read_duty_cmd)
        print(f"📤 Read PWM Duty: {' '.join(f'{b:02x}' for b in read_duty_cmd)}")
        time.sleep(0.5)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź PWM Duty: {' '.join(f'{b:02x}' for b in response)}")
        
        # Test 5: Odczyt rejestru PWM Enable
        read_enable_cmd = bytes([0x32, 0x03, 0x00, 0x08, 0x00, 0x01, 0xE4, 0x0B])  # Read register 0x0008
        ser.write(read_enable_cmd)
        print(f"📤 Read PWM Enable: {' '.join(f'{b:02x}' for b in read_enable_cmd)}")
        time.sleep(0.5)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź PWM Enable: {' '.join(f'{b:02x}' for b in response)}")
        
        print("\n🚀 Test 3: Ustawienie PWM Duty na 255 (100%)")
        
        # Test 6: Ustawienie PWM Duty na 255
        duty_cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0xFF, 0x48, 0x2C])  # Duty = 255
        ser.write(duty_cmd)
        print(f"📤 PWM Duty = 255: {' '.join(f'{b:02x}' for b in duty_cmd)}")
        print("🎯 Sprawdź czy silnik się obraca!")
        time.sleep(2)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
        
        print("\n🚀 Test 4: Sprawdzenie czy PWM jest aktywny")
        
        # Test 7: Ponowny odczyt PWM Duty
        read_duty_cmd = bytes([0x32, 0x03, 0x00, 0x06, 0x00, 0x01, 0x64, 0x0B])
        ser.write(read_duty_cmd)
        print(f"📤 Read PWM Duty: {' '.join(f'{b:02x}' for b in read_duty_cmd)}")
        time.sleep(0.5)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź PWM Duty: {' '.join(f'{b:02x}' for b in response)}")
            
            # Analizuj odpowiedź
            if len(response) >= 7:
                duty_value = response[3] * 256 + response[4]
                print(f"🎯 PWM Duty Value: {duty_value}/255 ({duty_value/255*100:.1f}%)")
        
        print("\n🚀 Test 5: Zatrzymanie silnika")
        
        # Test 8: Zatrzymanie silnika
        stop_cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0x00, 0x6C, 0x08])  # Duty = 0
        ser.write(stop_cmd)
        print(f"📤 PWM Duty = 0: {' '.join(f'{b:02x}' for b in stop_cmd)}")
        print("🛑 Silnik powinien się zatrzymać!")
        time.sleep(1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
        
        ser.close()
        print("\n✅ Test zakończony!")
        
    except Exception as e:
        print(f"❌ Błąd: {e}")

if __name__ == "__main__":
    test_motor_start_stop()
