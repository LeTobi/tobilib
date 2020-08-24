# Guidelines

## Error- / Exception-Handling
3 Fälle werden unterschieden:
1) **Precondition verletzt**: Es soll eine std::exception geworfen werden. Zusätzlich sollte eine Funktion zur Verfügung stehen, um Precondition zu prüfen.
2) **Ungewöhnliche Ausnahme**: Es wird eine std::exception geworfen. Die Fehlerbehandlung ist *nicht* vorgesehen.
3) **Erwartete Ausnahme**: Eine erwartetet Ausnahme muss durch einen Status erkennbar sein. Die *throw* Anweisung darf nicht verwendet werden.

## State Machine
Klassen, die eine State Machine implementieren, müssen folgende Struktur Einhalten:
1) Zustandsvariablen: Bestimmen Funktionalität eindeutig.
2) Tick-Funktionalität: (wenn nötig) Interne Vorgänge, die zum Statusverlauf gehören. Gebündelt durch den Aufruf von tick()
3) Steuerfunktionen: Applikationsabhängige Beeinflussung des Statusverlaufs
4) Ausgabe: In Form von Variablen. Events sollen Abgerufen werden. **Keine Callbacks**
