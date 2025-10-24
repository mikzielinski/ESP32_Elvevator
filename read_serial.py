#!/usr/bin/env python3
import serial
import time
import sys

try:
    ser = serial.Serial('/dev/cu.usbmodem5A671675611', 115200, timeout=1)
    print("Connected to ESP32-S3. Reading data...")
    print("=" * 80)
    
    start_time = time.time()
    line_count = 0
    max_lines = 50
    max_duration = 15  # seconds
    
    while line_count < max_lines and (time.time() - start_time) < max_duration:
        if ser.in_waiting > 0:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').rstrip()
                if line:
                    print(line)
                    line_count += 1
            except Exception as e:
                print(f"Error reading line: {e}")
        time.sleep(0.01)
    
    print("=" * 80)
    print(f"Read {line_count} lines in {time.time() - start_time:.1f} seconds")
    ser.close()
    
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
    sys.exit(1)
except KeyboardInterrupt:
    print("\nInterrupted by user")
    sys.exit(0)

