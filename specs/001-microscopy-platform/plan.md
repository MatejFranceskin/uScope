# Implementation Plan: μScope Microscopy Platform

**Branch**: `001-microscopy-platform` | **Date**: 2025-12-05 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/001-microscopy-platform/spec.md`

## Summary

μScope is a cross-platform open-source microscopy application for educational institutions, hobbyists, and small laboratories using USB/UVC-compliant cameras. The platform provides live camera preview, image capture, manual camera controls, calibration/measurement tools, automated object detection, video recording, image stitching, extended depth of focus (EDF), professional export formats, classroom collaboration via RTSP streaming, and optional iNaturalist integration. Built with Qt 6.10+ and C++17, supporting Linux, Windows, macOS, Android, and iOS with OpenCV for advanced image processing and Tesseract for OCR-based scale bar detection.

## Technical Context

**Language/Version**: C++17 or later  
**Primary Dependencies**: Qt 6.10+ (Multimedia, Widgets, SVG, Quick for mobile), OpenCV 4.x (image stitching, EDF, object detection), Tesseract OCR 4.x (scale bar text recognition), FFmpeg/libx264 (H.264 video encoding via Qt Multimedia)  
**Storage**: QSettings (application preferences, microscope/objective profiles, calibrations), QFile/QDir (image/video files to ~/Documents/uScope/), Embedded EXIF/TIFF metadata (calibration, annotations in images), Custom metadata fields (PNG text chunks, TIFF tags)  
**Testing**: Manual verification with real UVC cameras, Qt Test framework for unit tests (if needed), integration tests for image processing pipelines  
**Target Platform**: Linux (primary development), Windows, macOS (desktop with Qt Widgets), Android 6.0+ and iOS 13+ (mobile with Qt Quick/QML)  
**Project Type**: Qt cross-platform application with tile-based overlay UI system (resolution-independent scaling, video background with transparent tile overlays)  
**UI Architecture**: Custom tile-based system - QGraphicsScene with video background, transparent tile widgets overlayed on top, proportional scaling based on window height (base tile = height/12), tiles anchor left/right/center, modal dialogs as centered tiles  
**Performance Goals**: 15+ fps live video preview, <1s still image capture, <200ms UI response to camera controls, <500ms annotation sync in RTSP streaming, 30 concurrent students for classroom mode, <3s scale bar OCR processing, <10s object detection for 100 objects  
**Constraints**: <200ms UI response time, <500ms RTSP latency on LAN, offline-capable core functionality (network only for RTSP/iNaturalist), 44x44pt minimum touch targets on mobile, proper mobile lifecycle handling (backgrounding, camera interruptions)  
**Scale/Scope**: Single-user desktop application with optional multi-user classroom mode (1 teacher + up to 30 students), 11 prioritized user stories (P1-P11), 75+ functional requirements, cross-platform (5 OSes)  
**CI/CD Requirements**: GitHub Actions workflows for automated builds on PR and release tags, producing platform-specific distribution packages (DEB for Ubuntu, RPM for Fedora/RHEL, WiX installer for Windows, DMG for macOS, APK for Android, IPA for iOS)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify compliance with constitution principles:

### Initial Evaluation (Pre-Design)

- [x] **Qt Framework Compliance**: Uses Qt signals/slots for camera events and UI interactions, follows Qt naming conventions (camelCase methods, _private members), uses parent-child ownership for QObjects (widgets, camera instances), leverages QStandardPaths/QDir for file operations, uses QString/QList throughout
- [x] **Cross-Platform Compatibility**: Uses QStandardPaths for Documents folder (FR-008.1), Qt path APIs (QDir::separator()), validates geometry for different DPI/screen sizes/orientations, touch-first design with 44x44pt targets (SC-014), handles mobile permissions (camera, storage), optimized for battery (mobile lifecycle handling in SC-015)
- [x] **Resource Management**: All Qt objects have parent ownership (MainWindow owns widgets, camera instances), camera/video resources released in closeEvent(), validates QImage::isNull() before QPainter operations, checks calculated font sizes/geometry for validity, prevents integer overflow in measurement calculations
- [x] **User Experience First**: Maintains 15+ fps video (FR-002, SC-002), UI operations <200ms (FR-015, SC-004), <100ms measurement display (SC-006), immediate visual feedback for buttons/tiles, camera settings reflect within 200ms (FR-015), 44x44pt touch targets (SC-014), clear error messages for camera disconnect/unsupported formats, handles mobile backgrounding without data loss (SC-015)
- [x] **Code Quality**: Functions focused on single responsibility (max 50 lines target), named constants for magic numbers (FPS targets, timeouts, measurement units), complex algorithms commented (EDF fusion, stitching, object detection), public APIs documented in headers, verification steps included in acceptance scenarios

**Additional Project-Specific Gates**:
- [x] **OpenCV Integration**: Optional dependency with graceful degradation (advanced features: stitching P7, EDF P8, object detection P5) - core features (P1-P4) work without OpenCV
- [x] **Tesseract OCR Integration**: Optional dependency for scale bar detection (FR-017.1-017.6) with manual calibration fallback (FR-017.6) when OCR unavailable
- [x] **RTSP Streaming**: Optional classroom mode (P10) with clean separation from core functionality - disabled by default, no performance impact when not in use
- [x] **iNaturalist API**: Optional integration (P11) with offline queue and retry logic (FR-058) - core microscopy features fully offline-capable
- [x] **GitHub Actions CI/CD**: Automated build workflows for all platforms, producing distribution packages on PR creation and release tags, ensuring consistent builds and simplified distribution

**Pre-Design Status**: ✅ PASS - All principles satisfied, optional dependencies properly isolated

---

### Post-Design Re-evaluation (After Phase 1)

**Verification against completed design documents** (data-model.md, contracts/):

- [x] **Qt Framework Compliance - VERIFIED**:
  - ✅ All 9 controller classes (CameraController, CalibrationController, MeasurementController, ObjectDetectionController, VideoRecordingController, ImageProcessingController, ExportController, RTSPStreamingController, iNaturalistController) inherit QObject with Q_OBJECT macro
  - ✅ All events use Qt signals: `frameReady`, `cameraConnected`, `calibrationSaved`, `measurementCompleted`, `objectsDetected`, etc. (see contracts/qt-signals-slots.md)
  - ✅ All actions use Qt slots: `startCamera`, `captureImage`, `createCalibration`, `detectObjects`, etc.
  - ✅ All 17 data model entities use Qt types: QString, QDateTime, QList, QSize, QPointF, QRectF, QPolygonF
  - ✅ File operations use QStandardPaths::DocumentsLocation for cross-platform paths (data-model.md Storage sections)
  - ✅ Persistent storage uses QSettings for microscope profiles, objectives, calibrations (data-model.md Entity 2, 3, 4)

- [x] **Cross-Platform Compatibility - VERIFIED**:
  - ✅ Image metadata uses both PNG text chunks (cross-platform via Qt) and EXIF/TIFF tags via exiv2 (data-model.md Entity 6)
  - ✅ File paths use QStandardPaths::DocumentsLocation (Linux: ~/Documents/uScope/, Windows: %USERPROFILE%\Documents\uScope\, macOS: ~/Documents/uScope/)
  - ✅ Mobile UI via Qt Quick/QML planned with shared C++ controllers (plan.md Project Structure)
  - ✅ RTSP service discovery uses both UDP broadcast (cross-platform) and mDNS/Bonjour fallback (contracts/rtsp-protocol.md)
  - ✅ Touch targets: 44x44pt minimum enforced in UI requirements (spec.md SC-014)
  - ✅ Resolution-independent tile UI: Proportional scaling (base_tile = height/12) works across all resolutions and DPIs (research.md Task 11)

- [x] **Resource Management - VERIFIED**:
  - ✅ All controllers have parent QObject ownership (created with `new Controller(this)` pattern per contracts/qt-signals-slots.md examples)
  - ✅ Camera resources lifecycle: [Disconnected] → [Available] → [Active] → [Capturing] with explicit state transitions (data-model.md Entity 1)
  - ✅ RTSP streaming session lifecycle: [Idle] → [Starting] → [Active] → [Stopping] with cleanup (data-model.md Entity 15)
  - ✅ Video recording state machine: [Idle] → [Recording] → [Paused] → [Stopped] with resource release (data-model.md Entity 12)
  - ✅ Image validation: QImage::isNull() checks before processing (implied in image processing workflows)
  - ✅ iNaturalist upload queue with retry logic prevents memory leaks from failed uploads (contracts/inaturalist-api.md)

- [x] **User Experience First - VERIFIED**:
  - ✅ Performance targets documented: 15+ fps (data-model.md Entity 1: supportedFrameRates), <200ms UI (SC-004), <500ms RTSP latency (contracts/rtsp-protocol.md)
  - ✅ Immediate feedback signals: `frameReady` (real-time video), `exposureChanged` (instant slider feedback), `imageCaptured` (capture confirmation)
  - ✅ Error signals for user notification: `cameraError`, `calibrationError`, `detectionError`, `recordingError`, `streamingError`, `uploadError` (all controllers have error signals)
  - ✅ Network quality indicators for RTSP: latency, jitter, packetLoss (data-model.md Entity 16: ConnectedStudent)
  - ✅ iNaturalist upload queue with retry prevents blocking UI (contracts/inaturalist-api.md: exponential backoff, offline queue)
  - ✅ Adaptive bitrate streaming (4 quality levels) ensures smooth classroom experience (contracts/rtsp-protocol.md)
  - ✅ Resolution-independent UI: Consistent appearance across all window sizes/resolutions, proportional scaling maintains readability (research.md Task 11)
  - ✅ Tile overlay transparency: Video remains visible through UI elements, modal dialogs dim background for focus (research.md Task 11)

- [x] **Code Quality - VERIFIED**:
  - ✅ Single responsibility: Each controller handles one feature area (Camera, Calibration, Measurement, ObjectDetection, VideoRecording, ImageProcessing, Export, RTSPStreaming, iNaturalist)
  - ✅ Named constants: FPS targets (15+ fps), timeouts (200ms UI, 500ms RTSP), measurement units (µm), quality levels (High/Medium/Low/Minimal)
  - ✅ Clear entity relationships: 17 entities with explicit foreign keys (microscopeId, objectiveId, calibrationId, sessionId) and documented cardinality (data-model.md)
  - ✅ Validation rules: All entities have validation constraints (non-empty strings, positive numbers, valid enums, date ranges)
  - ✅ State transitions documented: Camera (4 states), VideoRecording (4 states), RTSPStreamingSession (4 states), iNaturalistSession (5 states)

- [x] **Tile UI Architecture - VERIFIED**:
  - ✅ Proportional scaling validated: baseTileSize = windowHeight/12 ensures consistent sizing across all resolutions and DPIs (research.md Task 11)
  - ✅ Rounded corners specified: cornerRadius = baseTileSize/8 provides aesthetically consistent rounded rectangles for all tile sizes (contracts/tile-ui-architecture.md)
  - ✅ State-based rendering: Idle/Hover/Active/Disabled states with defined color/transparency values documented (contracts/tile-ui-architecture.md)
  - ✅ Resolution independence: Tile system scales correctly across desktop (1920×1080 to 3840×2160) and mobile (720×1280 portrait/landscape) confirmed
  - ✅ Touch target compliance: Base tile (height/12) meets 44×44pt minimum on all target devices per SC-014
  - ✅ Cross-platform consistency: Same proportional scaling formula works on Linux/Windows/macOS/Android/iOS without platform-specific adjustments

**Additional Project-Specific Gates - VERIFIED**:
- [x] **OpenCV Integration**: Isolated in `OpenCVService` class (plan.md services/), graceful degradation via optional dependency checks at runtime
- [x] **Tesseract OCR Integration**: Isolated in `OCRService` class, manual calibration fallback via FR-017.6 when Tesseract unavailable
- [x] **RTSP Streaming**: Isolated in `RTSPService` + `RTSPStreamingController`, disabled by default, independent lifecycle from core camera features
- [x] **iNaturalist API**: Isolated in `iNaturalistAPIService` + `iNaturalistController`, offline queue (data-model.md Entity 17: uploadQueue), core features work without network

**Post-Design Status**: ✅ PASS - All constitution principles verified in design, no violations detected

---

**Final Constitution Compliance**: ✅ APPROVED FOR IMPLEMENTATION - Design adheres to all Qt framework principles, cross-platform requirements, resource management, UX targets, and code quality standards. Optional dependencies properly isolated with graceful degradation.

## Project Structure

### Documentation (this feature)

```text
specs/[###-feature]/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)

```text
uScope/                     # Qt/C++ cross-platform microscopy application
├── CMakeLists.txt          # Build configuration (desktop platforms)
├── main.cpp                # Application entry point
│
# Desktop UI (Qt Widgets with Custom Tile System)
├── MainWindow.h            # Main window with QGraphicsView for video + tile overlay
├── MainWindow.cpp          # Main window implementation (no .ui file - custom layout)
├── VideoGraphicsScene.h    # QGraphicsScene with video background layer
├── VideoGraphicsScene.cpp  # Video preview renderer with tile overlay support
├── VideoSource.h           # Camera backend abstraction declaration
├── VideoSource.cpp         # Camera backend abstraction implementation
│
# Tile-based UI Components (Resolution-Independent Overlay System)
├── Tile.h                  # Base tile: square/rectangle, transparent, state-based colors
├── Tile.cpp                # Tile implementation with proportional scaling (height/12 base)
├── TileCombo.h             # Tile with dropdown (icon + text + combo)
├── TileCombo.cpp           # Tile combo implementation
├── TileDialog.h            # Modal dialog tile (centered, scrollable if needed)
├── TileDialog.cpp          # Modal dialog implementation with optional slider
├── TileDialogSettings.h    # Settings dialog tile declaration
├── TileDialogSettings.cpp  # Settings dialog implementation
├── TileLabel.h             # Basic clickable tile (icon + text, anchored left/right/center)
├── TileLabel.cpp           # Tile label implementation
├── TileButton.h            # NEW: Button tile (icon + text, state colors)
├── TileButton.cpp          # NEW: Clickable button tile
├── TileSlider.h            # NEW: Slider tile (for camera controls)
├── TileSlider.cpp          # NEW: Slider tile implementation
│
# Resources
├── images/                 # SVG/PNG UI resources
│   ├── camera.svg          # Camera icon
│   ├── media-playback-pause.svg
│   ├── media-playback-start.svg
│   ├── media-playback-stop.svg
│   ├── media-record.svg    # Record button icon
│   ├── settings.svg        # Settings icon
│   ├── uScope.ico          # Windows icon
│   └── uScope.svg          # Application logo
├── sounds/                 # Audio resources
│   ├── camera-shutter.mp3  # Shutter sound
│   └── snap.mp3            # Snap sound
│
# Platform-Specific Files
├── android/                # Android-specific build files
│   ├── AndroidManifest.xml # Android app manifest
│   └── res/                # Android resources
│       ├── drawable-hdpi/icon.png
│       ├── drawable-ldpi/icon.png
│       ├── drawable-mdpi/icon.png
│       ├── drawable-xhdpi/icon.png
│       ├── drawable-xxhdpi/icon.png
│       └── drawable-xxxhdpi/icon.png
├── ios/                    # iOS-specific build files
│   └── Info.plist.in       # iOS app property list template
├── macos/                  # macOS-specific build files
│   └── Info.plist.in       # macOS app property list template
│
# Feature Implementation (NEW FILES TO ADD)
├── controllers/            # NEW: Business logic controllers
│   ├── CameraController.h/.cpp         # Camera state management (FR-001 to FR-016)
│   ├── CalibrationController.h/.cpp    # Calibration/measurement (FR-017 to FR-024)
│   ├── MeasurementController.h/.cpp    # Measurement tools (FR-022 to FR-024)
│   ├── ObjectDetectionController.h/.cpp # Object detection (FR-025 to FR-029)
│   ├── VideoRecordingController.h/.cpp # Video recording (FR-030 to FR-034)
│   ├── ImageProcessingController.h/.cpp # Stitching/EDF (FR-035 to FR-042)
│   ├── ExportController.h/.cpp         # Professional export (FR-043 to FR-049)
│   ├── RTSPStreamingController.h/.cpp  # Classroom mode (FR-050 to FR-056)
│   └── iNaturalistController.h/.cpp    # iNaturalist integration (FR-057 to FR-062)
│
├── models/                 # NEW: Data models
│   ├── CameraProfile.h/.cpp            # Camera device model
│   ├── MicroscopeProfile.h/.cpp        # Microscope configuration
│   ├── Objective.h/.cpp                # Objective lens model
│   ├── Calibration.h/.cpp              # Calibration data
│   ├── DetectedScaleBar.h/.cpp         # Scale bar detection result
│   ├── CapturedImage.h/.cpp            # Image with metadata
│   ├── Measurement.h/.cpp              # Single measurement
│   ├── MeasurementDataset.h/.cpp       # Measurement collection
│   ├── DetectedObjectSet.h/.cpp        # Object detection result set
│   ├── DetectedObject.h/.cpp           # Single detected object
│   ├── Annotation.h/.cpp               # Annotation layer
│   ├── VideoRecording.h/.cpp           # Video recording metadata
│   ├── FocusStack.h/.cpp               # Focus stack data
│   ├── StitchedPanorama.h/.cpp         # Panorama data
│   ├── RTSPStreamingSession.h/.cpp     # RTSP session state
│   ├── ConnectedStudent.h/.cpp         # Student connection info
│   └── iNaturalistSession.h/.cpp       # iNaturalist session state
│
├── services/               # NEW: Service layer
│   ├── CameraService.h/.cpp            # Camera backend (QCamera integration)
│   ├── CalibrationService.h/.cpp       # Calibration persistence (QSettings)
│   ├── OCRService.h/.cpp               # Tesseract OCR wrapper
│   ├── OpenCVService.h/.cpp            # OpenCV operations wrapper
│   ├── RTSPService.h/.cpp              # GStreamer RTSP server wrapper
│   └── iNaturalistAPIService.h/.cpp    # iNaturalist API client
│
├── ui/                     # NEW: Feature-specific tile dialogs (no .ui files - custom tiles)
│   ├── CalibrationDialog.h/.cpp        # Calibration modal tile dialog
│   ├── MeasurementOverlay.h/.cpp       # Measurement tools tile overlay
│   ├── ObjectDetectionDialog.h/.cpp    # Object detection settings tile dialog
│   ├── ExportDialog.h/.cpp             # Export options tile dialog
│   ├── RTSPSettingsDialog.h/.cpp       # RTSP server settings tile dialog
│   ├── iNaturalistAuthDialog.h/.cpp    # iNaturalist OAuth tile dialog
│   └── CameraControlsPanel.h/.cpp      # Camera controls tile panel (exposure, WB, flip)
│
├── qml/                    # NEW: Mobile UI (Qt Quick/QML - future)
│   ├── main.qml            # Mobile main screen
│   ├── components/         # Reusable QML components
│   └── pages/              # QML pages (camera, settings, measurements)
│
# Build Output
├── build/                  # Build artifacts (gitignored)
│   └── Desktop_Qt_6_10_0-Debug/
│       └── uScope          # Compiled executable
│
# Specifications
├── specs/                  # Feature specifications
│   └── 001-microscopy-platform/
│       ├── spec.md         # Feature specification with user stories
│       ├── plan.md         # THIS FILE - Implementation plan
│       ├── research.md     # Technology research and decisions
│       ├── data-model.md   # Entity definitions and relationships
│       ├── quickstart.md   # Developer onboarding guide
│       └── contracts/      # API contracts
│           ├── qt-signals-slots.md     # Internal Qt signal/slot interfaces
│           ├── rtsp-protocol.md        # RTSP streaming protocol
│           ├── inaturalist-api.md      # iNaturalist API specification
│           ├── tile-ui-architecture.md # Tile-based overlay UI system specification
│           └── annotation-rendering.md # Annotation rendering with outlined style
│
# External Dependencies
├── OpenCV-android-sdk/     # OpenCV for Android (bundled)
│   ├── sdk/
│   └── samples/
│
# Project Metadata
├── LICENSE                 # License file
├── README.md               # Project documentation
├── CMakeLists.txt.user     # Qt Creator user settings (gitignored)
│
# CI/CD Infrastructure (NEW FILES TO ADD)
├── .github/
│   └── workflows/
│       ├── build-pr.yml             # Build all platforms on PR (artifacts uploaded)
│       ├── build-release.yml        # Build and release packages on tagged releases
│       ├── build-linux-deb.yml      # Ubuntu/Debian DEB package workflow
│       ├── build-linux-rpm.yml      # Fedora/RHEL RPM package workflow
│       ├── build-windows.yml        # Windows WiX installer workflow
│       ├── build-macos.yml          # macOS DMG package workflow
│       ├── build-android.yml        # Android APK workflow
│       └── build-ios.yml            # iOS IPA workflow (requires macOS runner)
```

**Structure Decision**: Custom tile-based overlay UI architecture for resolution-independent scaling. Desktop uses Qt Widgets with QGraphicsView/QGraphicsScene - video feed renders as background layer, transparent tiles overlay on top. Tiles are squares or rectangles (sides = multiples of base tile size = window_height/12), anchored left/right/center, with state-based background colors and transparency. Modal dialogs are centered tiles with optional scrollbar. This provides consistent appearance across all resolutions/DPIs with proportional scaling. Mobile will use same tile concept via Qt Quick/QML. Controllers implement business logic (see contracts/qt-signals-slots.md), models encapsulate data (see data-model.md), services wrap external dependencies (OpenCV, Tesseract, GStreamer, iNaturalist). Existing tile infrastructure (Tile.h/cpp, TileCombo, TileDialog, TileLabel) forms foundation, extended with new tile types (TileButton, TileSlider, feature-specific dialogs).

**CI/CD Strategy**: GitHub Actions workflows automate build and packaging for all platforms. PR builds produce artifacts for testing, release tag builds publish distribution packages. Linux builds use native package managers (dpkg-deb for DEB, rpmbuild for RPM), Windows uses WiX Toolset for MSI installer, macOS uses create-dmg for DMG, Android uses Gradle with Qt for Android, iOS uses xcodebuild with Qt for iOS. All workflows install Qt 6.10+, OpenCV 4.x, Tesseract 4.x, and GStreamer as dependencies, run CMake/Gradle/Xcode builds, and package outputs. Artifacts uploaded to GitHub Actions (PR builds) or GitHub Releases (tagged releases).

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [e.g., 4th project] | [current need] | [why 3 projects insufficient] |
| [e.g., Repository pattern] | [specific problem] | [why direct DB access insufficient] |
