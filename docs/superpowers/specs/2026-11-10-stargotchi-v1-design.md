# StarGotchi v1 – Design-Dokument

Stand: 2026-11-10 · Status: zur Prüfung

## 1. Ziel

Ein virtuelles Haustier („Tamagotchi-artig") im Weltraum-Abenteuer-Stil für den
**Waveshare ESP32-S3-Touch-AMOLED-1.75** (mit Gehäuse, ohne GPS). Komplett eigene
Firmware, eigene Pixel-Grafiken, eigener Web-Installer über GitHub Pages.
Das fertige Projekt wird auf **MakerWorld** veröffentlicht.

Erfolg heißt:
- Jemand mit dem Board öffnet die Installer-Seite in Chrome/Edge, klickt „Install"
  und hat ein laufendes StarGotchi – ohne Programmierkenntnisse.
- Der Bot lässt sich über Tage pflegen, entwickelt sich sichtbar weiter und
  übersteht Neustarts und leeren Akku.

### Rahmenbedingungen
- Der Nutzer programmiert nicht selbst. Claude schreibt Code, Grafiken, Sounds,
  Installer und MakerWorld-Texte; der Nutzer testet auf dem Gerät.
- Öffentliche Veröffentlichung: **keine** geschützten Namen, Logos oder Figuren
  (kein „Star Wars", kein „Droid", keine Original-Charaktere). Nur eigener Stil,
  der an Weltraum-Saga-Ästhetik erinnert. Im Spiel heißen die Wesen „Bots"/„Units".
- Name: **StarGotchi** (bewusste Entscheidung des Nutzers).
- Texte auf dem Gerät: **Englisch**.

## 2. Hardware

| Bauteil | Chip | Nutzung in v1 |
|---|---|---|
| MCU | ESP32-S3R8, 8 MB PSRAM, 16 MB Flash | ja |
| Display | 1.75" AMOLED rund, 466×466, CO5300 (QSPI) | ja |
| Touch | CST9217 (I2C) | ja |
| Power-Management | AXP2101 | Akkustand, Ladeerkennung |
| Uhr | PCF85063 RTC | Zeit für Werte-Abbau & Tag/Nacht |
| Bewegungssensor | QMI8658 (6-Achsen) | Steuerung im Minispiel |
| Audio | ES8311 Codec + Lautsprecher | Piep-/Bloop-Sounds |
| Mikrofone, SD-Karte | – | nicht genutzt in v1 |

Pinbelegung und Treiber werden aus Waveshares offiziellen Beispielen für dieses
Board übernommen.

## 3. Spielablauf

### 3.1 Werte (je 0–100)
| Wert | Anzeige | Entspricht | Aktion | Effekt |
|---|---|---|---|---|
| Öl | OIL (orange) | Hunger | Ölkanne | +30 Öl |
| Laune | MOOD (koralle) | Freude | Gamepad → Minispiel | +Laune je nach Ergebnis, −Ladung |
| Ladung | PWR (türkis) | Energie/Schlaf | Akku mit Blitz → Dock | Bot schläft, Ladung steigt |
| Zustand | COND (blau) | Hygiene | Maulschlüssel → Reparieren | +40 Zustand, Rost/Staub weg |

- Die Werte sinken mit der echten Zeit (RTC), auch wenn das Gerät aus war.
  Beim Start wird die verstrichene Zeit nachgerechnet.
- Startwerte für die Abbauraten (pro Stunde, wach): Öl −4, Laune −5,
  Ladung −3, Zustand −2. Im Schlaf: Öl/Laune/Zustand halbiert, Ladung +10.
  Die Raten sind zentrale Konstanten und werden nach Tests feinjustiert.

### 3.2 Tag/Nacht
- Nacht = 22:00–07:00: Bot schläft automatisch, Hintergrund mit Mond/Sternen.
- Beim allerersten Start wird die Uhrzeit eingestellt (Setup-Seite).

### 3.3 Entwicklung

**Update 2026-11-11:** Beim Start wird zufällig eine von drei eigenen Units gewählt –
**Crawler** (vierbeinig), **Wheeler** (Einrad) oder **Hover** (schwebend). Jede hat eigene
Grafiken für Mini, Junior und die drei Endformen; die Regeln unten gelten für alle drei.
Der Speicherstand (Version 2) merkt sich die Unit, alte Stände laden als Crawler.

1. **Kern** – leuchtender Energiekern, „bootet" (Start)
2. **Mini-Bot** – nach ca. 1 Tag
3. **Junior-Bot** – nach ca. 3 Tagen
4. **Endform** – nach ca. 7 Tagen, bestimmt durch den Pflege-Verlauf:
   - **Nova-Bot**: ausgewogen gute Pflege (Durchschnitt aller Werte hoch)
   - **Kampf-Bot**: überdurchschnittlich viel Spielen/Training
   - **Schrott-Bot**: vernachlässigt (Durchschnitt niedrig) – schief, aber liebenswert
- Level/XP steigen durch Pflegeaktionen und Minispiel-Punkte.

### 3.4 Vernachlässigung
- Sind mindestens zwei Werte 8 Stunden lang auf 0 → **Notabschaltung**.
- Kein „Tod": Der Bot kann neu hochgefahren werden, verliert aber Level-Fortschritt
  (Level −50 %, Entwicklungsstufe bleibt).

## 4. Oberfläche

### 4.1 Look
- Pixel-Art, Retro-Stil, schwarzer Weltraum-Hintergrund (spart Akku auf AMOLED).
- Akzentfarben: Holo-Blau und Warnorange („Raumschiff-Konsole").
- Animationen: Blinzeln, Wackeln, „Zzz" beim Schlafen, Funken/Rost bei niedrigen Werten.

### 4.2 Seiten (wischen links/rechts, Punkte-Anzeige unten)
1. **Home** (Mitte, Standard)
   - Oben: Name + Level, darunter Status-Text (z. B. „Systems nominal", „Low oil!")
   - Rand links: OIL (oben), MOOD (unten); Rand rechts: PWR (oben), COND (unten).
     Bögen liegen direkt am Bildschirmrand, mit dunkler Spur + farbiger Füllung.
   - Mitte: Bot auf einer Plattform
   - Unten: 4 Icon-Knöpfe (keine Schrift): Ölkanne, Gamepad, Akku mit Blitz,
     Maulschlüssel (offene Backen an beiden Enden)
2. **Stats**: Level, Alter, Entwicklungsstufe, Pflege-Verlauf
3. **Setup**: Uhrzeit, Helligkeit, Ton an/aus, Neustart (mit Bestätigung)

### 4.3 Minispiel „Asteroid Run"
- Start über das Gamepad-Icon. Der Bot fliegt durch ein Asteroidenfeld;
  Steuerung durch Neigen des Geräts (QMI8658).
- Ca. 30–60 Sekunden; Punkte → Laune + XP, kostet Ladung.

### 4.4 Display-Verhalten
- Nach 30 s ohne Berührung dimmen, danach Display aus. Tippen weckt.

## 5. Technik

### 5.1 Werkzeuge
- **Arduino-Framework (ESP32 Arduino Core 3.3.10) + LVGL 8.4.0**, gebaut mit **arduino-cli** –
  exakt die Kombination, mit der Waveshare die Beispiele für dieses Board in seiner CI baut
  (geringstes Risiko, dass Display/Touch nicht laufen).
- Repository: `github.com/Skinner2002/stargotchi` (öffentlich).

### 5.2 Bausteine der Firmware
| Baustein | Aufgabe | Hängt ab von |
|---|---|---|
| `hal/` | Display, Touch, RTC, IMU, Audio, Akku – dünne Schnittstellen | Hardware |
| `game/` | Werte, Abbau, Entwicklung, Tag/Nacht, Notabschaltung – **reine Logik ohne Hardware** | nichts |
| `ui/` | Seiten, Minispiel, Animationen (LVGL) | `game/`, `hal/` |
| `storage/` | Speicherstand sichern/laden (NVS) mit Versionsnummer + Prüfsumme | `game/` |
| `assets/` | Pixel-Grafiken & Sounds, aus PNG/WAV automatisch in C-Arrays umgewandelt | – |

- Die Spiel-Logik bekommt die aktuelle Zeit von außen übergeben, damit sie ohne
  Gerät getestet werden kann.

### 5.3 Speichern
- Speicherstand jede Minute und nach jeder Aktion.
- Speicherstand hat Versionsnummer + Prüfsumme. Ist er ungültig → neuer Kern
  statt Absturz.
- Updates über den Web-Installer löschen den Speicherstand **nicht**.

### 5.4 Build & Web-Installer
1. Jede Änderung auf GitHub startet **GitHub Actions**:
   Tests der Spiel-Logik (mit g++ auf dem Build-Server) → Firmware bauen.
2. Die Firmware landet zusammen mit einer **ESP-Web-Tools**-Installer-Seite
   auf **GitHub Pages**: `https://skinner2002.github.io/stargotchi/`
3. Nutzer: Gerät per USB an PC, Seite in Chrome/Edge öffnen, „Install" klicken.
4. Die Seite zeigt die aktuelle Versionsnummer.

### 5.5 Fehlerbehandlung
- Uhr nicht gesetzt / RTC-Zeit unplausibel → Setup fordert Uhrzeit an.
- Speicherstand beschädigt → neuer Kern.
- Sehr lange Pause (z. B. Wochen) → Abbau wird begrenzt nachgerechnet
  (Werte gehen höchstens auf 0, Notabschaltung greift regulär).

## 6. Testen
- **Automatisch** (bei jedem Build): Spiel-Logik – Abbau, Nachrechnen von
  Offline-Zeit, Entwicklungsregeln, Notabschaltung, Speicherstand-Prüfung.
- **Auf dem Gerät** (Nutzer): Anzeige, Touch, Minispiel-Gefühl, Sound, Akku.
  Rückmeldung per Foto/Beschreibung.

## 7. Nicht in v1 (spätere Updates)
- Mehrere Bots sammeln („Crew" bis 6)
- Erkunden / Schrottsuche
- Eigenes 3D-gedrucktes Gehäuse
- Mehrsprachigkeit
- Alternative Tipp-Steuerung fürs Minispiel (vorerst verworfen: Display zu klein)
