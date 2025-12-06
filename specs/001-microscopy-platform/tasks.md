# Tasks: μScope Microscopy Platform

**Input**: Design documents from `/specs/001-microscopy-platform/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: Not explicitly requested - focus on implementation with manual testing

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story. Per user request, initial focus is on project files for all platforms, empty main window, CI/CD, and installers.

## Format: `- [ ] [ID] [P?] [Story?] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

---

## Phase 1: Setup (Project Infrastructure for All Platforms)

**Purpose**: Initialize cross-platform Qt project structure, build system, and CI/CD workflows

**Target**: Create buildable skeleton application for Linux, Windows, macOS, Android, iOS with automated packaging

- [X] T001 Create CMakeLists.txt at repository root with Qt 6.10+ project configuration (project name: uScope, VERSION 0.1.0, C++17, Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::Svg)
- [X] T002 [P] Create main.cpp with minimal Qt application entry point (QApplication, MainWindow instantiation, exec loop)
- [X] T003 [P] Create MainWindow.h header with QMainWindow subclass, Q_OBJECT macro, forward declarations for QGraphicsView/QGraphicsScene
- [X] T004 [P] Create MainWindow.cpp with empty MainWindow constructor, destructor, setupUi method stub
- [X] T005 [P] Create VideoGraphicsScene.h header with QGraphicsScene subclass for video background rendering
- [X] T006 [P] Create VideoGraphicsScene.cpp with constructor, basic paint method stub
- [X] T007 [P] Update .gitignore to exclude build/, *.user, *.autosave, .vscode/, .idea/, CMakeCache.txt, CMakeFiles/
- [X] T008 Create README.md with project description, build instructions for all platforms, dependency list (Qt 6.10+, OpenCV 4.x, Tesseract 4.x, GStreamer)
- [X] T009 [P] Create resources.qrc file with images/ and sounds/ directories referenced from existing assets
- [X] T010 [P] Create uScope.desktop file for Linux desktop integration (Name, Comment, Exec, Icon, Categories=Education;Science)
- [X] T011 [P] Update android/AndroidManifest.xml with camera permissions (android.permission.CAMERA, android.permission.WRITE_EXTERNAL_STORAGE)
- [X] T012 [P] Update ios/Info.plist.in with camera usage description (NSCameraUsageDescription, NSPhotoLibraryUsageDescription)
- [X] T013 [P] Update macos/Info.plist.in with bundle identifier, version, camera entitlements
- [X] T014 Update CMakeLists.txt to install desktop file, icons, and resources for Linux package
- [X] T015 [P] Create debian/ directory structure for DEB packaging (control, rules, copyright, changelog)
- [X] T016 [P] Create debian/control with package metadata (Package: uscope, Version, Depends: libqt6core6, libqt6multimedia6, libopencv4, tesseract-ocr, gstreamer1.0-rtsp)
- [X] T017 [P] Create rpm/uscope.spec file with RPM package metadata and build instructions
- [X] T018 [P] Create windows/uscope.wxs WiX source file for MSI installer with file components, shortcuts, registry entries
- [X] T019 Verify project builds successfully on Linux with `cmake -B build && cmake --build build`

**Checkpoint**: Minimal Qt application compiles and runs with empty window on Linux

---

## Phase 2: CI/CD Workflows (GitHub Actions for All Platforms)

**Purpose**: Automated builds and packaging for PR validation and release distribution

**⚠️ CRITICAL**: These workflows enable continuous testing and distribution across all platforms

- [X] T020 [P] Create .github/workflows/build-linux-deb.yml with Ubuntu runner, Qt/OpenCV/Tesseract/GStreamer installation, CMake build, dpkg-deb packaging
- [X] T021 [P] Create .github/workflows/build-linux-rpm.yml with Fedora container, dnf dependencies, rpmbuild packaging
- [X] T022 [P] Create .github/workflows/build-windows.yml with windows-latest runner, aqtinstall for Qt 6.10, vcpkg for OpenCV, WiX toolset installation, MSI build
- [X] T023 [P] Create .github/workflows/build-macos.yml with macos-latest runner, Homebrew dependencies (qt@6, opencv, tesseract, gstreamer), macdeployqt bundling, create-dmg for DMG
- [X] T024 [P] Create .github/workflows/build-android.yml with ubuntu-latest runner, Android SDK/NDK setup, Qt for Android installation, Gradle build, APK signing with keystore from secrets
- [X] T025 [P] Create .github/workflows/build-ios.yml with macos-latest runner, Qt for iOS installation, Xcode build, IPA export with code signing from secrets
- [X] T026 Create .github/workflows/build-pr.yml orchestrating all platform builds on pull_request trigger with artifact upload
- [X] T027 Create .github/workflows/build-release.yml orchestrating all platform builds on tag push (v*.*.*) with GitHub Release creation and asset upload
- [X] T028 [P] Add workflow caching for Qt installation, vcpkg, Homebrew to reduce build times
- [X] T029 [P] Create .github/ISSUE_TEMPLATE/ with bug report and feature request templates
- [X] T030 [P] Create .github/PULL_REQUEST_TEMPLATE.md with checklist (builds pass, constitution check, manual testing)
- [ ] T031 Test build-pr.yml workflow by creating test PR, verify all platform builds complete, artifacts uploaded
- [X] T032 Document required GitHub Secrets in README.md (ANDROID_KEYSTORE, ANDROID_KEYSTORE_PASSWORD, IOS_CERTIFICATE, IOS_PROVISIONING_PROFILE)

**Checkpoint**: All platform workflows build successfully, packages created and uploaded as artifacts

---

## Phase 3: Foundational (Core Tile UI Architecture)

