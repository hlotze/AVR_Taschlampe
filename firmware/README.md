# Firmware der AVR-Taschenlampe

Firmware für **ATtiny2313**, passend zur Pinbelegung im Schaltplan. Sie erzeugt drei Hardware-PWM-Signale für die NPN-Treiber, wertet die sieben Taster aus und speichert RGB-Werte im internen EEPROM. Helligkeitsänderung je Farbkanal, STORE mit Doppelblinken und Wiederherstellung der gespeicherten Farbe nach RESET wurden am 16.11.2026 vom Anwender auf dem Evaluationboard erfolgreich getestet.

## Programmierstand vom 16.11.2026

Der ATtiny2313 auf dem Pollin-Evaluationboard wurde über den vorhandenen ISP-Programmer programmiert (Protokoll `stk500v2`). Die damals aktuelle Firmware einschließlich des Grenzstopps für INC/DEC wurde übertragen: **1108 Byte Flash und 9 Byte EEPROM erfolgreich zurückgelesen und verifiziert**. Startwerte: RGB (127,127,127).

Erkannte Signatur: `1E 91 0A`. Vorhandene Fuses: LFUSE `64`, HFUSE `DF`, EFUSE `FF`; sie wurden nicht verändert. Vorheriger Flash- und EEPROM-Inhalt sowie die Fuses sind unter `backups/2026-11-16-vor-programmierung/` gesichert. Der Anwender hat die Helligkeitsänderung je Farbkanal über INC/DEC, das Speichern mit Doppelblinken und die Wiederherstellung der zuletzt gespeicherten Farbe nach RESET an der angeschlossenen Schaltung erfolgreich bestätigt. Weitere noch offene Hardwaretests sind unten aufgeführt.

## Erweiterung vom 17.11.2026: JP1 / SOLO

Die neu gebaute Firmware unterstützt JP1 an PB0 (Pin 12) zur Einzelkanalanzeige beim Einstellen. Die neue Version wurde am 17.11.2026 auf den ATtiny2313 übertragen: **1194 Byte Flash erfolgreich verifiziert**. Der zuvor gesicherte vollständige EEPROM-Inhalt (128 Byte) wurde wiederhergestellt und verifiziert. Die gespeicherten RGB-Werte **(63, 3, 255)** blieben erhalten; die Fuses wurden nicht verändert. Sicherung: `backups/2026-11-17-vor-solo-update/`. Der praktische Test der JP1-Funktion steht noch aus. Der Programmierstand vom 16.11.2026 oben bezeichnet die zuvor getestete Version. Das EEPROM-Format bleibt unverändert; beim späteren Update sollen die gespeicherten Einstellungen erhalten bleiben. Dazu vor dem Flashen EEPROM sichern und nach einem gegebenenfalls löschenden Chip-Erase wiederherstellen; die Initialdatei `.eep` setzt die Einstellungen auf 127 zurück.

## Grenzwertkorrektur vom 17.11.2026

Die Firmware erreicht jetzt mit DEC genau 0 und mit INC genau 255. Der letzte Schritt wird bei Bedarf verkürzt; die Gegenrichtung beginnt am Endwert wieder mit einem vollen Schritt von 4. Die Korrektur wurde nach erfolgreichen Builds und Tests aller vier Takt-/PWM-Konfigurationen auf den ATtiny2313 übertragen: **1194 Byte Flash verifiziert**. Der zuvor gesicherte EEPROM-Inhalt wurde vollständig wiederhergestellt und verifiziert (128 Byte); die Fuses blieben unverändert. Sicherung: `backups/2026-11-17-vor-grenzwert-update/`. Der praktische Test der geänderten Endwerte steht noch aus.

## Schrittrückmeldung vom 17.11.2026

Die Testversion mit 15-ms-Dunkelimpulsen wurde auf den angeschlossenen ATtiny2313 übertragen: **1306 Byte Flash und 128 Byte wiederhergestelltes EEPROM verifiziert**. Alle vier unterstützten Takt-/PWM-Konfigurationen bestehen Build und Logiktests. Die gespeicherten Farbwerte blieben erhalten; die Fuses wurden nicht verändert. Sicherung: `backups/2026-11-17-vor-15ms-update/`. Die optische Beurteilung der kurzen Dunkelimpulse am Aufbau steht noch aus.

