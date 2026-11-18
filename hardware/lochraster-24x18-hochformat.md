# Lochrasterplan AVR-Taschenlampe · Hochformat (Entwurf)

[Bestückungsseite, Lötseite und Bestückungsseite mit Verdrahtung](lochraster-24x18-hochformat.svg)

Die Platine hat 18 Spalten und 24 Zeilen. Der Plan setzt **einzelne, elektrisch getrennte Lötaugen** voraus. Der angegebene Lochabstand beträgt 2,56 mm. Die Lochmitten am Rand liegen damit 43,52 × 58,88 mm auseinander. DIP-Sockel und Stiftleisten mit 2,54-mm-Nennraster vor dem Löten trocken einstecken.

Alle Koordinaten sind als **Spalte/Zeile** angegeben. Von oben gesehen liegt **01/01 links oben**. Auf der gespiegelten Lötseite liegt **01/01 rechts oben**. Die Spaltennummern in beiden Ansichten bezeichnen dieselben Löcher.

## In KiCad bearbeiten

Das [editierbare KiCad-Board](lochraster-24x18-hochformat.kicad_pcb) im PCB-Editor öffnen. Zum Verschieben der Bauteile ein benutzerdefiniertes X/Y-Raster von 2,56 mm wählen. Der Rasterursprung liegt auf 01/01.

- **User.Drawings** zeigt die 18 × 24 Lochpositionen. **F.Fab** zeigt die Bauteilumrisse und Referenzen.
- **User.Comments** enthält den bisherigen Verlauf der isolierten Drahtbrücken. Die Segmente jeder Brücke sind gruppiert. Das Board zeigt sie von oben, wie in der dritten Ansicht der Zeichnung.
- Die Bauteilpads tragen die elektrischen Netze. Für die Entflechtung kann man die Drahtgruppen umzeichnen oder die Netzlinien als Ausgangspunkt für den Leiterbahnrouter verwenden. KiCad meldet die Drahtbrücken im DRC als offene Kupferverbindungen, weil sie auf einer Zeichnungsebene liegen. Die Footprints sind im Board eingebettet und lassen sich dort bearbeiten.

## Bestückung

- BT1 sind zwei Lötpunkte für die Kabel des Batteriehalters. Pin 1 ist +4,5 V (06/24), Pin 2 ist GND (13/24).
- SW1 (ON) belegt ein Feld von 5 × 5 Löchern. SW2–SW8 (INC, DEC und STORE) belegen je ein Feld von 3 × 3 Löchern. Der Plan setzt voraus, dass die Beinchen in den Ecken sitzen und die beiden oberen Lötpunkte beim Drücken miteinander verbunden werden. Taster ausrichten und diese Kontaktzuordnung vor dem Löten mit dem Durchgangsprüfer bestätigen. Weiß umrandete Zusatzbeinchen sind in der Drahtliste als Anschlüsse genutzt. Grau gefüllte Zusatzbeinchen bleiben ohne Draht.
- R2–R10 werden stehend bestückt. Die Basisvorwiderstände R5–R7 liegen in der Bestückungsansicht unter der jeweiligen Basis, die Basis-Emitter-Pulldowns R8–R10 rechts daneben unter dem jeweiligen Emitter. Die LED-Vorwiderstände R2–R4 liegen senkrecht direkt unter dem jeweiligen Kollektor (Zeilen 4 und 5). Die Transistoren sitzen in Zeile 3 und R5–R7 darunter in den Zeilen 5 und 6 und R8–R10 in den Zeilen 4 und 5. Ihre Anschlusslöcher liegen nebeneinander (2,56 mm Lochmittenabstand). R1 ist der 10-kΩ-Pull-up für RESET und sitzt links neben U1 in Zeile 11. Er hat ein freies Loch zwischen den Anschlüssen (5,12 mm Lochmittenabstand) und kann ebenfalls stehend bestückt werden.
- D1 hat von links nach rechts auf der Bestückungsseite die bestätigte Pinfolge Rot-Kathode, gemeinsame Anode, Grün-Kathode, Blau-Kathode. Die gemeinsame Anode liegt an VCC_SW. Den Beinchenabstand vor der Montage prüfen.
- Q1–Q3 sind in der Bestückungsansicht von links nach rechts mit **C (Kollektor), B (Basis), E (Emitter)** beschriftet. Die gespiegelte Lötseite zeigt von links nach rechts **E, B, C**. Die Zuordnung gilt jeweils für die drei Löcher eines Transistors. Die Tabellenpositionen für U1 beginnen bei der Pin-1-Markierung.
- JP1 (SOLO) ist eine zweipolige Stiftleiste. Ein Jumper verbindet 16/07 (PB0, U1 Pin 12) mit 17/07 (GND). Für diesen Aufbau U1 vor dem Einsetzen programmieren. Der Lochrasteraufbau hat keinen ISP-Anschluss.
- C1 (100 nF) belegt in Zeile 9 drei Rasterpunkte von Spalte 2 bis 4 (5,12 mm Anschlussabstand). C2 (1 µF) belegt darüber in Zeile 8 vier Rasterpunkte von Spalte 1 bis 4 (7,68 mm Anschlussabstand). Die angegebenen Rasterpunkte schließen die beiden Anschlusslöcher ein. C1 direkt mit U1.20 und dem Masseanschluss von U1 verbinden. Die direkte Masseverbindung ist in der Drahtliste aufgeführt.

