# ESP32 HX711 Load Cell WiFi Dashboard

Real-time monitoring wagi z belki tensometrycznej przez HX711 z WiFi dashboardem!

## 📋 Opis

Ten projekt umożliwia odczyt wagi z belki tensometrycznej podłączonej przez wzmacniacz HX711 do mikrokontrolera ESP32, z **real-time dashboardem WiFi**!

## ✨ Funkcje

- 📊 **Web Dashboard** - piękny interfejs w przeglądarce
- 📡 **WiFi** - dostęp z dowolnego urządzenia w sieci
- ⚡ **WebSocket** - dane w czasie rzeczywistym
- 📈 **Live wykres** - historia pomiarów
- 🎯 **Zerowanie (Tara)** - funkcja jak w sklepowej wadze
- 📱 **Mobile-friendly** - działa na telefonach

Idealny do projektów:
- Inteligentne wagi
- IoT monitoring obciążenia
- Systemy kontroli wagi
- Home automation
- Przemysłowy monitoring

## 🔌 Podłączenie Sprzętowe

### HX711 Load Cell Amplifier
- **DT (Data)**: GPIO13 (kabel pomarańczowy)
- **SCK (Clock)**: GPIO14 (kabel żółty)
- **VCC**: 3.3V lub 5V (w zależności od modułu)
- **GND**: GND

### Belka Tensometryczna → HX711
- **E+ (Excitation+)**: Czerwony
- **E- (Excitation-)**: Czarny
- **A+ (Signal+)**: Biały
- **A- (Signal-)**: Zielony

```
ESP32          HX711          Load Cell
              ┌─────┐
GPIO13 ──────►│ DT  │
GPIO14 ──────►│ SCK │         E+ ← Czerwony
3.3V   ──────►│ VCC │         E- ← Czarny
GND    ──────►│ GND │         A+ ← Biały
              │     │         A- ← Zielony
              └─────┘
```

## 🚀 Szybki Start

### 1. Konfiguruj WiFi

Edytuj `main/wifi_config.h` i wpisz swoje dane WiFi:
```c
#define WIFI_SSID "TwojaSiecWiFi"
#define WIFI_PASSWORD "TwojeHaslo"
```

### 2. Zainstaluj ESP-IDF

Pobierz i zainstaluj ESP-IDF (wersja 5.0 lub nowsza):
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/

### 2. Sklonuj Repozytorium

```bash
git clone <url-repozytorium>
cd ESP32-Elevator
```

### 3. Kalibracja (Ważne!)

Przed pierwszym użyciem musisz skalibrować HX711:

1. Przeczytaj szczegółową instrukcję w `CALIBRATION_GUIDE.md`
2. Odkomentuj sekcję kalibracji w `main/main.c`
3. Zbuduj i wgraj kod
4. Postępuj zgodnie z instrukcjami na monitorze szeregowym
5. Zaktualizuj wartości w `main/hx711_config.h`:
   - `HX711_OFFSET` - wartość tary (waga bez obciążenia)
   - `HX711_CALIBRATION_FACTOR` - współczynnik kalibracji

### 4. Budowanie i Wgrywanie

```bash
# Budowanie
idf.py build

# Wgranie do ESP32 i uruchomienie monitora
idf.py -p /dev/cu.usbmodem21101 flash monitor
```

### 5. Otwórz Dashboard 🎉

W logach znajdź adres IP:
```
Dashboard: http://192.168.1.xxx
```

Otwórz ten adres w przeglądarce na komputerze/telefonie!

📖 **Szczegóły**: Zobacz `WIFI_SETUP.md` dla pełnej instrukcji.

Zamień `/dev/ttyUSB0` na port szeregowy twojego ESP32:
- Linux/Mac: `/dev/ttyUSB0` lub `/dev/cu.usbserial-*`
- Windows: `COM3`, `COM4`, etc.

## 📁 Struktura Projektu

```
ESP32-Elevator/
├── main/
│   ├── main.c              # Główny program
│   ├── hx711.c             # Sterownik HX711
│   ├── hx711.h             # Nagłówek HX711
│   ├── hx711_config.h      # Konfiguracja pinów i kalibracji
│   └── CMakeLists.txt
├── CMakeLists.txt          # Główna konfiguracja CMake
├── CALIBRATION_GUIDE.md    # Szczegółowa instrukcja kalibracji
├── README.md               # Ten plik
└── LICENSE
```

