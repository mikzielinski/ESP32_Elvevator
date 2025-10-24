#!/usr/bin/env python3

def decode_motor_response():
    response = "328004b00c"
    print(f"🔍 Analiza odpowiedzi sterownika: {response}")
    
    # Konwertuj hex na bytes
    data = bytes.fromhex(response)
    print(f"📊 Dane binarne: {data}")
    print(f"📊 ASCII: {data.decode('utf-8', errors='ignore')}")
    
    # Analiza każdego bajtu
    print("\n📋 Analiza bajtów:")
    for i, byte in enumerate(data):
        print(f"Bajt {i}: 0x{byte:02x} ({byte}) - {chr(byte) if 32 <= byte <= 126 else 'nie-ASCII'}")
    
    # Sprawdź czy to może być status
    print(f"\n🔍 Możliwe interpretacje:")
    print(f"- Status: {data[0]:02x}")
    print(f"- Motor state: {data[1]:02x}")
    print(f"- Speed: {data[2]:02x}")
    print(f"- Error code: {data[3]:02x}")

if __name__ == "__main__":
    decode_motor_response()
