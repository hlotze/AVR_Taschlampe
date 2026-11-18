# Schaltplan AVR-Taschenlampe

Der Schaltplan liegt im nativen KiCad-Format auf einem A4-Blatt im Querformat vor. Das Projekt wurde mit KiCad 10.0.8 erstellt.

- [KiCad-Projekt](AVR_Taschenlampe.kicad_pro)
- [Editierbarer Schaltplan](AVR_Taschenlampe.kicad_sch)
- [A4-Vorschau als SVG](preview/AVR_Taschenlampe.svg)
- [A4-Vorschau als PNG, 300 dpi](preview/AVR_Taschenlampe.png)
- [Elektrische Regelprüfung](erc.json)
- [Exportierte Netzliste](AVR_Taschenlampe.net)

Gleiche Netzbezeichnungen verbinden die räumlich getrennten Schaltungsteile elektrisch. Die Symbole und Beschriftungen sind gegenüber der ersten Fassung um etwa 30 % vergrößert. Die vergrößerten Symbole liegen in der Projektbibliothek `Taschenlampe.kicad_sym` und sind zusätzlich im Schaltplan eingebettet. Ihre Anschlussenden liegen weiterhin auf dem üblichen 50-mil-Raster. Die Bibliothekstabellen verwenden die Standardvariablen von KiCad 10.

## Aufbau

BT1 ist der Anschluss für drei AAA-Zellen in Reihe. SW1 (ON) schaltet ausschließlich den positiven Versorgungszweig. C1 (100 nF) gehört beim Layout unmittelbar an VCC und GND von U1; C2 (1 µF, ungepolt) puffert zusätzlich. Es gibt keine Selbsthaltung der Versorgung.

U1 ist ein ATtiny2313-20PU im DIP-20-Gehäuse. Das eingebettete KiCad-Symbol ATtiny2313V-10P besitzt dieselbe DIP-Pinbelegung; der Bestückungswert bezeichnet den vorgesehenen ATtiny2313-20PU. Der interne RC-Oszillator wird auf 1 MHz konfiguriert; ein externer Quarz ist nicht vorgesehen. Brown-out-Erkennung auf 2,7 V aktivieren, RESET und ISP aktiviert lassen, CKOUT deaktiviert lassen. Die konkreten Fuse-Bytes sind bei der Firmware-Erstellung anhand der tatsächlich eingesetzten Variante festzulegen.

D1 ist eine kleine RGB-LED mit gemeinsamer Anode. Q1–Q3 (BC547B) schalten deren Kathoden gegen Masse. Jede Farbe besitzt einen eigenen Vorwiderstand. Die Basispulldowns R8–R10 halten die Treiber auch während Reset und Programmierung ausgeschaltet. Die Firmware muss die PWM-Ausgänge vor der Aktivierung auf LOW setzen; ein HIGH-Pegel schaltet die jeweilige Farbe ein.

## Pinbelegung

| Funktion | Port | DIP-Pin |
|---|---|---:|
| Rot PWM | PB4 / OC1B | 16 |
| Grün PWM | PB3 / OC1A | 15 |
| Blau PWM | PB2 / OC0A | 14 |
| INC_rt | PD0 | 2 |
| DEC_rt | PD1 | 3 |
| INC_gn | PD2 | 6 |
| DEC_gn | PD3 | 7 |
| INC_bl | PD4 | 8 |
| DEC_bl | PD5 | 9 |
| STORE | PD6 | 11 |
| JP1 / SOLO | PB0 | 12 |
| RESET | PA2 | 1 |
| ISP MOSI | PB5 | 17 |
| ISP MISO | PB6 | 18 |
| ISP SCK | PB7 | 19 |
| VCC / GND | – | 20 / 10 |

Die sieben Bedientaster SW2–SW8 schalten gegen Masse. Interne Pull-ups aktivieren; gedrückt entspricht LOW. Entprellung, Wiederholung und die Behandlung gleichzeitig gedrückter Tasten erfolgen in der Firmware. Die ISP-Leitungen werden nicht durch Taster oder LED-Treiber belastet.

Timer 0 liefert Blau. Timer 1 liefert Grün und Rot und muss dafür im 8-Bit-PWM-Modus betrieben werden. Die Einstellung von PWM-Frequenz und Endwerten 0/255 ist Aufgabe der Firmware.

## JP1 / SOLO

JP1 besitzt zwei Pins: 1 = `SOLO_N` an U1/PB0 (DIP-Pin 12), 2 = GND. Der interne Pull-up wird in der Firmware aktiviert; es ist kein zusätzlicher Widerstand erforderlich. Offen bleibt die Mischung beim Einstellen sichtbar. Gesteckt werden während INC/DEC nur die bearbeiteten Farbkanäle angezeigt; nach Loslassen erscheint die Mischung wieder. Die RGB-Sollwerte bleiben erhalten, STORE speichert weiterhin alle drei Kanäle. Mehrere bearbeitete Kanäle sind gemeinsam sichtbar. Die Jumperstellung wird laufend ausgewertet. PB1 (Pin 13) bleibt unbeschaltet.

