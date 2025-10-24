#!/usr/bin/env python3
"""
Test czy ESP32 komunikuje się ze sterownikiem
"""

import serial
import time
import sys

def test_esp32_motor():
    print("🔍 Test komunikacji ESP32 → Sterownik...")
    
    try:
        # Otwórz port ESP32
        ser = serial.Serial('/dev/cu.usbmodem11101', 115200, timeout=1)
        time.sleep(2)
        
        print("📡 Podłączony do ESP32")
        print("📤 Wysyłanie komendy przez ESP32...")
        
        # Wyślij komendę przez ESP32 (symulacja kliknięcia przycisku)
        # To nie zadziała bezpośrednio, ale możemy sprawdzić logi
        
        print("🔍 Sprawdź logi ESP32 - czy komendy są wysyłane...")
        
        # Czekaj na logi
        time.sleep(5)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź ESP32: {response.decode('utf-8', errors='ignore')}")
        
        ser.close()
        
    except Exception as e:
        print(f"❌ Błąd: {e}")

if __name__ == "__main__":
    test_esp32_motor()