**Purpose**: Implement tile-based overlay UI system with proportional scaling - BLOCKS all feature user stories

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [ ] T033 Create Tile.h with QGraphicsWidget subclass, Anchor enum (Left, Right, Center), TileState enum (Idle, Hover, Active, Disabled), updateGeometry method, cornerRadius method
- [ ] T034 Create Tile.cpp with constructor taking width/height multipliers and anchor, paint method with rounded rectangle rendering (radius = baseTileSize/8), hover event handlers
- [ ] T035 [P] Create TileLabel.h with Tile subclass, icon QSvgRenderer member, text QString member
- [ ] T036 [P] Create TileLabel.cpp with paint override rendering scaled SVG icon and text below
- [ ] T037 [P] Create TileButton.h with Tile subclass, clicked() signal, pressed state animation
- [ ] T038 [P] Create TileButton.cpp with mousePressEvent/mouseReleaseEvent emitting clicked signal
- [ ] T039 [P] Create TileCombo.h with Tile subclass, QGraphicsProxyWidget for embedded QComboBox, currentTextChanged signal
- [ ] T040 [P] Create TileCombo.cpp with QComboBox creation, proxy widget setup, signal forwarding
- [ ] T041 [P] Create TileSlider.h with Tile subclass, QGraphicsProxyWidget for embedded QSlider, valueChanged signal
- [ ] T042 [P] Create TileSlider.cpp with QSlider creation, orientation setup, signal forwarding
- [ ] T043 [P] Create TileDialog.h with Tile subclass for modal centered dialogs, dimOverlay QGraphicsRectItem member, show/hide methods
- [ ] T044 [P] Create TileDialog.cpp with semi-transparent black overlay (alpha 180), centered positioning, Esc key handling
- [ ] T045 Update MainWindow.h with QGraphicsView* _view, VideoGraphicsScene* _scene, QList<Tile*> _leftTiles/_rightTiles/_centerTiles, calculateBaseTileSize() method, resizeEvent override
- [ ] T046 Update MainWindow.cpp setupUi to create QGraphicsView, set VideoGraphicsScene, configure view (no scrollbars, antialiasing), add to central widget
- [ ] T047 Implement MainWindow::calculateBaseTileSize() returning height() / 12
- [ ] T048 Implement MainWindow::resizeEvent calling updateTileLayout() which recalculates baseTileSize and calls updateGeometry on all tiles
- [ ] T049 Add placeholder TileButton instances to MainWindow (_leftTiles: camera icon, settings icon; _rightTiles: capture icon, record icon) in constructor
- [ ] T050 Update VideoGraphicsScene.cpp to render black background rectangle as placeholder for video feed
- [ ] T051 Verify tile UI renders correctly, tiles scale proportionally on window resize, hover states work, tiles have rounded corners

**Checkpoint**: Foundation ready - tile-based UI system functional with proportional scaling

---

## Phase 4: User Story 1 - Live Camera Preview and Basic Capture (Priority: P1) 🎯 MVP

**Goal**: Researcher connects USB microscope, sees live video feed, captures high-quality still images with one click

**Independent Test**: Connect UVC camera, verify live preview appears automatically, click snap button, confirm image saved to ~/Documents/uScope/ with timestamp

### Implementation for User Story 1

- [ ] T052 [P] [US1] Create models/CameraProfile.h with QString id/name/manufacturer, QSize resolution, int frameRate, QDateTime lastUsed
- [ ] T053 [P] [US1] Create models/CameraProfile.cpp with constructor, validation (non-empty name, positive resolution/frameRate), toJson/fromJson for QSettings persistence
- [ ] T054 [P] [US1] Create models/CapturedImage.h with QString filePath, QDateTime timestamp, QSize resolution, QString cameraId, QImage imageData
- [ ] T055 [P] [US1] Create models/CapturedImage.cpp with constructor, save() method writing to QStandardPaths::DocumentsLocation + "/uScope/", embedded metadata via QImageWriter::setText
- [ ] T056 [P] [US1] Create services/CameraService.h with QObject subclass, QCamera* _camera, QVideoSink* _sink, frameReady(QVideoFrame) signal, error(QString) signal
- [ ] T057 [US1] Create services/CameraService.cpp with enumerateCameras() using QMediaDevices::videoInputs(), startCamera(cameraId), stopCamera(), captureFrame() methods
- [ ] T058 [US1] Implement CameraService::startCamera connecting QVideoSink::videoFrameChanged to frameReady signal, setting QCameraFormat, calling QCamera::start()
- [ ] T059 [US1] Implement CameraService::captureFrame() grabbing current QVideoFrame, converting to QImage, emitting imageCaptured signal
- [ ] T060 [P] [US1] Create controllers/CameraController.h with QObject subclass, CameraService* _service, frameReady(QVideoFrame) signal, cameraConnected(QString id) signal
- [ ] T061 [US1] Create controllers/CameraController.cpp with startCamera(id) slot calling _service->startCamera(), captureImage() slot calling _service->captureFrame(), signal forwarding
- [ ] T062 [US1] Update VideoGraphicsScene to render QVideoFrame as background via QGraphicsPixmapItem, connect to CameraController::frameReady signal
- [ ] T063 [US1] Update MainWindow constructor to create CameraController instance, auto-start first available camera
- [ ] T064 [US1] Create ui/CameraControlsPanel.h with Tile subclass, TileCombo for camera selection dropdown, TileButton for snap
- [ ] T065 [US1] Create ui/CameraControlsPanel.cpp populating camera combo from CameraController::enumerateCameras(), connecting snap button to CameraController::captureImage()
- [ ] T066 [US1] Add CameraControlsPanel tile to MainWindow _leftTiles, position at top-left with updateGeometry
- [ ] T067 [US1] Implement image save to ~/Documents/uScope/ with filename format "image_yyyyMMdd_HHmmss.png" using QDateTime
- [ ] T068 [US1] Add visual feedback on capture (camera shutter sound from sounds/camera-shutter.mp3, brief flash overlay)
- [ ] T069 [US1] Handle camera disconnect gracefully: show error dialog, attempt reconnection every 2 seconds
- [ ] T070 [US1] Test with real UVC camera: verify 15+ fps preview, <1s capture latency, images saved with correct timestamp

