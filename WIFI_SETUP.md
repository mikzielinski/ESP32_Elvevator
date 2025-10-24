# WiFi Dashboard Setup Guide

## 📡 Konfiguracja WiFi

### Krok 1: Edytuj dane WiFi

Otwórz plik `main/wifi_config.h` i zmień dane na swoje:

```c
#define WIFI_SSID "YOUR_WIFI_SSID"        // Twoja nazwa sieci WiFi
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD" // Hasło do WiFi
```

### Krok 2: Zbuduj i wgraj

```bash
idf.py build
idf.py -p /dev/cu.usbmodem21101 flash
```

### Krok 3: Sprawdź IP

Monitoruj logi aby zobaczyć adres IP:

```bash
idf.py monitor
```

Poszukaj w logach:
```
Dashboard: http://192.168.1.xxx
```

### Krok 4: Otwórz Dashboard

W przeglądarce wejdź na wyświetlony adres IP.

## 🎨 Funkcje Dashboardu

### Real-time monitoring
- **Duży wyświetlacz wagi** w kg
- **Wartość RAW** z HX711
- **Status połączenia** WebSocket

### Wykres
- **Live chart** - ostatnie 50 pomiarów
- Automatyczne skalowanie
- Przycisk **Clear Chart** - czyści wykres

### Zerowanie (Tara)
- Przycisk **Zero (Tare)** - zeruje wagę
- Działa jak w normalnej wadze - odejmuje aktualną wartość

## 📱 Używanie

1. **Otwórz dashboard** w przeglądarce (może być telefon/tablet!)
2. **Połączenie WebSocket** - status powinien pokazać "✓ Connected"
3. **Obserwuj wagę** - aktualizacja co 2 sekundy
4. **Wykres** automatycznie rysuje historię pomiarów
5. **Zero** - kliknij aby wyzerować wagę (jak w sklepowej wadze)

## 🔧 Troubleshooting

### Nie łączy się z WiFi
- Sprawdź SSID i hasło w `wifi_config.h`
- Sprawdź czy ESP32 jest w zasięgu WiFi
- ESP32 wymaga WiFi 2.4GHz (nie działa z 5GHz!)

### Nie mogę otworzyć dashboardu
- Sprawdź adres IP w logach monitora
- Upewnij się że komputer/telefon jest w tej samej sieci WiFi
- Spróbuj http:// a nie https://

### WebSocket się rozłącza
- Sprawdź stabilność WiFi
- Niektóre routery mają timeout - możliwe że trzeba zwiększyć częstotliwość wysyłania

### Wykres nie pokazuje danych
- Odśwież stronę
- Sprawdź czy WebSocket jest połączony (status "✓ Connected")
- Zobacz logi ESP32 czy są błędy

## 💡 Porady

- **Mobilny dostęp**: Dashboard działa na telefonach i tabletach!
- **Wiele urządzeń**: Możesz otworzyć dashboard na wielu urządzeniach jednocześnie
- **Bookmark**: Zapisz adres IP jako zakładkę w przeglądarce
- **Static IP**: Możesz skonfigurować stały IP w routerze dla ESP32

## 🔐 Bezpieczeństwo

⚠️ **UWAGA**: To jest prosty dashboard bez autoryzacji!
- Każdy w sieci WiFi może zobaczyć dashboard
- Nie umieszczaj ESP32 w niezaufanych sieciach
- Dla produkcji dodaj autentykację (Basic Auth, hasło)

## 📊 Specyfikacja Techniczna

- **Protokół**: WebSocket (ws://)
- **Port**: 80 (HTTP)
- **Format danych**: JSON: `{"weight": 0.63, "raw": -81000}`
- **Częstotliwość**: Co 2 sekundy (konfigurowalny w `hx711_config.h`)
- **Max klientów**: 32 jednoczesne połączenia

## 🎯 Co dalej?

Możesz rozbudować dashboard o:
- Zapisywanie danych do pliku/bazy
- Alarmy przy przekroczeniu wagi
- Eksport danych do CSV
- Integrację z Home Assistant / Node-RED
- Wykresy historyczne (np. z 24h)
- REST API do odczytu danych

