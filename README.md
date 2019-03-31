# tobilib
Bibliothek wird für Raspberry Projekte mit Internetsteuerung und GPIO verwendet. Basisfunktionen auch für Windows verfügbar.
Verwendet boost::asio

## Inhalt

* __StringPlus__: Erweiterter u32_string, konvertierbar zu und von std::string mit diversen Encodings
* __GPIO__: Einfacher GPIO-Zugriff (verwendet std::fstream)
* __Networking:__ Endpoints für TCP (ToDo) und WebSockets mit Verbindungsmöglichkeit als Client oder Acceptor
* __H2-Event-Protokoll__: Eigenes Protokoll für Eventaustausch mit JSON. Erweiterung für _Networking_
* __Webcap__: jpeg-Kameraaufnahmen mit video4linux-api und libjpeg (sehr eingeschränkt)

### Makefile
* base
* networking (nur linux)
* webcap (nur linux)

### Sonst so
Präprozessoranweisung TC_AS_HPP lässt Headerdateien mit Implementierung einbinden. Praktisch für VisualStudio
Ausführlichere Dokumentation folgt noch (vielleicht)