**Checkpoint**: User Story 1 fully functional - camera preview and image capture working

---

## Phase 5: User Story 2 - Manual Camera Controls (Priority: P2)

**Goal**: Researcher adjusts exposure, white balance, color settings for optimal image quality

**Independent Test**: With live preview running, adjust exposure slider, verify brightness changes in <200ms. Click white balance, confirm color correction

### Implementation for User Story 2

- [ ] T071 [P] [US2] Update CameraService.h with setExposure(qreal value), setWhiteBalance(mode), setBrightness(int), setContrast(int), setSaturation(int), flipHorizontal(bool), flipVertical(bool) methods
- [ ] T072 [US2] Implement CameraService camera control methods calling QCamera::setManualExposureTime(), QCamera::setWhiteBalanceMode(), updating exposure/color parameters
- [ ] T073 [US2] Add flip transform via QVideoSink transformation matrix (QTransform::scale(-1, 1) for horizontal, (1, -1) for vertical)
- [ ] T074 [P] [US2] Update ui/CameraControlsPanel.h adding TileSlider for exposure, TileSlider for brightness, TileSlider for contrast, TileSlider for saturation
- [ ] T075 [US2] Update ui/CameraControlsPanel.cpp connecting sliders to CameraController slots (setExposure, setBrightness, setContrast, setSaturation)
- [ ] T076 [P] [US2] Add TileButton for auto white balance (one-click on center of frame), TileButton for flip horizontal, TileButton for flip vertical
- [ ] T077 [US2] Implement auto white balance by sampling center region of current frame, calculating color correction, applying via QCamera::setColorTemperature()
- [ ] T078 [US2] Verify <200ms latency for exposure/brightness/contrast/saturation adjustments (SC-004 requirement)
- [ ] T079 [US2] Add persistence of camera settings to QSettings per camera ID, restore on camera selection
- [ ] T080 [US2] Test manual controls: verify real-time updates, flip works correctly, white balance corrects color cast

**Checkpoint**: User Stories 1 AND 2 both work independently

---

## Phase 6: User Story 3 - Calibration and Measurement (Priority: P3)

**Goal**: Researcher calibrates with stage micrometer, measures specimen features in micrometers with scientific accuracy

**Independent Test**: Load micrometer image, draw calibration line on 1mm, save calibration for 10x objective. Load specimen, draw line measurement, verify µm display

### Implementation for User Story 3

- [ ] T081 [P] [US3] Create models/MicroscopeProfile.h with QString id/name/manufacturer/model, QList<QString> objectiveIds
- [ ] T082 [P] [US3] Create models/MicroscopeProfile.cpp with constructor, validation, persistence to QSettings, objectives management
- [ ] T083 [P] [US3] Create models/Objective.h with QString id/name, qreal magnification, QString microscopeId
- [ ] T084 [P] [US3] Create models/Objective.cpp with constructor, validation (positive magnification), persistence
- [ ] T085 [P] [US3] Create models/Calibration.h with QString id, QString microscopeId, QString objectiveId, qreal pixelsPerMicron, QSize imageResolution, QDateTime createdAt
- [ ] T086 [P] [US3] Create models/Calibration.cpp with constructor, validation (positive pixelsPerMicron), calculateScale(pixels) method, persistence
- [ ] T087 [P] [US3] Create models/Measurement.h with QString id, enum Type (Line, Circle, Polygon, Angle), QList<QPointF> points, qreal value, QString unit
- [ ] T088 [P] [US3] Create models/Measurement.cpp with calculateLength(), calculateArea(), calculateAngle() methods using Calibration::pixelsPerMicron
- [ ] T089 [P] [US3] Create models/MeasurementDataset.h with QString id/name, QList<Measurement> measurements, statistics (mean, stdDev, min, max)
- [ ] T090 [P] [US3] Create models/MeasurementDataset.cpp with addMeasurement(), removeM measurement(), calculateStatistics() methods
- [ ] T091 [P] [US3] Create services/CalibrationService.h with QObject subclass, methods for createMicroscope(), createObjective(), saveCalibration(), loadCalibration()
- [ ] T092 [US3] Create services/CalibrationService.cpp implementing persistence to QSettings with key format "microscopes/{id}/objectives/{id}/calibration"
- [ ] T093 [P] [US3] Create controllers/CalibrationController.h with slots for createMicroscope(), addObjective(), selectObjective(), drawCalibrationLine(), saveCalibration()
- [ ] T094 [US3] Create controllers/CalibrationController.cpp implementing calibration workflow: capture reference distance in pixels, real-world value in µm, calculate pixelsPerMicron
- [ ] T095 [P] [US3] Create controllers/MeasurementController.h with slots for startLineMeasurement(), startCircleMeasurement(), startPolygonMeasurement(), finishMeasurement()
- [ ] T096 [US3] Create controllers/MeasurementController.cpp implementing measurement tools with live preview overlay, final value calculation using active Calibration
- [ ] T097 [P] [US3] Create ui/CalibrationDialog.h with TileDialog subclass, microscope/objective selection combos, calibration input fields (pixel distance, real distance)
- [ ] T098 [US3] Create ui/CalibrationDialog.cpp with UI layout, validation (positive values), save button connecting to CalibrationController::saveCalibration
- [ ] T099 [P] [US3] Create ui/MeasurementOverlay.h with QGraphicsItem subclass for rendering measurement annotations with outlined style (semi-transparent outline, bright fill, antialiasing)
- [ ] T100 [US3] Create ui/MeasurementOverlay.cpp implementing outlined rendering per contracts/annotation-rendering.md (yellow lines, cyan area measurements, text via QPainterPath)
- [ ] T101 [US3] Add scale bar overlay to VideoGraphicsScene rendering calibration scale (e.g., "100 µm" with white line, outlined style)
- [ ] T102 [US3] Implement scale bar auto-display when Calibration exists for current microscope/objective combination
- [ ] T103 [US3] Create measurement tools tile panel (TileButton for line, circle, polygon tools) in MainWindow _rightTiles
- [ ] T104 [US3] Connect measurement tool buttons to MeasurementController, display measurement values in tooltip or overlay label
- [ ] T105 [US3] Implement dataset management: create dataset, add measurements, calculate statistics (mean, std dev, min, max)
- [ ] T106 [US3] Add export dataset to CSV with columns (ID, Type, Value, Unit, Timestamp)
- [ ] T107 [US3] Test calibration workflow: create microscope "Olympus BX43", add objective "10x Plan", calibrate with 1mm = 1000px, verify pixelsPerMicron = 1.0
- [ ] T108 [US3] Test measurement accuracy: measure 100µm line, verify displayed value within 1% error
- [ ] T108a [US3] Implement automatic scale bar detection on external image load: connect CapturedImage::load signal to OCRService::detectScaleBar, display confidence score with detected scale region highlighted, prompt user to validate/correct before applying calibration (implements FR-017.1.1)

