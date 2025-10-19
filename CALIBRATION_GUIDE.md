# Instrukcja Kalibracji HX711

## Podłączenie

Wzmacniacz HX711 jest podłączony do ESP32:
- **DT (Data)**: GPIO13 (kabel pomarańczowy)
- **SCK (Clock)**: GPIO14 (kabel żółty)
- **VCC**: 3.3V lub 5V
- **GND**: GND

Belka tensometryczna podłączona do HX711:
- E+ (Excitation+): Czerwony
- E- (Excitation-): Czarny
- A+ (Signal+): Biały
- A- (Signal-): Zielony

## Krok 1: Przygotowanie do Kalibracji

### Opcja A: Użyj przykładowego kodu kalibracji

1. Otwórz plik `main/hx711_calibration_example.c`
2. Tymczasowo zmień `main/CMakeLists.txt`:

```cmake
idf_component_register(SRCS "hx711_calibration_example.c"
                              "hx711.c"
                       INCLUDE_DIRS ".")
```

3. Zbuduj i wgraj kod:
```bash
idf.py build flash monitor
```

### Opcja B: Użyj monitora szeregowego

Możesz też dodać komendy kalibracji bezpośrednio w głównym kodzie.

## Krok 2: Kalibracja OFFSET (Tara)

1. **Usuń całe obciążenie** z belki tensometrycznej (winda pusta)
2. Uruchom kod kalibracji
3. Poczekaj na stabilizację odczytu
4. Zapisz wartość **OFFSET** wyświetloną na monitorze szeregowym

Przykład:
```
OFFSET = 50682624
```

## Krok 3: Kalibracja CALIBRATION_FACTOR (Skala)

1. Umieść **znany ciężar** na belce (np. 10kg, 50kg, 100kg)
2. Poczekaj na stabilizację odczytu
3. Zapisz wartość **RAW** wyświetloną na monitorze
4. Oblicz współczynnik kalibracji:

```
CALIBRATION_FACTOR = (RAW_VALUE - OFFSET) / KNOWN_WEIGHT_KG
```

Przykład:
- Offset: 50682624
- Raw value przy 100kg: 43632624
- Calculation: (43632624 - 50682624) / 100 = -70500

```
CALIBRATION_FACTOR = -70500.0
```

**Uwaga:** Wartość ujemna jest normalna i zależy od sposobu podłączenia belki.

## Krok 4: Aktualizacja Konfiguracji

Otwórz `main/elevator_config.h` i zaktualizuj wartości:

```c
#define HX711_CALIBRATION_FACTOR -70500.0  // Twoja obliczona wartość
#define HX711_OFFSET 50682624              // Twoja wartość offsetu
#define OVERLOAD_THRESHOLD_KG 500.0        // Max dopuszczalne obciążenie
```

## Krok 5: Test

1. Przywróć oryginalny `main/CMakeLists.txt`
2. Zbuduj i wgraj główny kod windy:

```bash
idf.py build flash monitor
```

3. Obserwuj logi - co 10 sekund powinna pokazywać się aktualna waga:

```
I (12345) SENSORS: Current weight: 0.00 kg
I (22345) SENSORS: Current weight: 75.50 kg
I (32345) SENSORS: Current weight: 150.25 kg
```

## Rozwiązywanie Problemów

### Problem: Odczyty niestabilne

**Rozwiązanie:**
- Sprawdź jakość połączeń
- Upewnij się, że kable są dobrze zabezpieczone
- Dodaj kondensator 100nF między VCC a GND HX711
- Użyj ekranowanych kabli dla belki tensometrycznej

### Problem: Wartości zawsze 0

**Rozwiązanie:**
- Sprawdź podłączenie DT i SCK
- Sprawdź zasilanie HX711 (3.3V lub 5V)
- Sprawdź czy belka tensometryczna jest prawidłowo podłączona

### Problem: Wartości rosną/maleją odwrotnie

**Rozwiązanie:**
- Zmień znak CALIBRATION_FACTOR
- Lub zamień przewody A+ i A- belki

### Problem: Przeciążenie nie wykrywane

**Rozwiązanie:**
- Dostosuj `OVERLOAD_THRESHOLD_KG` w elevator_config.h
- Sprawdź czy kalibracja jest poprawna
- Zweryfikuj czy belka jest odpowiednio zamontowana

## Przykładowe Wartości dla Różnych Belek

| Typ Belki | Zakres | Typowy Calibration Factor |
|-----------|--------|---------------------------|
| 1kg       | 0-1kg  | -500 do -1000            |
| 5kg       | 0-5kg  | -2000 do -4000           |
| 10kg      | 0-10kg | -4000 do -8000           |
| 50kg      | 0-50kg | -20000 do -40000         |
| 100kg     | 0-100kg| -40000 do -80000         |
| 200kg     | 0-200kg| -80000 do -160000        |

**Uwaga:** To tylko przykładowe wartości. Rzeczywisty współczynnik zależy od konkretnej belki.

## Testowanie

### Test 1: Waga Pusta
```
Expected: 0-2 kg (błąd pomiaru)
```

### Test 2: Znany Ciężar
```
Umieść znany ciężar, porównaj odczyt z rzeczywistą wagą
Dopuszczalny błąd: ±2%
```

### Test 3: Przeciążenie
```
Umieść ciężar > OVERLOAD_THRESHOLD_KG
Expected: Log "Overload detected"
```

## Konserwacja

- Sprawdzaj kalibrację co 6 miesięcy
- Sprawdzaj połączenia co miesiąc
- Czyść belkę z kurzu/brudu
- Zabezpiecz przed wilgocią

