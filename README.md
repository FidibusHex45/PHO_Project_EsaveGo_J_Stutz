# Semesterprojekt esave Go!!!
Die Firma [esave](https://www.esaveag.com/) produziert und verkauft intelligente Strassenbeleuchtung. Ein `Forschungsprojekt` mit der [FHGR](https://www.fhgr.ch/) hat zum Ziel diese Strassenbeleuchtungen mit zusätzlicher Intelligenz auszustatten. Es sollen neu statische und dynamische Objekte `detektiert` und `klassifiziert`, sowie `getrackt` und deren `Geschwindigkeit` bestimmt werden. Im Wesentlichen geht es darum, Fahrzeuge im Sichtbereich einer Kamera zu detektieren, deren Position bzw. Bewegung zu verfolgen und die jeweilige Geschwindigkeit zu bestimmen.
Das hier behandelte Projekt, das den Namen «`esave Go`» trägt, ist eine Vereinfachung der Problemstellung des Forschungsprojektes der FHGR mit esave. Ausgangslage des Projektes ist eine vorgegebene [Carrera Go](https://www.carrera-toys.com/go) Rennbahn, ein mechanischer Aufbau mit Kamerahaltung und eine [Industriekamera](https://de.ids-imaging.com/store/u3-3080cp-rev-2-2.html).
Die folgenden Probleme sollen im Zusammenhang mit diesem Projekt gelöst werden:

- `P1`.	Die Geschwindigkeit eines Carrera Go Auto kann über die Tastatur eines Laptops gesteuert werden
- `P2`.	Die zur Verfügung gestellte Industriekamera lässt sich ansteuern und funktioniert.
- `P3`.	Das Carrera Go Auto kann auf einen Startbefehl hin eine langsame Runde fahren, ohne die Bahn zu verlassen.
- `P4`.	Die Position und Geschwindigkeit des Carrera Go Autos kann bestimmt werden.
- `P5`.	Das Fahrzeug kann an beliebiger Stelle auf die Bahn gesetzt werden und von dort aus mit einem Startbefehl möglichst schnelle Rundenzeiten fahren.

Die Probleme P1-P5 werden mit folgenden Methoden gelöst:

- `M1`.	Mikrocontroller verbunden mit Laptop gibt ein PWM-Signal aus, welches eine Elektronik steuert, um das Auto gepulst zu betreiben.
- `M2`.	Die Ueye Library (C++) ermöglicht eine Ansteuerung der Kamera.
- `M3`.	Ein (C++) Programm wartet auf eine Eingabe und übermittelt dem Mikrokontroller anschliessend über USB eine langsame Geschwindigkeit (PWM >10% Tastverhältnis).
- `M4`.	Durch Bild-Subtraktion und Schwellwertverfahren mit anschliessender Schwerpunktermittlung wird die Position des Autos bestimmt.
- `M5`.	Der Carrera Go track wird über das Tracking des Autos eingelesen. Der eingelesene Track wird dann mit Hilfe von PCA analysiert. Ergebnis sind Geschwindigkeitsvorgaben über die ganze Bahn hinweg. Fährt das Carrera Go Auto, so wird kontinuierlich dessen Position mit den Geschwindigkeitsvorgaben an jener Stelle verglichen und eingestellt.

Resultat ist dementsprechend ein Carrera Auto, welches so schnell wie möglich Runden auf der vorgegebenen Bahn fährt. Dabei wird kontinuierlich die Geschwindigkeit gemessen und angezeigt.

## Projekt Aufbau
Die Software ist teils in C++ und teils in Python verfasst. Dabei ist die Auswertung der eingelesenen Rennbahn in Python implementiert und der Rest in C++.

### Klassendiagram C++ Software
~~~mermaid
classDiagram
  Camera ..> MemHandler
  Camera ..> TrackHandler
  Camera ..> CarController
  MemHandler ..> TrackHandler
  MemHandler ..> CarController
  CSVHandler ..> TrackHandler
  CSVHandler ..> CarController
  SerialSTM32 ..> TrackHandler
  SerialSTM32 ..> CarController
  class Camera {
    +configureCam(string)
    +startAqusition(int)
    +getFPS()
    +getImageFormat()
    +getColorMode()
    +getAvailableFormats()
    +stopAquisition()
  }
  class MemHandler {
    +allocRingBuffer()
    +allocSingleBuffer()
    +getOpenCVMatRingImg()
    +getOpenCVMatSingeImg()
    +drawFpsOnImg(Mat)
    +drawDroppedFramesOnImg(Mat)
    +drawTimeStamp(Mat)
    +saveSingleImg(string, optional~Mat~)
    +getActiveBufferID()
    +getRealFps()
    +getTimeStamp()
    +int activeBufferIdx
    +int imgWidth
    +int imgHeight
  }
  class TrackHandler {
    +getTrackData(string, string)
  }
  class CSVHandler {
    +save2csv(string, vector~Point~)
    +loadSplineData(string, int, int)
    +loadSplineDataConfig(string)
    +saveSplineData(string, vector~splineData_proc~)
  }
  class SerialSTM32 {
    +writeSerialPort(int)
    +closeSerialPort()
    +bool connected_
  }
  class CarController {
    +run()
  }
~~~

### Klassendiagramm Python Software
~~~mermaid
classDiagram
  CarreraTrackAnalysisApp <.. CSVHAndler
  CarreraTrackAnalysisApp <.. PointSorter
  CarreraTrackAnalysisApp <.. SequentialPCA
  CarreraTrackAnalysisApp <.. SplineInterpolation
  CarreraTrackAnalysisApp <.. VelocityEditor
  class CSVHAndler {
    +__init__(self)
    +loadCSV_Points(self, path)
    +saveData_2CSV(self, data, path)
  }
  class PointSorter {
    +__init__(self, points)
    +sortPoints(self, refPoint)
    +points
    +tree
  }
  class SequentialPCA {
    +__init__(self, points)
    +seqPCA(self, nPCA, nAvg)
    +points
    +pcaS
    +pcaS_diff
    +pcaS_diff_avg
  }
  class SplineInterpolation {
    +__init__(self, points, numKnots, smoothing, periodic, derivative)
    +interpolate(self)
    +points
    +numKnots
    +smoothing
    +periodic
    +derivative
    +bInt_SplinePoints
  }
  class VelocityEditor {
    +__init__(self, data, color_scheme)
    +adjust(self)
    +x_data
    +y_data
    +v_data
    +cmap
    +color_original
    +color_adjust
    +points
    +v_adjust
    +indices_picked_points
    +rotB
  }
  class CarreraTrackAnalysisApp {
    +__init__(self, path_load, path_write, ref_point)
    +run(self)
    +path_write
    +path_load
    +ref_point
    +csvHandler
  }
~~~

## Dieses Projekt verwenden
Dieses Projekt basiert auf dem `cmake` Buildsystem unter der verwendung von `vcpkg`. Beides muss auf dem PC installiert sein für eine problemlose Ausführung des Projektes. `vcpkg` wird für die Module opencv4 und nlohmann-json benötigt. Weiter muss möglicherweise der Python Interpreter-Pfad in [detectTrack.cpp](src/detectTrack.cpp) angepasst werden. Kamera und Mikrokontroller müssen vor dem Programmstart am PC angeschlossen werden.