**Checkpoint**: User Stories 1, 2, AND 3 all work independently

---

## Phase 7: User Story 4 - Video Recording (Priority: P4)

**Goal**: Researcher records dynamic processes as video or time-lapse for later analysis

**Independent Test**: Start video recording, verify MP4 file created with H.264 encoding. Stop recording, confirm playback with proper compression

### Implementation for User Story 4

- [ ] T109 [P] [US4] Create models/VideoRecording.h with QString filePath, QDateTime startTime/endTime, qreal duration, qint64 fileSize, enum State (Idle, Recording, Paused, Stopped)
- [ ] T110 [P] [US4] Create models/VideoRecording.cpp with constructor, state transition methods, finalize() saving metadata
- [ ] T111 [P] [US4] Update CameraService.h adding QMediaRecorder* _recorder, startRecording(filePath), stopRecording(), pauseRecording(), resumeRecording() methods
- [ ] T112 [US4] Implement CameraService recording methods: create QMediaRecorder with QMediaFormat::MPEG4, set video codec to QMediaFormat::VideoCodec::H264, configure encoding with EncodingMode::ConstantQuality and quality=23 (CRF 23 per FR-006.1), connect to _camera, start/stop
- [ ] T113 [US4] Add recording duration timer updating VideoRecording::duration every second
- [ ] T114 [P] [US4] Create controllers/VideoRecordingController.h with slots startRecording(), stopRecording(), pauseRecording(), signals recordingStarted(), recordingStopped()
- [ ] T115 [US4] Create controllers/VideoRecordingController.cpp coordinating CameraService recorder, managing VideoRecording model state
- [ ] T116 [US4] Add record button (TileButton with record icon) to MainWindow _rightTiles, toggle between start/stop states
- [ ] T117 [US4] Implement recording indicator overlay (red dot, "REC" text, elapsed time) on VideoGraphicsScene
- [ ] T118 [US4] Add disk space check before recording: warn if <1GB free, prevent recording if <100MB free
- [ ] T119 [US4] Implement time-lapse mode: capture frames at user-defined interval (e.g., 5 seconds), compile to video with QMediaRecorder frame-by-frame
- [ ] T120 [US4] Test video recording: start recording, verify MP4 file created in ~/Documents/uScope/, stop recording, confirm H.264 encoding with ffprobe

**Checkpoint**: User Stories 1-4 all work independently

---

## Phase 8: User Story 5 - Automated Object Detection (Priority: P5)

**Goal**: Researcher runs automated spore/object detection, manually corrects false positives/negatives, generates measurement statistics

**Independent Test**: Load calibrated spore image, run detection with size/circularity parameters, verify most spores detected. Remove false positive, add missed spore, run statistics

### Implementation for User Story 5

- [ ] T121 [P] [US5] Create models/DetectedObject.h with QString id, QPolygonF contour, QRectF boundingBox, qreal area, qreal perimeter, qreal circularity, QPointF centroid
- [ ] T122 [P] [US5] Create models/DetectedObject.cpp with constructor calculating properties (area, perimeter, circularity = 4π×area/perimeter²)
- [ ] T123 [P] [US5] Create models/DetectedObjectSet.h with QString id, QList<DetectedObject> objects, detection parameters (minArea, maxArea, minCircularity)
- [ ] T124 [P] [US5] Create models/DetectedObjectSet.cpp with addObject(), removeObject(), calculateStatistics() methods
- [ ] T125 [P] [US5] Create services/OpenCVService.h with QObject subclass, detectObjects(QImage, params) method returning QList<QPolygonF> contours
- [ ] T126 [US5] Create services/OpenCVService.cpp implementing OpenCV detection pipeline: QImage→cv::Mat, grayscale, Gaussian blur, Otsu threshold, morphologyEx(OPEN/CLOSE), findContours, filter by area/circularity
- [ ] T127 [US5] Add progress callback to OpenCVService::detectObjects emitting detectionProgress(int percent) signal for long operations
- [ ] T128 [P] [US5] Create controllers/ObjectDetectionController.h with slots runDetection(params), addObject(contour), removeObject(id), measureAllObjects()
- [ ] T129 [US5] Create controllers/ObjectDetectionController.cpp coordinating OpenCVService detection, DetectedObjectSet management, MeasurementController integration
- [ ] T130 [P] [US5] Create ui/ObjectDetectionDialog.h with TileDialog subclass, parameter inputs (min/max area sliders, circularity threshold slider), preview checkbox
- [ ] T131 [US5] Create ui/ObjectDetectionDialog.cpp with parameter UI, real-time preview updating detection overlay on parameter change
- [ ] T132 [US5] Implement detection overlay rendering all detected contours with cyan outlined style, bounding boxes with magenta outlined style per contracts/annotation-rendering.md
- [ ] T133 [US5] Add manual correction UI: click detected object to select, Delete key or button to remove, click-and-drag to add missed object
- [ ] T134 [US5] Implement "Measure All Objects" workflow: iterate detected objects, create Measurement for each with length/width/area, add to MeasurementDataset
- [ ] T135 [US5] Display statistics panel (mean, std dev, min, max, histogram) for detected object measurements
- [ ] T136 [US5] Test detection pipeline: load spore image with 50+ objects, verify >90% detection rate, <10s processing time (SC-012.3)

