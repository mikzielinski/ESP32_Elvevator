# Rozwiązanie: Konwerter poziomów napięć

## Problem
- ESP32: 3.3V logic
- Sterownik: oczekuje 5V logic
- Komunikacja UART nie działa

## Rozwiązanie: Konwerter poziomów

### Opcja 1: Moduł konwertera
- **TX**: ESP32 GPIO12 (3.3V) → Konwerter → Sterownik TX (5V)
- **RX**: Sterownik RX (5V) → Konwerter → ESP32 GPIO11 (3.3V)
- **GND**: wspólne masy

### Opcja 2: Dzielnik napięcia (prosty)
- **Rezystor 1kΩ** między GPIO12 a sterownik TX
- **Rezystor 2kΩ** między sterownik TX a GND
- **To da ~3.3V na sterownik**

### Opcja 3: Inne GPIO (5V tolerant)
- **GPIO21, GPIO22** - mogą być 5V tolerant
- **Test z innymi pinami**

## Test
1. Dodaj konwerter poziomów
2. Przetestuj komunikację
3. Sprawdź czy silnik reaguje

## Alternatywa
- **Użyj Arduino** zamiast ESP32 (5V logic)
- **Lub znajdź sterownik 3.3V compatible**
