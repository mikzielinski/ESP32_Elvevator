#!/usr/bin/env python3
import serial
import time

def test_motor_pwm():
    print("🔍 TEST: Czy sterownik DRI0050 potrzebuje sygnałów PWM?")
    print("=" * 60)
    
    print("📋 INSTRUKCJE:")
    print("1. Sprawdź dokumentację sterownika DRI0050")
    print("2. Czy ma piny PWM_IN1, PWM_IN2?")
    print("3. Czy ma piny DIR1, DIR2?")
    print("4. Czy ma piny ENABLE?")
    print("")
    print("🔧 MOŻLIWE ROZWIĄZANIE:")
    print("- GPIO12 → PWM_IN1 (prędkość)")
    print("- GPIO11 → DIR1 (kierunek)")
    print("- GPIO10 → ENABLE (włączenie)")
    print("")
    print("📊 Sprawdź piny na sterowniku:")
    print("- PWM_IN1, PWM_IN2")
    print("- DIR1, DIR2") 
    print("- ENABLE")
    print("")
    print("🎯 To może być sterownik PWM, nie UART!")

if __name__ == "__main__":
    test_motor_pwm()