**Checkpoint**: User Stories 1-5 all work independently

---

## Phase 9: User Story 6 - Image Enhancement (Priority: P6)

**Goal**: Researcher applies filters and segmentation to highlight specimen features

**Independent Test**: Load specimen image, apply sharpen filter, verify edge enhancement. Apply Otsu threshold, confirm binary mask generated

### Implementation for User Story 6

- [ ] T137 [P] [US6] Update services/OpenCVService.h adding applyFilter(QImage, filterType) method with enum FilterType (Sharpen, EdgeDetection, HistogramEqualization, Threshold)
- [ ] T138 [US6] Implement OpenCVService filter methods: sharpen via cv::filter2D with kernel [[-1,-1,-1],[-1,9,-1],[-1,-1,-1]], edge detection via cv::Sobel, equalization via cv::equalizeHist
- [ ] T139 [US6] Implement Otsu thresholding: cv::threshold with THRESH_OTSU, return binary cv::Mat converted to QImage
- [ ] T140 [P] [US6] Create controllers/ImageProcessingController.h with slots applySharpen(), applyEdgeDetection(), applyHistogramEq(), applyThreshold(threshold)
- [ ] T141 [US6] Create controllers/ImageProcessingController.cpp managing filter stack, undo/redo, updating VideoGraphicsScene with processed image
- [ ] T142 [US6] Add filter tools panel (TileButton for sharpen, edge detect, histogram EQ, threshold) to MainWindow
- [ ] T143 [US6] Implement histogram display overlay showing RGB/grayscale distribution with black/white clipping sliders
- [ ] T144 [US6] Add real-time preview for threshold adjustment (slider updating binary image live)
- [ ] T145 [US6] Test filters: verify sharpen enhances edges without artifacts, Otsu threshold produces clean binary mask

**Checkpoint**: User Stories 1-6 all work independently

---

## Phase 10: User Story 7 - Image Stitching (Priority: P7)

**Goal**: Researcher captures overlapping frames, stitches into high-resolution panorama

**Independent Test**: Capture 3x3 grid of overlapping stage micrometer images, run stitching, verify seamless panorama with correct scale

### Implementation for User Story 7

- [ ] T146 [P] [US7] Create models/StitchedPanorama.h with QString id, QImage panorama, QList<QString> sourceImagePaths, QDateTime createdAt
- [ ] T147 [P] [US7] Create models/StitchedPanorama.cpp with constructor, save() method, metadata embedding
- [ ] T148 [P] [US7] Update services/OpenCVService.h adding stitchImages(QList<QImage>) method returning QImage panorama
- [ ] T149 [US7] Implement OpenCVService::stitchImages using cv::Stitcher::create(cv::Stitcher::PANORAMA), convert QImage→cv::Mat, call stitcher.stitch(), convert result→QImage
- [ ] T150 [US7] Add progress callback emitting stitchingProgress(int percent) for long operations (target <30s per SC-008)
- [ ] T151 [US7] Handle stitching errors (insufficient overlap, ERR_NEED_MORE_IMGS): emit stitchingError(QString message) signal
- [ ] T152 [P] [US7] Create controllers/ImageProcessingController stitching slots: startStitchingSession(gridSize), captureStitchFrame(), finishStitching()
- [ ] T153 [US7] Implement stitching session workflow: guide user through grid positions (1/9, 2/9, etc.), collect frames, call OpenCVService::stitchImages
- [ ] T154 [US7] Create stitching progress dialog showing grid capture status, current frame, estimated time remaining
- [ ] T155 [US7] Transfer calibration from source images to stitched panorama (same pixelsPerMicron applies)
- [ ] T156 [US7] Test stitching: capture 3x3 micrometer grid with 30% overlap, verify panorama has no visible seams, scale bar correct

**Checkpoint**: User Stories 1-7 all work independently

---

## Phase 11: User Story 8 - Extended Depth of Focus (Priority: P8)

**Goal**: Researcher captures focus stack, fuses into all-in-focus composite image

**Independent Test**: Manually capture 5-10 images at different focus depths of thick specimen, run EDF, verify all layers sharp

### Implementation for User Story 8

- [ ] T157 [P] [US8] Create models/FocusStack.h with QString id, QList<QImage> frames, QImage fusedImage, QDateTime createdAt
- [ ] T158 [P] [US8] Create models/FocusStack.cpp with constructor, addFrame(), save() method
- [ ] T159 [P] [US8] Update services/OpenCVService.h adding fuseEDF(QList<QImage>) method returning QImage composite
- [ ] T160 [US8] Implement OpenCVService::fuseEDF using Laplacian pyramid: for each frame compute cv::Laplacian, find max response per pixel across stack, blend using mask
- [ ] T161 [US8] Add progress callback emitting edfProgress(int percent) for processing (target <30s per SC-009)
- [ ] T162 [P] [US8] Update controllers/ImageProcessingController with EDF slots: startEDFSession(), captureEDFFrame(), finishEDF()
- [ ] T163 [US8] Implement EDF session workflow: prompt user to adjust focus, capture frame, repeat, call OpenCVService::fuseEDF
- [ ] T164 [US8] Create EDF progress dialog showing frame count, focus quality indicator (Laplacian variance), estimated time
- [ ] T165 [US8] Transfer calibration from source frames to EDF composite
- [ ] T166 [US8] Test EDF: capture 8 frames of pollen grain at different depths, verify all structures sharp in composite

