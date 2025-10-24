#!/usr/bin/env python3
"""
Test różnych protokołów dla sterownika DRI0050
"""

import serial
import time
import sys

def test_motor_protocol():
    print("🔍 Test różnych protokołów dla sterownika DRI0050...")
    
    try:
        # Otwórz port sterownika
        ser = serial.Serial('/dev/cu.usbmodem5A671675611', 9600, timeout=1)
        time.sleep(2)
        
        print("✅ Połączono ze sterownikiem!")
        
        print("\n🚀 Test 1: Protokół ModBus RTU (Write Single Register)")
        
        # Test 1: ModBus RTU Write Single Register
        cmd = bytes([0x32, 0x06, 0x00, 0x06, 0x00, 0xFF, 0x48, 0x2C])  # Duty = 255
        ser.write(cmd)
        print(f"📤 ModBus RTU: {' '.join(f'{b:02x}' for b in cmd)}")
        time.sleep(0.5)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
        
        print("\n🚀 Test 2: Protokół ModBus RTU (Read Holding Register)")
        
        # Test 2: ModBus RTU Read Holding Register
        cmd = bytes([0x32, 0x03, 0x00, 0x06, 0x00, 0x01, 0x64, 0x0B])  # Read register 0x0006
        ser.write(cmd)
        print(f"📤 ModBus RTU Read: {' '.join(f'{b:02x}' for b in cmd)}")
        time.sleep(0.5)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
        
        print("\n🚀 Test 3: Protokół ModBus RTU (Write Multiple Registers)")
        
        # Test 3: ModBus RTU Write Multiple Registers
        cmd = bytes([0x32, 0x10, 0x00, 0x06, 0x00, 0x01, 0x02, 0x00, 0xFF, 0x48, 0x2C])  # Write multiple
        ser.write(cmd)
        print(f"📤 ModBus RTU Write Multiple: {' '.join(f'{b:02x}' for b in cmd)}")
        time.sleep(0.5)
        
        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting)
            print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
        
        print("\n🚀 Test 4: Protokół ASCII (Text Commands)")
        
        # Test 4: ASCII Text Commands
        text_commands = [
            b"PWM_DUTY=255\r\n",
            b"SET_PWM=255\r\n",
            b"PWM=255\r\n",
            b"DUTY=255\r\n"
        ]
        
        for cmd in text_commands:
            ser.write(cmd)
            print(f"📤 ASCII: {cmd.decode('utf-8', errors='ignore').strip()}")
            time.sleep(0.5)
            
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting)
                print(f"📥 Odpowiedź: {response}")
        
        print("\n🚀 Test 5: Protokół Binary (Simple Commands)")
        
        # Test 5: Binary Simple Commands
        binary_commands = [
            bytes([0x32, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]),  # Simple binary
            bytes([0x32, 0x01, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00]),  # Binary with data
            bytes([0x32, 0x02, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00]),  # Binary with register
        ]
        
        for i, cmd in enumerate(binary_commands):
            ser.write(cmd)
            print(f"📤 Binary {i+1}: {' '.join(f'{b:02x}' for b in cmd)}")
            time.sleep(0.5)
            
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting)
                print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
        
        print("\n🚀 Test 6: Sprawdzenie rejestrów sterownika")
        
        # Test 6: Sprawdzenie rejestrów
        registers = [
            (0x0000, "PID"),
            (0x0001, "VID"),
            (0x0002, "Device Address"),
            (0x0005, "Version"),
            (0x0006, "PWM Duty"),
            (0x0007, "PWM Frequency"),
            (0x0008, "PWM Enable")
        ]
        
        for reg_addr, reg_name in registers:
            cmd = bytes([0x32, 0x03, (reg_addr >> 8) & 0xFF, reg_addr & 0xFF, 0x00, 0x01, 0x00, 0x00])
            # Calculate CRC for read command
            crc = 0xFFFF
            for b in cmd[:6]:
                crc ^= b
                for _ in range(8):
                    if crc & 1:
                        crc = (crc >> 1) ^ 0xA001
                    else:
                        crc >>= 1
            crc = ((crc & 0x00FF) << 8) | ((crc & 0xFF00) >> 8)
            cmd = cmd[:6] + bytes([(crc >> 8) & 0xFF, crc & 0xFF])
            
            ser.write(cmd)
            print(f"📤 Read {reg_name} (0x{reg_addr:04x}): {' '.join(f'{b:02x}' for b in cmd)}")
            time.sleep(0.5)
            
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting)
                print(f"📥 Odpowiedź: {' '.join(f'{b:02x}' for b in response)}")
        
        ser.close()
        print("\n✅ Test zakończony!")
        
    except Exception as e:
        print(f"❌ Błąd: {e}")

if __name__ == "__main__":
    test_motor_protocol()