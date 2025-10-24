#!/usr/bin/env python3
"""
Test sterownika DRI0050 według dokumentacji Windows
"""

import serial
import time
import sys

def test_motor_windows():
    print("🔍 Test sterownika DRI0050 według dokumentacji Windows...")
    
    try:
        # Otwórz port sterownika
        ser = serial.Serial('/dev/cu.usbmodem5A671675611', 9600, timeout=1)
        time.sleep(2)
        
        print("✅ Połączono ze sterownikiem!")
        
        print("\n🚀 Test 1: Sprawdzenie trybu sterownika")
        
        # Test 1: Sprawdzenie czy sterownik odpowiada
        test_cmd = bytes([0x32, 0x03, 0x00, 0x00, 0x00, 0x01, 0x81, 0xC9])  # Read PID
        ser.write(test_cmd)
        print(f"📤 Test komenda: {' '.join(f'{b:02x}' for b in test_cmd)}")
        time.sleep(0.5)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            
            # Sprawdź czy odpowiedź jest prawidłowa
            if len(response) >= 7 and response[0] == 0x32 and response[1] == 0x03:
                print("✅ Sterownik odpowiada na ModBus RTU!")
                return True
            else:
                print("❌ Sterownik nie odpowiada na ModBus RTU!")
                print("💡 Może sterownik jest w trybie analogowym?")
        else:
            print("❌ Brak odpowiedzi!")
            print("💡 Sprawdź przełącznik trybu na sterowniku!")
        
        print("\n🚀 Test 2: Sprawdzenie różnych trybów")
        
        # Test 2: Sprawdzenie czy sterownik ma przełącznik trybu
        print("🔍 Sprawdź na sterowniku:")
        print("   - Czy jest przełącznik UART/COM vs Analog?")
        print("   - Czy jest przełącznik protokołu?")
        print("   - Czy są jumpers lub przełączniki?")
        
        print("\n🚀 Test 3: Sprawdzenie połączeń")
        
        # Test 3: Sprawdzenie połączeń
        print("🔍 Sprawdź połączenia:")
        print("   - GPIO12 (ESP32) → TX (Sterownik)")
        print("   - GPIO11 (ESP32) → RX (Sterownik)")
        print("   - GND (ESP32) → GND (Sterownik)")
        print("   - Silnik → PWM OUT (Sterownik)")
        print("   - 12V → VIN (Sterownik)")
        
        print("\n🚀 Test 4: Sprawdzenie zasilania")
        
        # Test 4: Sprawdzenie zasilania
        print("🔍 Sprawdź zasilanie:")
        print("   - Sterownik: 5V (zasilanie)")
        print("   - Silnik: 12V (VIN)")
        print("   - ESP32: 3.3V (logika UART)")
        
        ser.close()
        print("\n✅ Test zakończony!")
        
    except Exception as e:
        print(f"❌ Błąd: {e}")

if __name__ == "__main__":
    test_motor_windows()
