#!/usr/bin/env python3
import time

def test_motor_direct():
    print("🧪 TEST BEZPOŚREDNI SILNIKA")
    print("=" * 50)
    print("📋 INSTRUKCJE:")
    print("1. Odłącz silnik od sterownika (PWM OUT)")
    print("2. Podłącz 12V bezpośrednio do silnika")
    print("3. Sprawdź czy silnik się obraca")
    print("4. Naciśnij ENTER gdy skończysz test")
    print("=" * 50)
    
    input("Naciśnij ENTER gdy przetestujesz silnik bezpośrednio...")
    
    print("\n❓ WYNIKI TESTU:")
    print("1. Czy silnik się obracał przy 12V bezpośrednio?")
    print("2. Czy słyszałeś dźwięk obracania?")
    print("3. Czy silnik reagował na podłączenie 12V?")
    
    result = input("\nOdpowiedz (tak/nie): ")
    
    if result.lower() in ['tak', 'yes', 'y']:
        print("✅ Silnik działa - problem jest w sterowniku!")
        print("🔧 Sprawdź przełącznik na sterowniku")
    else:
        print("❌ Silnik nie działa - problem z silnikiem lub zasilaniem")
        print("🔧 Sprawdź zasilanie 12V i połączenia silnika")

if __name__ == "__main__":
    test_motor_direct()