## Bauen

Benötigt werden AVR-GCC mit AVR-LibC, AVR-Binutils und Make. Das Makefile verwendet AVR-Werkzeuge aus `PATH` oder die bereits installierte PlatformIO-Toolchain unter `~/.platformio/packages/toolchain-atmelavr/bin`.

Aus dem Projektverzeichnis:

```sh
make -C firmware all
make -C firmware test
```

Eine andere Installation kann mit `AVR_PREFIX=/pfad/zum/bin/avr-` angegeben werden. Die Tests laufen mit dem lokalen C-Compiler und benötigen für AddressSanitizer und UndefinedBehaviorSanitizer einen passenden Clang/GCC.

Erzeugte Dateien:

| Datei | Verwendung |
|---|---|
| `build/taschenlampe.hex` | Programm für den Flash-Speicher |
| `build/taschenlampe.eep` | Initialwerte und Prüfinformationen für das EEPROM |
| `build/taschenlampe.elf` | Firmware einschließlich Debug- und Speicherinformationen |
| `build/taschenlampe.map` | Speicherbelegung des Linkers |

Gemessene Belegung der Standardkonfiguration: **1306 von 2048 Byte Flash**, **23 von 128 Byte statischem RAM**, **9 von 128 Byte EEPROM**. Der Stack benötigt zusätzlich RAM; es wird kein Heap verwendet.

## Takt und PWM

Die Firmware erwartet, dass die Clock-Fuses den **internen 8-MHz-RC-Oszillator** auswählen. Sie stellt den CPU-Teiler selbst auf 8 und läuft damit standardmäßig mit **1 MHz**. `F_CPU` beschreibt diesen tatsächlichen CPU-Takt, nicht die ungeteilte Oszillatorfrequenz. Bei einem Evaluationboard mit externem Quarz muss zunächst die Taktkonfiguration geprüft werden; diese Version ist dafür nicht konfiguriert.

Die Firmware schreibt keine Fuses. Für den späteren Batteriebetrieb sind Brown-out-Erkennung bei 2,7 V, freigegebener RESET-/ISP-Anschluss und deaktiviertes CKOUT vorgesehen. WDTON muss unprogrammiert sein, damit sich der Watchdog abschalten lässt. Konkrete Fuse-Bytes erst anhand des tatsächlich eingesetzten Tiny und seiner bestehenden Konfiguration festlegen.

Timer 0 und Timer 1 laufen im nichtinvertierenden, phasenkorrekten 8-Bit-PWM-Modus. HIGH schaltet den NPN-Treiber ein; die Schaltung ist nicht für einen direkten Anschluss der LED-Kathoden an die MCU-Ausgänge ausgelegt. Die Endwerte 0 und 255 erzeugen dauerhaft LOW beziehungsweise HIGH.

Standard: `F_CPU=1000000UL`, `PWM_PRESCALER=1`, daraus **etwa 1961 Hz** auf allen drei Kanälen. Die Tasterzeitbasis wird vom Timer-0-Overflow-Interrupt abgeleitet. Das Abtastintervall beträgt etwa 5,1 ms:

| Verhalten | Tatsächliche nominale Zeit bei Standardkonfiguration |
|---|---:|
| Entprellung | etwa 20,4 ms |
| Erste Wiederholung nach erkanntem Tastendruck | etwa 499,8 ms |
| Weitere Wiederholungen | etwa 102 ms |
| Blinkphase an / aus | jeweils etwa 198,9 ms |

Hinzu kommen die Toleranz des internen RC-Oszillators und geringe Laufzeitabweichungen. Änderungen sind beispielsweise möglich mit:

```sh
make -C firmware all F_CPU=8000000UL PWM_PRESCALER=8
```

Unterstützt werden CPU-Takte 1 und 8 MHz sowie PWM-Teiler 1 und 8. Die Zeitbasis wird aus diesen Einstellungen berechnet. Beide Timer werden vollständig von dieser Firmware verwendet.

## Anschluss am Evaluationboard

