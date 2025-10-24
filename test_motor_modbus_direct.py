#!/usr/bin/env python3
import serial
import time

def calculate_crc(data):
    """Calculate ModBus RTU CRC"""
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc >>= 1
                crc ^= 0xA001
            else:
                crc >>= 1
    return ((crc & 0x00FF) << 8) | ((crc & 0xFF00) >> 8)

def test_motor_modbus_direct():
    try:
        print("🔌 Łączenie ze sterownikiem DRI0050...")
        ser = serial.Serial('/dev/cu.usbserial-210', 9600, timeout=2)
        print("✅ Połączono! Testuję komendy ModBus RTU...")
        
        # Test ModBus RTU commands
        commands = [
            ("FORWARD", [0x32, 0x06, 0x00, 0x06, 0x00, 0xFF]),    # Forward 100% duty
            ("STOP", [0x32, 0x06, 0x00, 0x06, 0x00, 0x00]),      # Stop motor
            ("BACKWARD", [0x32, 0x06, 0x00, 0x06, 0x00, 0xFF]),  # Backward 100% duty
            ("STOP", [0x32, 0x06, 0x00, 0x06, 0x00, 0x00]),      # Stop motor
        ]
        
        for name, cmd in commands:
            print(f"\n📤 Test: {name}")
            print(f"📤 Wysyłam ModBus RTU: {[hex(x) for x in cmd]}")
            
            # Calculate CRC
            crc = calculate_crc(cmd)
            cmd_with_crc = cmd + [(crc >> 8) & 0xFF, crc & 0xFF]
            print(f"📤 Z CRC: {[hex(x) for x in cmd_with_crc]}")
            
            ser.write(bytes(cmd_with_crc))
            
            # Czekaj na odpowiedź
            time.sleep(1)
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting)
                print(f"📥 Odpowiedź: {response.hex()}")
            else:
                print("📥 Brak odpowiedzi")
            
            # Sprawdź czy silnik reaguje
            print("🔍 Sprawdź czy silnik zmienił kierunek/prędkość...")
            time.sleep(2)
        
        print("\n🧪 Test zakończony!")
        ser.close()
        
    except Exception as e:
        print(f"❌ Błąd: {e}")

if __name__ == "__main__":
    test_motor_modbus_direct()
