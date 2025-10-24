#!/usr/bin/env python3
import serial
import time

def listen_to_motor():
    try:
        print("🔌 Łączenie ze sterownikiem...")
        ser = serial.Serial('/dev/cu.usbserial-210', 9600, timeout=1)
        print("✅ Połączono! Nasłuchuję co wysyła sterownik...")
        print("📡 Czekam 10 sekund na dane...")
        
        start_time = time.time()
        data_received = False
        
        while time.time() - start_time < 10:
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting)
                print(f"📥 Odebrano: {data}")
                print(f"📥 Hex: {data.hex()}")
                print(f"📥 ASCII: {data.decode('utf-8', errors='ignore')}")
                data_received = True
            
            time.sleep(0.1)
        
        if not data_received:
            print("📥 Brak danych z sterownika")
        
        print("\n🧪 Teraz wyślę komendy i sprawdzę odpowiedzi...")
        
        # Test komend
        commands = ["F\n", "S\n", "FORWARD\n", "STOP\n"]
        
        for cmd in commands:
            print(f"\n📤 Wysyłam: {cmd.strip()}")
            ser.write(cmd.encode())
            time.sleep(1)
            
            if ser.in_waiting > 0:
                response = ser.read(ser.in_waiting)
                print(f"📥 Odpowiedź: {response}")
                print(f"📥 Hex: {response.hex()}")
            else:
                print("📥 Brak odpowiedzi")
        
        ser.close()
        
    except Exception as e:
        print(f"❌ Błąd: {e}")

if __name__ == "__main__":
    listen_to_motor()
