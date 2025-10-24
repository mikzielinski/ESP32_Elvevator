#!/usr/bin/env python3
"""
Twarda, zero-wymówek diagnostyka DRI0050
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

def test_motor_hard_diagnosis():
    print("🔍 Twarda, zero-wymówek diagnostyka DRI0050...")
    
    print("💡 CHECKLISTA PRZED TESTEM:")
    print("   1. ✅ Odłącz USB-C od DRI0050 podczas testu z ESP32")
    print("   2. ✅ Przełącznik na płytce = UART/COM (nie analog)")
    print("   3. ✅ Wspólna masa (GND ESP32 ↔ GND DRI0050)")
    print("   4. ✅ TX/RX skrzyżuj (ESP TX → DRI0050 RX, ESP RX → DRI0050 TX)")
    print("   5. ✅ Baud i format: 9600, 8N1")
    print("   6. ✅ t3.5/przerwy: ≥ 5-10 ms pauzy między ramkami")
    print("   7. ✅ Nie myl UARTa z PWM OUT")
    print("   8. ✅ Adres slave: domyślnie 0x32")
    print("   9. ✅ Piny ESP32-S3: GPIO11 (RX), GPIO12 (TX)")
    print("   10. ✅ Poziom sygnału 3V3 → 5V (konwerter jeśli trzeba)")
    
    try:
        # Otwórz port sterownika
        ser = serial.Serial('/dev/cu.usbmodem5A671675611', 9600, timeout=1)
        time.sleep(2)
        
        print("\n✅ Połączono ze sterownikiem!")
        
        print("\n🚀 Test 1: READ ADDR (0x0002) - powinien zwrócić 0x0032")
        # READ ADDR (0x0002) - 32 03 00 02 00 01 09 20
        cmd = bytes([0x32, 0x03, 0x00, 0x02, 0x00, 0x01, 0x09, 0x20])
        ser.write(cmd)
        print(f"📤 READ ADDR: {' '.join(f'{b:02x}' for b in cmd)}")
        print("💡 Oczekiwana odpowiedź: 32 03 02 00 32 XX YY")
        time.sleep(0.1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            if len(response) >= 7 and response[0] == 0x32 and response[1] == 0x03:
                print("✅ READ ADDR - OK!")
                print(f"🎯 DEV ADDR: 0x{response[3]:02x}{response[4]:02x}")
            else:
                print("❌ READ ADDR - FAIL!")
                print("💡 To nie jest ramka ModBus!")
        else:
            print("❌ Brak odpowiedzi!")
        
        print("\n🚀 Test 2: READ ENABLE (0x0008) - sprawdź czy ENABLE=1")
        # READ ENABLE (0x0008) - 32 03 00 08 00 01 0B 00
        cmd = bytes([0x32, 0x03, 0x00, 0x08, 0x00, 0x01, 0x0B, 0x00])
        ser.write(cmd)
        print(f"📤 READ ENABLE: {' '.join(f'{b:02x}' for b in cmd)}")
        print("💡 Oczekiwana odpowiedź:")
        print("   - Gdy ENABLE=1: 32 03 02 00 01 7D 80")
        print("   - Gdy ENABLE=0: 32 03 02 00 00 3C 40")
        time.sleep(0.1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            if len(response) >= 7 and response[0] == 0x32 and response[1] == 0x03:
                print("✅ READ ENABLE - OK!")
                enable_value = response[4]
                print(f"🎯 ENABLE: {enable_value}")
            else:
                print("❌ READ ENABLE - FAIL!")
                print("💡 To nie jest ramka ModBus!")
        else:
            print("❌ Brak odpowiedzi!")
        
        print("\n🚀 Test 3: READ PID (0x0000) - powinien zwrócić 0xC032")
        # READ PID (0x0000) - 32 03 00 00 00 01 C9 81
        cmd = bytes([0x32, 0x03, 0x00, 0x00, 0x00, 0x01, 0xC9, 0x81])
        ser.write(cmd)
        print(f"📤 READ PID: {' '.join(f'{b:02x}' for b in cmd)}")
        print("💡 Oczekiwana odpowiedź: 32 03 02 C0 32 XX YY")
        time.sleep(0.1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            if len(response) >= 7 and response[0] == 0x32 and response[1] == 0x03:
                print("✅ READ PID - OK!")
                pid = (response[3] << 8) | response[4]
                print(f"🎯 PID: 0x{pid:04x}")
            else:
                print("❌ READ PID - FAIL!")
                print("💡 To nie jest ramka ModBus!")
        else:
            print("❌ Brak odpowiedzi!")
        
        print("\n🚀 Test 4: WRITE ENABLE=1 (0x0008)")
        # WRITE ENABLE=1 (0x0008) - 32 06 00 08 00 01 0B CC
        cmd = bytes([0x32, 0x06, 0x00, 0x08, 0x00, 0x01, 0x0B, 0xCC])
        ser.write(cmd)
        print(f"📤 WRITE ENABLE=1: {' '.join(f'{b:02x}' for b in cmd)}")
        print("💡 Oczekiwana odpowiedź: 32 06 00 08 00 01 XX YY")
        time.sleep(0.1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            if len(response) >= 8 and response[0] == 0x32 and response[1] == 0x06:
                print("✅ WRITE ENABLE - OK!")
            else:
                print("❌ WRITE ENABLE - FAIL!")
                print("💡 To nie jest ramka ModBus!")
        else:
            print("❌ Brak odpowiedzi!")
        
        print("\n🚀 Test 5: WRITE DUTY=255 (0x0006)")
        # WRITE DUTY=255 (0x0006) - 32 06 00 06 00 FF 48 2C
        cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0xFF, 0x48, 0x2C])
        ser.write(cmd)
        print(f"📤 WRITE DUTY=255: {' '.join(f'{b:02x}' for b in cmd)}")
        print("💡 Oczekiwana odpowiedź: 32 06 00 06 00 FF XX YY")
        print("🎯 Sprawdź czy silnik się obraca!")
        time.sleep(0.1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            if len(response) >= 8 and response[0] == 0x32 and response[1] == 0x06:
                print("✅ WRITE DUTY - OK!")
            else:
                print("❌ WRITE DUTY - FAIL!")
                print("💡 To nie jest ramka ModBus!")
        else:
            print("❌ Brak odpowiedzi!")
        
        print("\n🚀 Test 6: Zatrzymanie silnika")
        # WRITE DUTY=0 (0x0006) - 32 06 00 06 00 00 6C 08
        cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0x00, 0x6C, 0x08])
        ser.write(cmd)
        print(f"📤 WRITE DUTY=0: {' '.join(f'{b:02x}' for b in cmd)}")
        print("💡 Oczekiwana odpowiedź: 32 06 00 06 00 00 XX YY")
        print("🛑 Silnik powinien się zatrzymać!")
        time.sleep(0.1)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
            if len(response) >= 8 and response[0] == 0x32 and response[1] == 0x06:
                print("✅ WRITE DUTY=0 - OK!")
            else:
                print("❌ WRITE DUTY=0 - FAIL!")
                print("💡 To nie jest ramka ModBus!")
        else:
            print("❌ Brak odpowiedzi!")
        
        ser.close()
        print("\n✅ Test zakończony!")
        
        print("\n🔍 ANALIZA WYNIKÓW:")
        print("   - Jeśli widzisz ramki ModBus (32 03/06 XX XX XX XX XX XX) → OK!")
        print("   - Jeśli widzisz śmieci (52 77 62 d6) → Problem z połączeniem!")
        print("   - Jeśli brak odpowiedzi → Problem z protokołem!")
        
    except Exception as e:
        print(f"❌ Błąd: {e}")

if __name__ == "__main__":
    test_motor_hard_diagnosis()