| Funktion | Port | DIP-Pin |
|---|---|---:|
| Rot PWM über R5 an Q1 | PB4 / OC1B | 16 |
| Grün PWM über R6 an Q2 | PB3 / OC1A | 15 |
| Blau PWM über R7 an Q3 | PB2 / OC0A | 14 |
| INC_rt / DEC_rt | PD0 / PD1 | 2 / 3 |
| INC_gn / DEC_gn | PD2 / PD3 | 6 / 7 |
| INC_bl / DEC_bl | PD4 / PD5 | 8 / 9 |
| STORE | PD6 | 11 |
| JP1 / SOLO gegen GND | PB0 | 12 |

Beim Wechsel von der bisherigen Pinbelegung die Leitungen für Rot und Blau am AVR tauschen. Rot geht jetzt an Pin 16 und Blau an Pin 14. Bei einem bereits programmierten ATtiny genügt dafür ein Flash-Update, der EEPROM-Inhalt kann beibehalten werden.

Die Firmware aktiviert die internen Pull-ups an PD0–PD6 und PB0. JP1 verbindet PB0 mit gemeinsamer Masse; ein zusätzlicher Pull-up-Widerstand ist nicht erforderlich. Auf dem Pollin-Board liegt PB0 am Extension-Port J4, Pin 9 (am Portsplitter die passende PB0-Beschriftung beachten). PB1 bleibt frei.

Die sieben Taster liegen an PD0–PD6. Jeder Taster verbindet den jeweiligen Eingang mit gemeinsamer Masse; gedrückt bedeutet LOW. Am Evaluationboard müssen diese Pins ohne konkurrierende Beschaltung über die Portsplitter verfügbar sein.

Der Tiny bleibt ausschließlich über die 5-V-Versorgung des Evaluationboards versorgt. Die LED-Treiber dürfen über die separate 4,5-V-Batterie laufen. Beide GND verbinden; Batterie-Plus und Board-5-V bleiben getrennt. Der ISP-Anschluss J1 auf der Taschenlampenschaltung wird für diesen Test nicht benötigt.

Beim Einschalten beziehungsweise MCU-Reset werden gehaltene Tasten erst nach einem entprellten Loslassen akzeptiert. Da ON am Steckbrett die Versorgung des Tiny auf dem Evaluationboard nicht schaltet, erzeugt ON dort **keinen Neustart und kein erneutes Einlesen des EEPROM**. Das vollständige Einschaltverhalten lässt sich mit Reset am Evaluationboard oder später am eigenständig versorgten Tiny prüfen.

## Bedienung

INC/DEC verändert den jeweiligen Kanal unmittelbar nach der Entprellung um ±4. Der letzte Schritt wird bei Bedarf auf 0 beziehungsweise 255 begrenzt: INC setzt 253 auf 255, DEC setzt 2 auf 0. Damit sind vollständig aus und vollständig an von jedem Startwert aus erreichbar. Am Endwert ändern Halten oder erneutes Drücken derselben Taste nichts mehr. Die entgegengesetzte Taste arbeitet von dort mit ±4 weiter: 0 → 4 beziehungsweise 255 → 251. Es gibt keinen Umlauf. Gedrückthalten wiederholt die Aktion nach etwa 500 ms und danach etwa alle 100 ms. Gleichzeitiges INC und DEC eines Kanals unterdrückt dessen Änderung; andere Kanäle bleiben bedienbar. Wenn anschließend nur noch eine Taste dieses Paars gehalten wird, beginnt für diese Richtung eine neue Aktion mit neuer Wiederholverzögerung.

STORE übernimmt den aktuellen RGB-Stand. Im gleichen Abtastschritt erkannte Änderungen werden zuvor angewendet. Nur geänderte oder ungültige EEPROM-Kanäle werden geschrieben. Bei identischen gültigen Daten gibt es keinen Schreibvorgang und keine Blinkbestätigung.

Nach abgeschlossener und geprüfter Speicherung blinkt die LED zweimal mit RGB (127,127,127), jeweils ungefähr 200 ms an und 200 ms aus. Anschließend erscheint die eingestellte Farbe wieder. RGB (127,127,127) entspricht der vereinbarten Weißbestätigung; die tatsächliche Farbe hängt von LED und Vorwiderständen ab. Während Schreiben und Bestätigung werden alle Taster ignoriert; danach ist erneut ein entprelltes Loslassen erforderlich. Bei fehlgeschlagener EEPROM-Rückleseprüfung bleibt die Bestätigung aus; nach Loslassen kann STORE erneut versucht werden.