Für den Evaluationboard-Test PB0 am Portsplitter verwenden (Extension-Port J4, Pin 9) und JP1 gegen die gemeinsame Masse schalten. JP1 dieses Schaltplans ist nicht mit dem gleichnamigen UART-Jumper auf dem Pollin-Evaluationboard zu verwechseln.

## Bauteile und Dimensionierung

| Referenzen | Wert / Typ | Zweck |
|---|---|---|
| BT1 | 3 × AAA, nominell 4,5 V | Batteriehalteranschluss |
| SW1–SW8 | Schließertaster | ON und sieben Bedientaster |
| U1 | ATtiny2313-20PU | Steuerung, internes EEPROM |
| D1 | RGB, gemeinsame Anode | LED, konkreter Typ noch auszuwählen |
| Q1–Q3 | BC547B, C/B/E = Pins 1/2/3 | Kathodentreiber |
| R1 | 10 kΩ | RESET-Pull-up |
| R2–R4 | je 330 Ω, 0,25 W, 5 % oder besser | LED-Strombegrenzung |
| R5–R7 | je 1 kΩ | Basisstrombegrenzung |
| R8–R10 | je 100 kΩ | Basis-Emitter-Pulldown |
| C1 | 100 nF, Keramik, mindestens 10 V | Abblockung |
| C2 | 1 µF, Keramik, mindestens 10 V | Versorgungspuffer |
| J1 | AVR-ISP, 2 × 3, 2,54 mm | Programmierung |
| JP1 | Jumper, 1 × 2, 2,54 mm | Einzelkanalanzeige: PB0 nach GND |

Die 330-Ω-Widerstände sind konservative Startwerte für eine kleine Anzeige-LED, keine Auslegung für eine Leistungs-LED. Beispielrechnung bei 4,5 V und angenommenen 0,2 V Transistorspannung: Rot mit 2,0 V Durchlassspannung etwa 7 mA, Grün/Blau mit 3,0 V etwa 4 mA. Die tatsächlichen Ströme und die optische Weißmischung hängen vom LED-Typ ab; RGB (127,127,127) garantiert kein farbmetrisches Weiß.

Vor Bestückung sind LED-Typ, zulässige Ströme und Pinfolge festzulegen. Das Symbol nimmt 1 = Rot-Kathode, 2 = gemeinsame Anode, 3 = Grün-Kathode und 4 = Blau-Kathode an. Deshalb hat D1 noch keinen verbindlichen Footprint. Auch der Batteriehalter benötigt nach mechanischer Auswahl einen Footprint oder Anschluss. Die übrigen Footprints sind Vorschläge für bedrahtete Bauteile.

Der Mikrocontroller arbeitet bei 1 MHz im vorgesehenen Spannungsbereich; für den Entwurf wird eine Versorgung oberhalb der Brown-out-Schwelle bis etwa 4,8 V angenommen. Bei sinkender Batteriespannung nimmt die LED-Helligkeit ab; Grün und Blau können früher ausfallen. Eine geregelte, batteriezustandsunabhängige Helligkeit ist nicht vorgesehen.

## ISP und EEPROM

J1: 1 = MISO, 2 = VCC_SW, 3 = SCK, 4 = MOSI, 5 = RESET, 6 = GND. Beim Programmieren ON gedrückt halten. Der Programmer muss die Zielspannung berücksichtigen und darf die Schaltung nicht zusätzlich versorgen. Mit langsamem ISP-Takt beginnen, beispielsweise 125 kHz bei 1 MHz CPU-Takt.

Es gibt keinen Versorgungspuffer, der einen vollständigen EEPROM-Schreibvorgang nach Loslassen von ON garantiert. Die Firmware muss deshalb die vereinbarte Behandlung unterbrochener Schreibvorgänge umsetzen: pro Kanal einen prüfbaren Datensatz vorsehen, gültige alte/neue Werte zulassen und ungültige Werte durch 127 ersetzen. Eine Integritätsprüfung erkennt Fehler nur entsprechend ihrer Prüfstärke; eine absolute Erkennung beliebiger Beschädigungen ist nicht garantiert. Die Schaltung allein stellt diese Softwareanforderung nicht sicher.

## Prüfung und Grenzen

KiCad ERC: **0 Fehler und 0 Warnungen**, ohne ausgeschlossene Meldungen. Die exportierte Netzliste wurde mit `python3 tools/check_netlist.py` aus dem Projektverzeichnis gegen die beabsichtigten Pin-Netz-Zuordnungen geprüft. Die A4-Vorschau wurde visuell kontrolliert. Der Anwender hat am 16.11.2026 INC/DEC, STORE mit Doppelblinken und die Wiederherstellung nach RESET am Evaluationboard bestätigt. Die am 17.11.2026 ergänzte JP1-Funktion ist noch nicht am Aufbau getestet; elektrische Messungen und eine Schaltungssimulation stehen ebenfalls aus. Ein Platinenlayout ist nicht Bestandteil dieses Schaltplans.

Quellen: [Microchip ATtiny2313/V Datenblatt](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2543-AVR-ATtiny2313_Datasheet.pdf), [onsemi BC547-Datenblatt](https://www.onsemi.com/pdf/datasheet/bc546-d.pdf).