**Checkpoint**: User Stories 1-8 all work independently

---

## Phase 12: User Story 9 - Professional Export (Priority: P9)

**Goal**: Researcher generates publication-ready reports and exports with embedded metadata

**Independent Test**: Capture calibrated image with measurements, generate PDF report, verify image/table/metadata included. Export OME-TIFF, confirm metadata preserved

### Implementation for User Story 9

- [ ] T167 [P] [US9] Create models/Annotation.h with enum Type (Line, Arrow, Text, Shape, ScaleBar), QList<QPointF> points, QString text, QColor color, int lineWidth, int outlineWidth, qreal outlineOpacity
- [ ] T168 [P] [US9] Create models/Annotation.cpp with constructor, validation, toJson/fromJson for metadata serialization
- [ ] T169 [P] [US9] Update models/CapturedImage adding QList<Annotation> annotations, embedMetadata() method
- [ ] T170 [US9] Implement CapturedImage::embedMetadata writing PNG text chunks with uScope.Calibration and uScope.Annotations JSON, or EXIF UserComment via exiv2 for JPEG/TIFF
- [ ] T171 [P] [US9] Create services/ExportService.h with methods generatePDF(images, measurements), exportOMETIFF(image), exportWithBurnedAnnotations(image)
- [ ] T172 [US9] Implement ExportService::generatePDF using QPdfWriter: render images, measurement tables (QTextDocument with HTML), statistics, metadata (microscope, objective, timestamp)
- [ ] T173 [US9] Implement ExportService::exportOMETIFF writing OME-XML metadata to TIFF via libtiff with calibration (PhysicalSizeX/Y in µm)
- [ ] T174 [US9] Implement ExportService::exportWithBurnedAnnotations rendering all Annotation overlays directly into image pixels (non-removable)
- [ ] T175 [P] [US9] Create controllers/ExportController.h with slots generateReport(), exportImage(format), exportDataset()
- [ ] T176 [US9] Create controllers/ExportController.cpp coordinating ExportService, file dialogs, progress indication
- [ ] T177 [US9] Create export dialog (TileDialog) with format selection (PDF, OME-TIFF, JPEG, PNG, TIFF), options (burn annotations, include statistics)
- [ ] T178 [US9] Implement annotation loading: read PNG text chunks or EXIF UserComment, deserialize JSON, reconstruct Annotation objects
- [ ] T179 [US9] Add annotation editing UI: click annotation to select, move points, edit text, change color, delete
- [ ] T180 [US9] Test metadata embedding: save PNG with calibration/annotations, reopen in μScope, verify annotations editable
- [ ] T181 [US9] Test PDF generation: create report with 3 images, measurement table, statistics, verify high-resolution images, searchable text

**Checkpoint**: User Stories 1-9 all work independently

---

## Phase 13: User Story 10 - RTSP Classroom Streaming (Priority: P10)

**Goal**: Teacher broadcasts live microscope feed with annotations via RTSP, students connect on LAN and view real-time stream

**Independent Test**: Teacher enables RTSP streaming, student app on same LAN discovers stream, connects, verifies live video and annotations appear in <500ms

### Implementation for User Story 10

- [ ] T182 [P] [US10] Create models/RTSPStreamingSession.h with QString sessionId, QString teacherName, QString streamURL, QHostAddress teacherIP, quint16 port, enum State (Idle, Starting, Active, Stopping)
- [ ] T183 [P] [US10] Create models/RTSPStreamingSession.cpp with state transitions, validation
- [ ] T184 [P] [US10] Create models/ConnectedStudent.h with QString id, QString name, QHostAddress ip, qreal latency, qreal jitter, int packetLoss
- [ ] T185 [P] [US10] Create models/ConnectedStudent.cpp with updateQualityMetrics() method
- [ ] T186 [P] [US10] Create services/RTSPService.h with startServer(port), stopServer(), embedAnnotation(annotation), methods, studentConnected/Disconnected signals
- [ ] T187 [US10] Implement RTSPService using GStreamer gst-rtsp-server: create GstRTSPServer, add factory with appsrc→x264enc→rtph264pay pipeline, feed QVideoFrame to appsrc
- [ ] T188 [US10] Implement annotation embedding via custom RTP payload (PT=97) or RTSP metadata track, serialize Annotation to JSON, send with video frames
- [ ] T189 [US10] Implement UDP broadcast service discovery: send JSON payload every 2 seconds with {teacherName, IP, port, streamURL}, listen for broadcasts on client
- [ ] T190 [US10] Add mDNS/Bonjour service registration for cross-subnet discovery (Linux: avahi-publish-service, macOS: dns-sd, Windows: Bonjour SDK)
- [ ] T191 [P] [US10] Create controllers/RTSPStreamingController.h with slots startStreaming(teacherName), stopStreaming(), updateAnnotations(), signals streamingStarted(), studentConnected(student)
- [ ] T192 [US10] Create controllers/RTSPStreamingController.cpp coordinating RTSPService, managing RTSPStreamingSession state, ConnectedStudent list
- [ ] T193 [US10] Create RTSP settings dialog (TileDialog) with teacher name input, port selection (default 8554), quality/bitrate presets (High/Medium/Low/Minimal per contracts/rtsp-protocol.md)
- [ ] T194 [US10] Add streaming control panel (TileButton to start/stop, student count indicator, quality indicator) to MainWindow
- [ ] T195 [US10] Implement client stream discovery: scan UDP broadcasts, parse JSON, populate stream list
- [ ] T196 [US10] Implement client RTSP playback: GStreamer rtspsrc→rtph264depay→avdec_h264→appsink pipeline, emit QVideoFrame to VideoGraphicsScene
- [ ] T197 [US10] Parse annotation metadata from RTP payload, deserialize JSON, render as overlay on client
- [ ] T198 [US10] Implement adaptive bitrate: monitor student latency/jitter/packetLoss, switch quality levels (1080p→720p→480p→360p) dynamically
- [ ] T199 [US10] Add connection quality indicator overlay (green/yellow/red based on latency <200ms/200-500ms/>500ms)
- [ ] T200 [US10] Test classroom mode: teacher starts stream, student discovers and connects, verify <500ms annotation sync (FR-052), 30 concurrent students supported

