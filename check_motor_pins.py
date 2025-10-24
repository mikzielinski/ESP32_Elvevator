#!/usr/bin/env python3

def check_motor_pins():
    print("🔍 SPRAWDŹ PINS NA STEROWNIKU DRI0050:")
    print("=" * 50)
    print("")
    print("📋 Szukaj pinów:")
    print("✅ PWM_IN1, PWM_IN2  (sygnały PWM)")
    print("✅ DIR1, DIR2        (kierunek)")
    print("✅ ENABLE            (włączenie)")
    print("✅ GND               (masa)")
    print("✅ VCC               (zasilanie)")
    print("")
    print("🔧 Jeśli masz te piny, to sterownik potrzebuje:")
    print("- Sygnałów PWM (nie UART)")
    print("- Kontroli kierunku przez GPIO")
    print("- Sygnału ENABLE")
    print("")
    print("🎯 To wyjaśniałoby dlaczego UART nie działa!")
    print("")
    print("📝 Sprawdź i powiedz jakie piny widzisz!")

if __name__ == "__main__":
    check_motor_pins()
