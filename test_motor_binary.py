#!/usr/bin/env python3
import serial
import time

def test_motor_binary():
    try:
        print("🔌 Łączenie ze sterownikiem DRI0050...")
        ser = serial.Serial('/dev/cu.usbserial-210', 9600, timeout=2)
        print("✅ Połączono! Testuję komendy binarne...")
        
        # Test komend binarnych (takie same jak w ESP32)
        commands = [
            ("FORWARD", [0x01, 0x64, 0x01]),    # Forward 100% speed
            ("STOP", [0x00, 0x00, 0x00]),       # Stop motor
            ("BACKWARD", [0x01, 0x64, 0x00]),   # Backward 100% speed
            ("STOP", [0x00, 0x00, 0x00]),       # Stop motor
            ("FORWARD_50", [0x01, 0x32, 0x01]), # Forward 50% speed
            ("STOP", [0x00, 0x00, 0x00]),       # Stop motor
        ]
        
        for name, cmd in commands:
            print(f"\n📤 Test: {name}")
            print(f"📤 Wysyłam binarnie: {[hex(x) for x in cmd]}")
            ser.write(bytes(cmd))
            
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
    test_motor_binary()
