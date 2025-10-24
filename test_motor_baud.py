#!/usr/bin/env python3
import serial
import time

def test_different_baud_rates():
    baud_rates = [9600, 115200, 57600, 38400, 19200, 4800]
    
    for baud in baud_rates:
        try:
            print(f"\n🔌 Testowanie baud rate: {baud}")
            ser = serial.Serial('/dev/cu.usbserial-210', baud, timeout=1)
            
            # Wyślij prostą komendę
            ser.write(b"F\n")
            time.sleep(0.5)
            
            # Sprawdź odpowiedź
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
                print(f"📥 Odpowiedź przy {baud}: {response.strip()}")
            else:
                print(f"📥 Brak odpowiedzi przy {baud}")
            
            ser.close()
            
        except Exception as e:
            print(f"❌ Błąd przy {baud}: {e}")

if __name__ == "__main__":
    test_different_baud_rates()