**Checkpoint**: User Stories 1-10 all work independently

---

## Phase 14: User Story 11 - iNaturalist Integration (Priority: P11)

**Goal**: Naturalist links session to iNaturalist observation, pushes images and measurement statistics to observation fields

**Independent Test**: Authenticate with iNaturalist, link to existing observation, capture images with spore measurements, push to observation, verify dimension field populated

### Implementation for User Story 11

- [ ] T201 [P] [US11] Create models/iNaturalistSession.h with QString sessionId, QString observationId, QString accessToken, QDateTime tokenExpiry, enum State (NotAuthenticated, Authenticated, Linked, Uploading, Complete)
- [ ] T202 [P] [US11] Create models/iNaturalistSession.cpp with state transitions, validation, token persistence to QSettings
- [ ] T203 [P] [US11] Create services/iNaturalistAPIService.h with authenticate(), searchObservations(query), createObservation(), uploadPhoto(observationId, imagePath), updateObservationField() methods
- [ ] T204 [US11] Implement iNaturalistAPIService OAuth 2.0 flow using QOAuthHttpServerReplyHandler: open browser to authorization URL, handle callback on localhost:8080, exchange code for token
- [ ] T205 [US11] Implement iNaturalistAPIService API calls via QNetworkAccessManager with Bearer token: GET /observations, POST /observations, POST /observations/:id/photos, PUT /observations/:id
- [ ] T206 [US11] Add rate limiter (90 requests/minute per contracts/inaturalist-api.md) using QElapsedTimer, queue requests if limit reached
- [ ] T207 [US11] Implement upload queue with exponential backoff: retry failed uploads with delay 2^attempt seconds, max 5 retries
- [ ] T208 [P] [US11] Create controllers/iNaturalistController.h with slots authenticate(), linkObservation(id), uploadImage(path), updateFields(fieldMap), signals authenticated(), uploadComplete()
- [ ] T209 [US11] Create controllers/iNaturalistController.cpp coordinating iNaturalistAPIService, managing iNaturalistSession state, upload queue
- [ ] T210 [US11] Create iNaturalist authentication dialog (TileDialog) with "Authenticate with iNaturalist" button launching OAuth flow, status indicator
- [ ] T211 [US11] Create observation link dialog (TileDialog) with search input, observation list, "Create New Observation" button
- [ ] T212 [US11] Implement measurement-to-observation-field mapping: detect common field names (Spore Length, Spore Width, Cap Diameter), format statistics as "mean ± std dev"
- [ ] T212a [US11] Create field mapping UI dialog (TileDialog) for linking measurement datasets to iNaturalist observation fields: dropdown to select MeasurementDataset, input for observation field name, format selection ("mean ± std dev" or "mean: X, range: Y-Z"), preview formatted output, save mapping (implements FR-055, FR-056, FR-057)
- [ ] T213 [US11] Add upload progress indicator showing queued/uploading/completed images, estimated time remaining
- [ ] T214 [US11] Handle network errors gracefully: queue images for offline upload, retry when connection restored, show sync status
- [ ] T215 [US11] Test OAuth flow: verify authentication successful, token stored securely, expired token triggers re-authentication (FR-060)
- [ ] T216 [US11] Test upload workflow: link to observation, capture 3 images with measurements, push to iNaturalist, verify images uploaded, spore dimensions populated in observation fields

**Checkpoint**: All user stories (1-11) work independently

---

## Phase 15: Polish & Cross-Cutting Concerns

**Purpose**: Improvements affecting multiple user stories, final validation

- [ ] T217 [P] Add comprehensive error handling: QCamera::errorOccurred, file I/O errors, network errors, OpenCV exceptions, display user-friendly error dialogs
- [ ] T218 [P] Implement logging framework: QLoggingCategory for subsystems (camera, calibration, measurement, export, rtsp, inaturalist), write to ~/Documents/uScope/logs/
- [ ] T219 [P] Add keyboard shortcuts: Ctrl+S (snap image), Ctrl+R (start/stop recording), Ctrl+M (measurement mode), Ctrl+C (calibration dialog), Esc (cancel current operation)
- [ ] T220 [P] Optimize VideoGraphicsScene rendering: use ItemCoordinateCache for tiles, disable antialiasing during real-time drawing, cache scaled SVG icons
- [ ] T221 [P] Add settings dialog (TileDialog) for global preferences: default save location, video codec/quality, RTSP port, language selection
- [ ] T222 [P] Implement internationalization: wrap all user-facing strings in tr(), create en_US.ts translation file, add language selection to settings
- [ ] T223 [P] Add about dialog (TileDialog) with application version, Qt version, OpenCV version, license information, credits
- [ ] T224 [P] Create comprehensive user manual in docs/ with screenshots, workflow examples, troubleshooting section
- [ ] T225 [P] Add telemetry opt-in: collect anonymous usage statistics (features used, camera models, crash reports), send to privacy-preserving analytics service
- [ ] T226 Verify constitution compliance: all Qt objects have parent ownership, <200ms UI response time met, 44x44pt touch targets on mobile, proper error messages
- [ ] T227 Run quickstart.md validation: verify build instructions work on Linux/Windows/macOS, all dependencies documented, sample images provided
- [ ] T228 Performance testing: verify 15+ fps live preview, <1s image capture, <200ms camera controls response, <3s scale bar OCR, <10s object detection for 100 objects
- [ ] T229 Create demo video showcasing all 11 user stories, workflow examples, classroom mode demonstration
- [ ] T230 [P] Update README.md with feature highlights, screenshots, installation instructions for all platforms, contribution guidelines

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **CI/CD (Phase 2)**: Depends on Setup (Phase 1) having buildable skeleton
- **Foundational (Phase 3)**: Depends on Setup (Phase 1) - BLOCKS all user stories
- **User Stories (Phase 4-14)**: All depend on Foundational (Phase 3) completion
  - User stories CAN proceed in parallel if staffed
  - OR sequentially in priority order (P1→P2→P3...→P11)
