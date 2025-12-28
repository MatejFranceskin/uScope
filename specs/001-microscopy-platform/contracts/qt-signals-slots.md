# Qt Signals/Slots Contract: Camera Module

**Module**: Camera control and video capture  
**Owner**: Camera backend classes (`CameraController`, `VideoCapture`)

## Purpose

Defines Qt signals and slots for camera lifecycle, frame delivery, and control updates. Ensures loose coupling between camera backend and UI components.

## Class: CameraController

### Signals

```cpp
class CameraController : public QObject {
    Q_OBJECT
signals:
    // Camera lifecycle events
    void cameraConnected(const QString& deviceId, const QString& cameraName);
    void cameraDisconnected(const QString& deviceId);
    void cameraError(const QString& errorMessage);
    
    // Video frame delivery
    void frameReady(const QVideoFrame& frame);
    void frameRateChanged(int fps);
    
    // Camera control updates
    void exposureChanged(double exposureTime);
    void gainChanged(int gain);
    void whiteBalanceChanged(const QString& mode);
    void resolutionChanged(const QSize& resolution);
    
    // Capture events
    void imageCaptured(const QString& filePath);
    void captureError(const QString& errorMessage);
    
    // Recording events
    void recordingStarted(const QString& filePath);
    void recordingStopped(const QString& filePath, int durationSeconds);
    void recordingProgress(int elapsedSeconds);
    void recordingError(const QString& errorMessage);
```

### Slots

```cpp
public slots:
    // Camera lifecycle
    void startCamera(const QString& deviceId);
    void stopCamera();
    void switchCamera(const QString& deviceId);
    
    // Video preview
    void startPreview();
    void stopPreview();
    
    // Camera controls
    void setExposure(double exposureTime);  // milliseconds
    void setGain(int gain);                 // ISO value
    void setWhiteBalance(const QString& mode);  // "Tungsten", "Daylight", "Auto"
    void setResolution(const QSize& resolution);
    void setFlip(bool horizontal, bool vertical);
    
    // Auto controls
    void autoWhiteBalance(const QPointF& clickPoint);  // Click on white area
    void autoExposure();
    
    // Capture
    void captureImage();                    // Snapshot to file
    void captureImageToBuffer();            // In-memory capture for processing
    
    // Recording
    void startRecording(const QString& filePath);
    void stopRecording();
    void startTimeLapse(int intervalSeconds, int durationSeconds);
    void stopTimeLapse();
```

### Usage Example

```cpp
// In MainWindow or UI controller
CameraController* camera = new CameraController(this);

// Connect signals to UI slots
connect(camera, &CameraController::frameReady, 
        this, &MainWindow::updatePreview);
connect(camera, &CameraController::cameraError,
        this, &MainWindow::showErrorMessage);
connect(camera, &CameraController::imageCaptured,
        this, &MainWindow::onImageCaptured);

// Connect UI signals to camera slots
connect(ui->exposureSlider, &QSlider::valueChanged,
        camera, &CameraController::setExposure);
connect(ui->captureButton, &QPushButton::clicked,
        camera, &CameraController::captureImage);

// Start camera
camera->startCamera(selectedDeviceId);
```

---

## Class: CalibrationController

### Signals

```cpp
class CalibrationController : public QObject {
    Q_OBJECT
signals:
    // Calibration events
    void calibrationCreated(int calibrationId);
    void calibrationUpdated(int calibrationId);
    void calibrationDeleted(int calibrationId);
    void calibrationActivated(int calibrationId, double pixelsPerMicrometer);
    
    // Scale bar detection
    void scaleBarDetected(const QRect& boundingBox, double confidence, 
                          const QString& ocrText, double parsedValue, const QString& unit);
    void scaleBarDetectionFailed(const QString& reason);
    void scaleBarValidated(double pixelsPerMicrometer);
```

### Slots

```cpp
public slots:
    // Microscope/Objective management
    void createMicroscope(const QString& name, const QString& manufacturer, const QString& model);
    void addObjective(int microscopeId, const QString& name, const QString& magnification);
    void selectMicroscope(int microscopeId);
    void selectObjective(int objectiveId);
    
    // Manual calibration
    void startCalibration(const QLineF& calibrationLine, double realWorldMicrometers);
    void saveCalibration();
    void cancelCalibration();
    
    // Automatic scale bar detection
    void detectScaleBar(const QString& imagePath);
    void validateDetectedScale(double valueOverride, const QString& unitOverride);
    void rejectDetectedScale();
```

---

## Class: MeasurementController

### Signals

```cpp
class MeasurementController : public QObject {
    Q_OBJECT
signals:
    // Measurement events
    void measurementCreated(int measurementId, const QString& type, double value, const QString& unit);
    void measurementDeleted(int measurementId);
    void measurementUpdated(int measurementId);
    
    // Dataset events
    void datasetCreated(const QString& datasetName);
    void datasetUpdated(const QString& datasetName, const MeasurementStatistics& stats);
    void datasetExported(const QString& filePath);
```