## ⚙️ Konfiguracja

Edytuj `main/hx711_config.h`:

```c
// Piny GPIO
#define HX711_DT_PIN GPIO_NUM_13    // Pomarańczowy - Data
#define HX711_SCK_PIN GPIO_NUM_14   // Żółty - Clock

// Wartości kalibracji (zaktualizuj po kalibracji!)
#define HX711_CALIBRATION_FACTOR -7050.0
#define HX711_OFFSET 50682624

// Ustawienia pomiaru
#define READINGS_PER_SAMPLE 10      // Liczba odczytów do uśrednienia
#define UPDATE_INTERVAL_MS 1000     // Częstotliwość odczytu (ms)
```

## 📊 Przykładowe Wyjście

```
I (324) HX711_DEMO: ===========================================
I (334) HX711_DEMO:     HX711 Load Cell Weight Monitor
I (334) HX711_DEMO: ===========================================
I (344) HX711_DEMO: DT Pin:  GPIO13 (Orange)
I (344) HX711_DEMO: SCK Pin: GPIO14 (Yellow)
I (354) HX711_DEMO: ===========================================
I (364) HX711_DEMO: Initializing HX711...
I (364) HX711: HX711 initialized on DT=GPIO13, SCK=GPIO14
I (374) HX711_DEMO: HX711 initialized successfully!
I (384) HX711_DEMO: Starting continuous weight monitoring...
I (1394) HX711_DEMO: [1] Weight: 0.00 kg | Raw: 50682540
I (2404) HX711_DEMO: [2] Weight: 0.02 kg | Raw: 50682680
I (3414) HX711_DEMO: [3] Weight: 1.05 kg | Raw: 50675226
I (4424) HX711_DEMO: [4] Weight: 5.23 kg | Raw: 50645774
```

## 🔧 Rozwiązywanie Problemów

### Problem: "HX711 not ready"
**Rozwiązanie:**
- Sprawdź połączenia DT i SCK
- Sprawdź zasilanie HX711 (VCC i GND)
- Upewnij się, że belka jest podłączona do HX711

### Problem: Odczyty niestabilne
**Rozwiązanie:**
- Dodaj kondensator 100nF między VCC a GND HX711
- Użyj ekranowanych kabli dla belki tensometrycznej
- Trzymaj kable z dala od źródeł zakłóceń
- Sprawdź jakość połączeń (luźne kable)

### Problem: Wartości zawsze 0 lub bardzo duże liczby
**Rozwiązanie:**
- Przeprowadź ponowną kalibrację
- Sprawdź podłączenie belki (czy przewody E+, E-, A+, A- są prawidłowo podłączone)
- Zweryfikuj wartości w `hx711_config.h`

### Problem: Waga pokazuje wartości odwrotne (rośnie zamiast maleć)
**Rozwiązanie:**
- Zmień znak `HX711_CALIBRATION_FACTOR` (z minus na plus lub odwrotnie)
- Lub zamień przewody A+ i A- belki

## 📖 Dodatkowe Informacje

### Specyfikacja HX711
- 24-bitowy przetwornik ADC
- Wzmocnienie: 128 (kanał A), 64 (kanał A), 32 (kanał B)
- Częstotliwość próbkowania: 10Hz lub 80Hz
- Zakres napięcia: 2.6V - 5.5V

### Typowe Belki Tensometryczne
- 1kg, 5kg, 10kg - małe wagi
- 50kg, 100kg - średnie wagi, systemy kontroli
- 200kg, 500kg - duże wagi, systemy przemysłowe

## 📝 Licencja

MIT License - zobacz plik LICENSE

## 🤝 Wkład

Pull requesty są mile widziane! W przypadku większych zmian, najpierw otwórz issue.

## 📧 Kontakt

W razie pytań otwórz issue na GitHubie.

---

**Uwaga:** Zawsze testuj system przed użyciem w aplikacjach krytycznych. Ten projekt jest dostarczany "jak jest" bez żadnych gwarancji.
