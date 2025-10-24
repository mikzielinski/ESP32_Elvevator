#!/usr/bin/env python3
"""
Test końcowy sterownika DRI0050 z poprawionym CRC
"""

import serial
import time
import sys

def test_motor_final():
    print("🔍 Test końcowy sterownika DRI0050 z poprawionym CRC...")
    
    try:
        # Otwórz port sterownika
        ser = serial.Serial('/dev/cu.usbmodem5A671675611', 9600, timeout=1)
        time.sleep(2)
        
        print("✅ Połączono ze sterownikiem!")
        
        print("\n🚀 Test 1: Sprawdzenie czy sterownik odpowiada na ModBus RTU")
        
        # Test 1: Odczyt PID (powinien zwrócić 0xC032)
        cmd = bytes([0x32, 0x03, 0x00, 0x00, 0x00, 0x01, 0x81, 0xC9])
        ser.write(cmd)
        print(f"📤 Read PID: {' '.join(f'{b:02x}' for b in cmd)}")
        time.sleep(0.5)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            
            # Sprawdź czy odpowiedź jest prawidłowa
            if len(response) >= 7 and response[0] == 0x32 and response[1] == 0x03:
                print("✅ Sterownik odpowiada na ModBus RTU!")
                print(f"🎯 PID: 0x{response[3]:02x}{response[4]:02x}")
            else:
                print("❌ Sterownik nie odpowiada na ModBus RTU!")
                print("💡 Sprawdź przełącznik trybu na sterowniku!")
        else:
            print("❌ Brak odpowiedzi!")
            print("💡 Sprawdź przełącznik trybu na sterowniku!")
        
        print("\n🚀 Test 2: Inicjalizacja PWM")
        
        # Test 2: Ustawienie częstotliwości PWM
        freq_cmd = bytes([0x32, 0x06, 0x00, 0x07, 0x00, 0x0A, 0x48, 0x2C])
        ser.write(freq_cmd)
        print(f"📤 Set PWM Frequency: {' '.join(f'{b:02x}' for b in freq_cmd)}")
        time.sleep(0.5)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
        
        # Test 3: Ustawienie PWM Duty na 0
        duty_cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0x00, 0x6C, 0x08])
        ser.write(duty_cmd)
        print(f"📤 Set PWM Duty = 0: {' '.join(f'{b:02x}' for b in duty_cmd)}")
        time.sleep(0.5)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
        
        # Test 4: Włączenie PWM
        enable_cmd = bytes([0x32, 0x06, 0x00, 0x08, 0x00, 0x01, 0x48, 0x2C])
        ser.write(enable_cmd)
        print(f"📤 Enable PWM: {' '.join(f'{b:02x}' for b in enable_cmd)}")
        time.sleep(0.5)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
        
        print("\n🚀 Test 3: Ustawienie PWM Duty na 255 (100%)")
        
        # Test 5: Ustawienie PWM Duty na 255
        duty_cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0xFF, 0x48, 0x2C])
        ser.write(duty_cmd)
        print(f"📤 Set PWM Duty = 255: {' '.join(f'{b:02x}' for b in duty_cmd)}")
        print("🎯 Sprawdź czy silnik się obraca!")
        time.sleep(2)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
        
        print("\n🚀 Test 4: Zatrzymanie silnika")
        
        # Test 6: Zatrzymanie silnika
        stop_cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0x00, 0x6C, 0x08])
        ser.write(stop_cmd)
        print(f"📤 Set PWM Duty = 0: {' '.join(f'{b:02x}' for b in stop_cmd)}")
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
    test_motor_final()
