#!/usr/bin/env python3
import serial
import time

def test_motor_with_load():
    try:
        print("🔌 Łączenie ze sterownikiem...")
        ser = serial.Serial('/dev/cu.usbserial-210', 9600, timeout=2)
        print("✅ Połączono! Testuję komendy z silnikiem...")
        
        # Test komend z dłuższymi opóźnieniami
        commands = [
            ("FORWARD", "F"),
            ("STOP", "S"), 
            ("BACKWARD", "B"),
            ("STOP", "S")
        ]
        
        for cmd_name, cmd in commands:
            print(f"\n📤 Wysyłam: {cmd_name} ({cmd})")
            ser.write(f"{cmd}\n".encode())
            
            # Czekaj dłużej na odpowiedź
            time.sleep(2)
            
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting)
                print(f"📥 Odpowiedź: {response}")
                print(f"📥 Hex: {response.hex()}")
            else:
                print("📥 Brak odpowiedzi")
            
            # Sprawdź czy silnik reaguje
            print("🔍 Sprawdź czy silnik zmienił kierunek/prędkość...")
            time.sleep(1)
        
        print("\n🧪 Test zakończony!")
        ser.close()
        
    except Exception as e:
        print(f"❌ Błąd: {e}")

if __name__ == "__main__":
    test_motor_with_load()