## Dunkelimpuls als Schrittrückmeldung

Jede tatsächliche INC-/DEC-Wertänderung blendet den betroffenen Kanal kurz aus, auch bei Tastenwiederholung. Standard: `STEP_DARK_MS = 15` in `src/config.h`. Bei der Standardzeitbasis sind das drei Abtastintervalle, nominal **15,3 ms**, zuzüglich der PWM-Aktualisierung und Oszillatortoleranz. Danach wird der neue Helligkeitswert ausgegeben. Andere Farbkanäle laufen weiter; bei SOLO ist entsprechend kurz die ganze LED dunkel, sofern kein weiterer ausgewählter Kanal leuchtet.

Pro Kanal wird ein nichtblockierender Zähler verwendet. Die tatsächlichen RGB-Werte bleiben während des Impulses erhalten und werden unverändert gespeichert. Bei unverändertem Endwert erfolgt kein Impuls. STORE beendet noch laufende Dunkelimpulse vor dem Schreiben und hat mit seiner Doppelblinkbestätigung Vorrang. Die bestehende Tastenfreigabe nach STORE bleibt erhalten.

## Einzelkanalanzeige mit JP1

JP1 offen: Alle Farbkanäle bleiben während der Einstellung sichtbar. JP1 gesteckt (PB0 nach GND): Nur Kanäle mit entprellten und freigegebenen INC-/DEC-Tasten sind sichtbar. Mehrere gleichzeitig bearbeitete Kanäle werden zusammen ausgegeben. INC und DEC desselben Kanals gemeinsam zeigen diesen Kanal unverändert. Auch an einem Grenzstopp bleibt nur der ausgewählte Kanal sichtbar; bei dessen Wert 0 bleibt die LED entsprechend dunkel.

Nach entprelltem Loslassen der letzten Einstelltaste erscheint wieder die Mischung mit den neuen Werten. Beim Einschalten beziehungsweise nach STORE blockierte Tasten lösen keine Einzelanzeige aus, solange sie nicht freigegeben wurden. Der Jumper wird bei jedem Bedienzyklus eingelesen, lässt sich also auch während des Haltens umstecken; die Jumperauswertung selbst hat keine zusätzliche Entprellung.

`control_display()` berechnet ausschließlich die sichtbaren PWM-Werte. Die tatsächlichen Werte in `Control.level` bleiben davon unabhängig, sodass STORE alle drei Werte einschließlich ausgeblendeter Kanäle speichert. Die Blinkbestätigung übersteuert die Einzelanzeige und stellt anschließend die volle Mischung wieder her. Die Jumperstellung wird nicht im EEPROM gespeichert.

## EEPROM initial programmieren

Beim ersten Programmieren auf dem Evaluationboard sind **zwei getrennte Speicherbereiche** zu beschreiben:

1. `build/taschenlampe.hex` in den Flash-Speicher.
2. `build/taschenlampe.eep` in das EEPROM, anschließend jeweils verifizieren.

Im Programmierwerkzeug als Zielgerät ATtiny2313 und den tatsächlich verwendeten Programmer auswählen. Ein Flash-Schreibvorgang allein überträgt die EEPROM-Initialwerte nicht automatisch. Diese Anleitung führt selbst keine Programmierung aus.

Die EEPROM-Datei enthält die Startwerte **127/127/127** einschließlich ihrer Prüfinformationen. Sie besteht aus neun Bytes an den Adressen 0–8:

```text
Adresse 0–2:  7F 80 A5   Rot
Adresse 3–5:  7F 80 A5   Grün
Adresse 6–8:  7F 80 A5   Blau
```

Pro Kanal werden Helligkeit, bitweise invertierte Helligkeit und eine Gültigkeitsmarkierung (`A5`) gespeichert. Deshalb nicht lediglich drei aufeinanderfolgende Bytes mit 127 beschreiben. Ein leerer oder ungültiger Kanal liefert beim Start ebenfalls 127, wird dabei jedoch nicht automatisch ins EEPROM zurückgeschrieben.