| Bauteil | Wert oder Funktion | Lochkoordinaten in Pinreihenfolge |
|---|---|---|
| U1 | ATtiny2313-20PU | 05/11, 06/11, 07/11, 08/11, 09/11, 10/11, 11/11, 12/11, 13/11, 14/11, 14/08, 13/08, 12/08, 11/08, 10/08, 09/08, 08/08, 07/08, 06/08, 05/08 |
| BT1 | 3 x AAA / 4.5 V | 06/24, 13/24 |
| SW1 | ON | 07/19, 11/19 |
| SW2 | INC_rt | 04/13, 02/13 |
| SW3 | DEC_rt | 04/16, 02/16 |
| SW4 | INC_gn | 08/13, 10/13 |
| SW5 | DEC_gn | 08/16, 10/16 |
| SW6 | INC_bl | 14/13, 16/13 |
| SW7 | DEC_bl | 14/16, 16/16 |
| SW8 | STORE | 16/09, 18/09 |
| JP1 | SOLO | 16/07, 17/07 |
| C1 | 100n | 04/09, 02/09 |
| C2 | 1u | 04/08, 01/08 |
| R1 | 10k | 02/11, 04/11 |
| D1 | 1 Rot-K, 2 Anode, 3 Grün-K, 4 Blau-K | 08/01, 09/01, 10/01, 11/01 |
| Q1 | BC547B | 03/03, 04/03, 05/03 |
| R2 | 330R | 03/05, 03/04 |
| R5 | 1k | 04/06, 04/05 |
| R8 | 100k | 05/05, 05/04 |
| Q2 | BC547B | 08/03, 09/03, 10/03 |
| R3 | 330R | 08/05, 08/04 |
| R6 | 1k | 09/06, 09/05 |
| R9 | 100k | 10/05, 10/04 |
| Q3 | BC547B | 14/03, 15/03, 16/03 |
| R4 | 330R | 14/05, 14/04 |
| R7 | 1k | 15/06, 15/05 |
| R10 | 100k | 16/05, 16/04 |

## Verdrahtung auf der Lötseite

Jedes Paar bezeichnet einen Draht von Loch zu Loch. Die Lötseitenansicht zeigt den vorgesehenen Verlauf. Knicke ohne markiertes Anschlussloch sind Richtungswechsel des Drahts. Isolierten Draht verwenden, insbesondere an Kreuzungen und beim Überqueren fremder Lötaugen.