- **Polish (Phase 15)**: Depends on desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1) - Camera Preview/Capture**: Independent after Foundational
- **User Story 2 (P2) - Camera Controls**: Extends US1 (CameraService), but testable independently
- **User Story 3 (P3) - Calibration/Measurement**: Independent after Foundational (new controllers/services)
- **User Story 4 (P4) - Video Recording**: Extends US1 (CameraService with QMediaRecorder)
- **User Story 5 (P5) - Object Detection**: Requires US3 (Calibration for measurements), new OpenCVService
- **User Story 6 (P6) - Image Enhancement**: Independent after Foundational (OpenCVService filters)
- **User Story 7 (P7) - Stitching**: Requires US3 (Calibration transfer), OpenCVService stitching
- **User Story 8 (P8) - EDF**: Requires US3 (Calibration transfer), OpenCVService fusion
- **User Story 9 (P9) - Export**: Uses US3 (Annotation model), exports all prior work
- **User Story 10 (P10) - RTSP Streaming**: Extends US1 (CameraService video feed), new RTSPService
- **User Story 11 (P11) - iNaturalist**: Uses US3 (MeasurementDataset), new iNaturalistAPIService

### Parallel Opportunities

**Setup Phase (Phase 1)**:
- T002-T006 (all source files) in parallel
- T009-T013 (all platform files) in parallel
- T015-T018 (all packaging files) in parallel

**CI/CD Phase (Phase 2)**:
- T020-T025 (all platform workflow files) in parallel
- T028-T030 (documentation templates) in parallel

**Foundational Phase (Phase 3)**:
- T035-T044 (all Tile subclass pairs .h/.cpp) in parallel
- Within each user story: all [P] tasks in parallel

**Across User Stories**:
Once Foundational complete, if team has multiple developers:
- Developer A: User Story 1 (Camera) + User Story 2 (Controls)
- Developer B: User Story 3 (Calibration)
- Developer C: User Story 4 (Video Recording)
- All can proceed in parallel, integrate independently

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup (T001-T019)
2. Complete Phase 2: CI/CD (T020-T032)
3. Complete Phase 3: Foundational (T033-T051)
4. Complete Phase 4: User Story 1 (T052-T070)
5. **STOP and VALIDATE**: Test with real UVC camera
6. Deploy/demo MVP

### Incremental Delivery (Per User Request)

1. **Foundation** (Phases 1-3): Project files, CI/CD, tile UI → Deployable skeleton with installers
2. **MVP** (Phase 4): Add User Story 1 → Camera preview/capture working
3. **Iteration 2** (Phase 5): Add User Story 2 → Camera controls
4. **Iteration 3** (Phase 6): Add User Story 3 → Calibration/measurement
5. Continue adding user stories in priority order, each independently testable

### Parallel Team Strategy

With 3 developers:

1. **Week 1** (All together): Phases 1-3 (Setup + CI/CD + Foundational)
2. **Week 2** (Parallel):
   - Dev A: US1 (Camera Preview/Capture)
   - Dev B: US3 (Calibration/Measurement)
   - Dev C: CI/CD testing and refinement
3. **Week 3** (Parallel):
   - Dev A: US2 (Camera Controls)
   - Dev B: US5 (Object Detection)
   - Dev C: US4 (Video Recording)
4. Continue in priority order with parallel stories

---

## Summary

**Total Tasks**: 230
**Phase 1 (Setup)**: 19 tasks - Project structure for all platforms
**Phase 2 (CI/CD)**: 13 tasks - Automated builds and packaging
**Phase 3 (Foundational)**: 19 tasks - Tile UI architecture
**Phase 4 (US1)**: 19 tasks - Camera preview and capture (MVP)
**Phase 5 (US2)**: 10 tasks - Manual camera controls
**Phase 6 (US3)**: 28 tasks - Calibration and measurement
**Phase 7 (US4)**: 12 tasks - Video recording
**Phase 8 (US5)**: 16 tasks - Object detection
**Phase 9 (US6)**: 9 tasks - Image enhancement
**Phase 10 (US7)**: 11 tasks - Image stitching
**Phase 11 (US8)**: 10 tasks - Extended depth of focus
**Phase 12 (US9)**: 15 tasks - Professional export
**Phase 13 (US10)**: 19 tasks - RTSP streaming
**Phase 14 (US11)**: 16 tasks - iNaturalist integration
**Phase 15 (Polish)**: 14 tasks - Cross-cutting improvements

**Parallel Opportunities**: 89 tasks marked [P] can run in parallel within their phase
**Independent Stories**: All 11 user stories independently testable after Foundational phase
**MVP Scope**: Phases 1-4 (70 tasks) - Recommended for initial delivery

**Suggested MVP**: Complete Phases 1-4 only (Setup + CI/CD + Foundational + User Story 1) for first release, validating core architecture and cross-platform builds before adding advanced features.
