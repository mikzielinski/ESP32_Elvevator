#!/usr/bin/env python3
"""
Deterministic smoke-test dla DRI0050 z dokładnymi ramkami
"""

import serial
import time
import sys

def crc16_modbus(data):
    """CRC calculation for ModBus RTU"""
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc

def test_motor_deterministic():
    print("🔍 Deterministic smoke-test dla DRI0050...")
    
    try:
        # Otwórz port sterownika
        ser = serial.Serial('/dev/cu.usbmodem5A671675611', 9600, timeout=1)
        time.sleep(2)
        
        print("✅ Połączono ze sterownikiem!")
        print("💡 Upewnij się, że USB-C jest odłączone od DRI0050!")
        print("💡 Przełącznik na UART/COM!")
        print("💡 Wspólna masa ESP32 i DRI0050!")
        
        print("\n🚀 Test 1: READ ENABLE (0x0008, 1 reg)")
        # READ ENABLE (0x0008, 1 reg) - 32 03 00 08 00 01 0B 00
        cmd = bytes([0x32, 0x03, 0x00, 0x08, 0x00, 0x01, 0x0B, 0x00])
        ser.write(cmd)
        print(f"📤 READ ENABLE: {' '.join(f'{b:02x}' for b in cmd)}")
        time.sleep(0.1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            if len(response) >= 7 and response[0] == 0x32 and response[1] == 0x03:
                print("✅ READ ENABLE - OK!")
            else:
                print("❌ READ ENABLE - FAIL!")
        else:
            print("❌ Brak odpowiedzi!")
        
        print("\n🚀 Test 2: READ PID (0x0000, 1 reg)")
        # READ PID (0x0000, 1 reg) - 32 03 00 00 00 01 C9 81
        cmd = bytes([0x32, 0x03, 0x00, 0x00, 0x00, 0x01, 0xC9, 0x81])
        ser.write(cmd)
        print(f"📤 READ PID: {' '.join(f'{b:02x}' for b in cmd)}")
        time.sleep(0.1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            if len(response) >= 7 and response[0] == 0x32 and response[1] == 0x03:
                print("✅ READ PID - OK!")
                print(f"🎯 PID: 0x{response[3]:02x}{response[4]:02x}")
            else:
                print("❌ READ PID - FAIL!")
        else:
            print("❌ Brak odpowiedzi!")
        
        print("\n🚀 Test 3: READ DEV ADDR (0x0002, 1 reg)")
        # READ DEV ADDR (0x0002, 1 reg) - 32 03 00 02 00 01 09 20
        cmd = bytes([0x32, 0x03, 0x00, 0x02, 0x00, 0x01, 0x09, 0x20])
        ser.write(cmd)
        print(f"📤 READ DEV ADDR: {' '.join(f'{b:02x}' for b in cmd)}")
        time.sleep(0.1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            if len(response) >= 7 and response[0] == 0x32 and response[1] == 0x03:
                print("✅ READ DEV ADDR - OK!")
                print(f"🎯 DEV ADDR: 0x{response[3]:02x}{response[4]:02x}")
            else:
                print("❌ READ DEV ADDR - FAIL!")
        else:
            print("❌ Brak odpowiedzi!")
        
        print("\n🚀 Test 4: WRITE ENABLE=1 (0x0008)")
        # WRITE ENABLE=1 (0x0008) - 32 06 00 08 00 01 0B CC
        cmd = bytes([0x32, 0x06, 0x00, 0x08, 0x00, 0x01, 0x0B, 0xCC])
        ser.write(cmd)
        print(f"📤 WRITE ENABLE=1: {' '.join(f'{b:02x}' for b in cmd)}")
        time.sleep(0.1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            if len(response) >= 8 and response[0] == 0x32 and response[1] == 0x06:
                print("✅ WRITE ENABLE - OK!")
            else:
                print("❌ WRITE ENABLE - FAIL!")
        else:
            print("❌ Brak odpowiedzi!")
        
        print("\n🚀 Test 5: WRITE DUTY=255 (0x0006)")
        # WRITE DUTY=255 (0x0006) - 32 06 00 06 00 FF 48 2C
        cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0xFF, 0x48, 0x2C])
        ser.write(cmd)
        print(f"📤 WRITE DUTY=255: {' '.join(f'{b:02x}' for b in cmd)}")
        print("🎯 Sprawdź czy silnik się obraca!")
        time.sleep(0.1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            if len(response) >= 8 and response[0] == 0x32 and response[1] == 0x06:
                print("✅ WRITE DUTY - OK!")
            else:
                print("❌ WRITE DUTY - FAIL!")
        else:
            print("❌ Brak odpowiedzi!")
        
        print("\n🚀 Test 6: WRITE FREQ ≈ 860 Hz (0x0007 = 0x0035)")
        # WRITE FREQ ≈ 860 Hz (0x0007 = 0x0035) - 32 06 00 07 00 35 DF FD
        cmd = bytes([0x32, 0x06, 0x00, 0x07, 0x00, 0x35, 0xDF, 0xFD])
        ser.write(cmd)
        print(f"📤 WRITE FREQ=0x0035: {' '.join(f'{b:02x}' for b in cmd)}")
        time.sleep(0.1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            if len(response) >= 8 and response[0] == 0x32 and response[1] == 0x06:
                print("✅ WRITE FREQ - OK!")
            else:
                print("❌ WRITE FREQ - FAIL!")
        else:
            print("❌ Brak odpowiedzi!")
        
        print("\n🚀 Test 7: WRITE MULTIPLE (duty=200, freq-div=0x0035)")
        # WRITE MULTIPLE (duty=200, freq-div=0x0035) - 32 10 00 06 00 02 04 00 C8 00 35 28 C2
        cmd = bytes([0x32, 0x10, 0x00, 0x06, 0x00, 0x02, 0x04, 0x00, 0xC8, 0x00, 0x35, 0x28, 0xC2])
        ser.write(cmd)
        print(f"📤 WRITE MULTIPLE: {' '.join(f'{b:02x}' for b in cmd)}")
        time.sleep(0.1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            if len(response) >= 8 and response[0] == 0x32 and response[1] == 0x10:
                print("✅ WRITE MULTIPLE - OK!")
            else:
                print("❌ WRITE MULTIPLE - FAIL!")
        else:
            print("❌ Brak odpowiedzi!")
        
        print("\n🚀 Test 8: Zatrzymanie silnika")
        # WRITE DUTY=0 (0x0006) - 32 06 00 06 00 00 6C 08
        cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0x00, 0x6C, 0x08])
        ser.write(cmd)
        print(f"📤 WRITE DUTY=0: {' '.join(f'{b:02x}' for b in cmd)}")
        print("🛑 Silnik powinien się zatrzymać!")
        time.sleep(0.1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            if len(response) >= 8 and response[0] == 0x32 and response[1] == 0x06:
                print("✅ WRITE DUTY=0 - OK!")
            else:
                print("❌ WRITE DUTY=0 - FAIL!")
        else:
            print("❌ Brak odpowiedzi!")
        
        ser.close()
        print("\n✅ Test zakończony!")
        
    except Exception as e:
        print(f"❌ Błąd: {e}")

if __name__ == "__main__":
    test_motor_deterministic()