Die Massezuführung nutzt bei SW2, SW3, SW4, SW6, SW7 und SW8 jeweils das zweite Bein derselben Kontaktgruppe. Vor dem Löten am losen Taster prüfen, dass die beiden Massebeinchen auch bei losgelassenem Taster dauerhaft Durchgang haben. Bei SW7 führt der Draht von **15/24 nach 16/18**. Von **16/16** geht die Masse weiter zu SW6 auf **16/15**. Bei SW4 kann man **10/04 → 10/15 → 10/16** als durchgehenden Draht mit einem freigelegten Abgriff auf **10/15** ausführen.

Rechts führt die Masseleitung von **18/24 nach 18/11**. Der intern verbundene STORE-Kontakt **18/09** führt weiter über **18/07** nach **17/07** (JP1). Bei **18/07** sitzt auch der Masseabgriff für Q3-E (16/03). Die übrige Masse verteilt sich über 04/24 und 15/24 sowie kurze Verbindungen zwischen Masseanschlüssen. Gleichfarbige Linien sind anhand ihrer Endpunkte zuzuordnen.

| Netz | Drahtverbindungen |
|---|---|
| B_BL | 15/03 ↔ 15/05, 15/05 ↔ 16/05 |
| B_GN | 09/03 ↔ 09/05, 09/05 ↔ 10/05 |
| B_RT | 04/03 ↔ 04/05, 04/05 ↔ 05/05 |
| C_BL | 14/03 ↔ 14/04 |
| C_GN | 08/03 ↔ 08/04 |
| C_RT | 03/03 ↔ 03/04 |
| DEC_BL_N | 13/11 ↔ 14/16 |
| DEC_GN_N | 11/11 ↔ 08/16 |
| DEC_RT_N | 07/11 ↔ 04/16 |
| GND | 04/24 ↔ 15/24, 15/24 ↔ 18/24, 18/24 ↔ 18/11, 18/09 ↔ 18/07, 18/07 ↔ 17/07, 04/24 ↔ 02/18, 02/16 ↔ 02/15, 02/13 ↔ 01/08, 01/08 ↔ 05/04, 05/04 ↔ 05/03, 15/24 ↔ 13/24, 15/24 ↔ 16/18, 16/16 ↔ 16/15, 16/13 ↔ 14/11, 14/11 ↔ 10/13, 10/15 ↔ 10/16, 10/15 ↔ 10/04, 10/04 ↔ 10/03, 18/07 ↔ 16/03, 16/03 ↔ 16/04, 02/09 ↔ 14/11 |
| INC_BL_N | 12/11 ↔ 14/13 |
| INC_GN_N | 10/11 ↔ 08/13 |
| INC_RT_N | 06/11 ↔ 04/13 |
| K_BL | 11/01 ↔ 14/05 |
| K_GN | 10/01 ↔ 08/05 |
| K_RT | 08/01 ↔ 03/05 |
| PWM_BL | 11/08 ↔ 15/06 |
| PWM_GN | 10/08 ↔ 09/06 |
| PWM_RT | 09/08 ↔ 04/06 |
| RESET_N | 05/11 ↔ 04/11 |
| SOLO_N | 13/08 ↔ 16/07 |
| STORE_N | 14/08 ↔ 16/09 |
| VBAT | 06/24 ↔ 07/19 |
| VCC_SW | 05/08 ↔ 04/09, 04/09 ↔ 04/08, 04/09 ↔ 02/11, 05/08 ↔ 09/01, 02/11 ↔ 11/19 |

Vor dem ersten Einschalten bei abgezogener Batterie die Drahtverbindungen und die Trennung von Batterieplus, geschaltetem Plus und Masse prüfen. Bei offenem ON-Taster müssen Batterieplus und geschaltetes Plus getrennt sein.