### Slots

```cpp
public slots:
    // Measurement creation
    void createLineMeasurement(const QPointF& start, const QPointF& end);
    void createCircleMeasurement(const QPointF& center, double radius);
    void createAngleMeasurement(const QPointF& p1, const QPointF& vertex, const QPointF& p2);
    void createRectangleMeasurement(const QRectF& rect);
    void createPolygonMeasurement(const QPolygonF& polygon);
    
    // Measurement management
    void deleteMeasurement(int measurementId);
    void setMeasurementUnit(const QString& unit);  // Recalculates all measurements
    
    // Dataset management
    void createDataset(const QString& name);
    void addMeasurementToDataset(int measurementId, const QString& datasetName);
    void removeMeasurementFromDataset(int measurementId, const QString& datasetName);
    void exportDatasetToCSV(const QString& datasetName, const QString& filePath);
```

---

## Class: ObjectDetectionController

### Signals

```cpp
class ObjectDetectionController : public QObject {
    Q_OBJECT
signals:
    // Detection events
    void detectionStarted();
    void detectionProgress(int currentObject, int totalObjects);
    void detectionCompleted(int objectCount);
    void detectionError(const QString& errorMessage);
    
    // Object events
    void objectAdded(int objectId);
    void objectRemoved(int objectId);
    void objectValidated(int objectId);
    
    // Statistics events
    void statisticsUpdated(const MeasurementStatistics& stats);
```

### Slots

```cpp
public slots:
    // Detection control
    void startDetection(const DetectionParameters& params);
    void cancelDetection();
    
    // Manual object editing
    void addObject(const QPolygonF& contour);
    void removeObject(int objectId);
    void validateObject(int objectId);
    
    // Export
    void exportResults(const QString& filePath);  // CSV export
    void saveDetectionSet();  // Save to image metadata
```

---

## Class: RTSPStreamingController

### Signals

```cpp
class RTSPStreamingController : public QObject {
    Q_OBJECT
signals:
    // Server events
    void streamingStarted(const QString& rtspUrl);
    void streamingStopped();
    void streamingError(const QString& errorMessage);
    
    // Student connection events
    void studentConnected(const QString& deviceId, const QString& deviceName);
    void studentDisconnected(const QString& deviceId);
    void connectionLimitReached(int maxStudents);
    
    // Network events
    void networkQualityChanged(const QString& quality);  // "excellent", "good", "fair", "poor"
    void bandwidthUsageChanged(double mbps);
    
    // Annotation sync events
    void annotationSyncCompleted(int annotationId);
    void annotationSyncFailed(int annotationId, const QString& reason);
```

### Slots

```cpp
public slots:
    // Server control
    void startStreaming();
    void stopStreaming();
    void setStreamQuality(const QSize& resolution, int frameRate, int bitrate);
    void setMaxStudents(int max);
    void setAuthToken(const QString& token);  // Optional authentication
    
    // Annotation broadcasting
    void broadcastAnnotation(const Annotation& annotation);
    void broadcastAnnotationDeleted(int annotationId);
    
    // Student management
    void disconnectStudent(const QString& deviceId);
```

---

## Signal/Slot Naming Conventions

1. **Signals**: Use past tense or state change verbs
   - ✅ `cameraConnected`, `frameReady`, `exposureChanged`
   - ❌ `connectCamera`, `getFrame`, `changeExposure`

2. **Slots**: Use imperative verbs (commands)
   - ✅ `startCamera`, `setExposure`, `captureImage`
   - ❌ `cameraStarted`, `exposureSet`, `imageCaptured`

3. **Parameter Naming**: Use descriptive names matching entity attributes
   - ✅ `deviceId`, `exposureTime`, `pixelsPerMicrometer`
   - ❌ `id`, `value`, `scale`

4. **Error Signals**: Include `Error` suffix and provide `QString` error message
   - ✅ `cameraError(const QString& errorMessage)`
   - ❌ `error(int code)`

## Thread Safety

- All signals are queued connections when crossing thread boundaries (Qt default)
- Camera frame delivery uses `Qt::QueuedConnection` to avoid blocking capture thread
- Heavy processing (object detection, stitching, EDF) runs in separate `QThread`, emits progress signals
- UI updates from signals always run in main thread (Qt enforces)

## Testing Strategy

- Mock signal emission in unit tests using `QSignalSpy`
- Verify signal emission order (e.g., `detectionStarted` → `detectionProgress` → `detectionCompleted`)
- Test error conditions (camera disconnect → `cameraDisconnected` signal)
- Validate signal parameters match expected types and ranges

---

**Contract Status**: ✅ COMPLETE - Qt signals/slots interfaces defined for all major modules
