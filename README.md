# Wordle

**Wordle für die Kommandozeile, geschrieben in C**

<img width="410" alt="grafik" src="https://github.com/user-attachments/assets/c39ecc75-ecf8-4cfc-b943-1da5a29382dc">



## Kompilieren

### Windows

#### Voraussetzungen

- [Buildtools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) herunterladen (siehe unter „Tools für Visual Studio“)
- Installer starten
- im Installer ein Häkchen bei „Desktopentwicklung mit C++“ setzen

Die Entwicklerkonsole kann nicht mit ANSI-Escape-Codes umgehen, darum die Standard-Eingabeaufforderung starten: Windows-Taste drücken, `eingabeaufforderung` eintippen, Enter drücken. Darin eintippen, um Git zu installieren, falls noch nicht vorhanden:

```
winget install --id Git.Git -e --source winget
```

Kompilierumgebung aktivieren:

```
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsamd64_x86.bat"
```

Ins Home-Verzeichnis wechseln und den Quellcode aus dem Repository laden:

```
cd %USERPROFILE%
git clone https://github.com/607011/wordle-c.git
```

In das soeben erstellte Verzeichnis wechseln und den Kompiliervorgang starten:

```
cd wordle-c
nmake exe
```


### macOS

Entwicklerwerkzeuge installieren:

```
xcode-select --install
brew install gcc git
```

Quellcode aus dem Repository laden:

```
cd ~
git clone https://github.com/607011/wordle-c.git
```

Ins soeben angelegte Verzeichnis wechseln und Kompiliervorgang starten:

```
cd wordle-c
make
```

### Linux (Ubuntu u.Ä.)

Build-Werkzeuge installieren:

```
sudo apt-get install build-essential git
```

Quellcode aus dem Repository laden:

```
cd ~
git clone https://github.com/607011/wordle-c.git
```

Ins soeben angelegte Verzeichnis wechseln und Kompiliervorgang starten:

```
cd wordle-c
make
```

---

### Nutzungshinweise

Diese Software wurde zu Lehr- und Demonstrationszwecken geschaffen und ist nicht für den produktiven Einsatz vorgesehen. Heise Medien und der Autor haften daher nicht für Schäden, die aus der Nutzung der Software entstehen, und übernehmen keine Gewähr für ihre Vollständigkeit, Fehlerfreiheit und Eignung für einen bestimmten Zweck.

### Terms Of Use

This software was created for teaching and demonstration purposes and is not intended for productive use. Heise Medien and the author are therefore not liable for damages arising from the use of the software and do not guarantee its completeness, freedom from errors and suitability for a specific purpose.