Bei Änderungen wird zuerst die Markierung ungültig gemacht, anschließend werden Wert und Gegenwert geschrieben und erst zuletzt wird der Kanal wieder gültig markiert. Nach einem Spannungsverlust können dadurch alte, neue oder ersatzweise 127-Werte vorliegen. Die Speicherung ist bewusst nicht atomar über alle drei Kanäle. Die Gegenwertprüfung erkennt einzelne Bitfehler, garantiert aber nicht die Erkennung beliebiger kombinierter Speicherfehler. Brown-out-Erkennung unterstützt den zuverlässigen Betrieb bei absinkender Versorgung.

Erneutes Schreiben der `.eep`-Datei setzt gespeicherte Einstellungen zurück. Auch ein Chip-Erase kann das EEPROM abhängig von EESAVE löschen. Für spätere Firmware-Updates deshalb bewusst entscheiden, ob Benutzereinstellungen erhalten oder initialisiert werden sollen.

## Prüfung und erster Hardwaretest

Automatisiert geprüft wurden die unveränderten Produktionsmodule für Taster und EEPROM: Entprellung, gehaltene Starttasten, erneute Freigabe, Wiederholung, Grenzstopps für alle 256 Startwerte auf allen drei Kanälen einschließlich Halten, erneutem Drücken und Richtungswechsel, gleichzeitige Tasten, STORE-Ereignisse, Dunkelimpulsdauer und automatische Rückkehr, Impulse bei Wiederholung, keine Impulse am unveränderten Endwert, Speicherung während eines Impulses, Einzelkanalanzeige bei offenem/gestecktem Jumper, mehreren Kanälen, Grenzstopps und Richtungswechseln, unveränderte gespeicherte Werte trotz ausgeblendeter Kanäle, Ersatzwerte, Schreibunterdrückung, einzelne Bitfehler und **3084 simulierte Spannungsabbrüche** einschließlich aller möglichen Werte eines unterbrochenen Schreibbytes. Die Tests laufen mit AddressSanitizer und UndefinedBehaviorSanitizer. Alle vier unterstützten Kombinationen aus CPU-Takt und PWM-Teiler wurden kompiliert und auf dieser Logikebene getestet.

Die Host-Tests simulieren keine AVR-Register, PWM-Wellenformen, Interruptlaufzeiten oder reale Spannungsabfälle.

Am 16.11.2026 hat der Anwender auf dem Pollin-Evaluationboard mit angeschlossener Schaltung folgenden Ablauf erfolgreich geprüft:

- Helligkeit der einzelnen Farbkanäle über INC und DEC ändern.
- Aktuelle Einstellung mit STORE speichern; die LED bestätigt mit Doppelblinken.
- RESET am Evaluationboard auslösen; danach erscheint die zuletzt gespeicherte Farbe wieder.

Noch gesondert auf Hardware zu prüfen sind:

- Neue Einzelkanalanzeige mit JP1: offen/gesteckt, Loslassen, mehrere Kanäle und STORE während der Einzelanzeige.

- Tastenwiederholung, Grenzstopps und gleichzeitig gedrückte Tasten.
- Änderungen ohne STORE und anschließender Reset: Der vorher gespeicherte Stand muss wieder erscheinen.
- Verhalten bei vollständigem Aus- und Einschalten des Tiny sowie bei unterbrochenem Speichern.
- Bei Bedarf PWM-Frequenz und Einschaltverhalten mit Oszilloskop oder Logikanalysator messen.

## Quelltexte und Quellen

- `src/main.c`: Pininitialisierung, PWM, Zeitbasis, AVR-EEPROM-Zugriff und Ablauf.
- `src/control.c`: Entprellung, Tastenfreigabe, Helligkeitseinstellung und Auswahl der sichtbaren Kanäle.
- `src/storage.c`: EEPROM-Format, Gültigkeitsprüfung und geordnetes Speichern.
- `src/config.h`: Takt, PWM-Teiler und Bedienzeiten.
- `tests/test_firmware.c`: Host-Tests mit simuliertem EEPROM.

Grundlagen: [Microchip ATtiny2313/V-Datenblatt](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2543-AVR-ATtiny2313_Datasheet.pdf), [AVR-LibC EEPROM-Funktionen](https://avrdudes.github.io/avr-libc/avr-libc-user-manual/group__avr__eeprom.html).
