# Research Document: μScope Microscopy Platform

**Branch**: `001-microscopy-platform` | **Date**: 2025-12-05

## Purpose

This document consolidates research findings to resolve all technical unknowns identified during planning phase for the μScope microscopy platform implementation.

## Research Tasks

### 1. Qt Multimedia Camera API for UVC Devices

**Question**: How to enumerate and access UVC cameras using Qt Multimedia? What are the capabilities and limitations?

**Decision**: Use `QMediaDevices::videoInputs()` for camera enumeration and `QCamera` for capture

**Rationale**:
- Qt 6.x provides unified cross-platform camera API via Qt Multimedia module
- `QMediaDevices::videoInputs()` returns list of available cameras with metadata (name, description, id)
- `QCamera` class handles camera lifecycle, format negotiation, and frame delivery via `QVideoSink`
- `QCameraFormat` allows querying/selecting resolution, framerate, pixel format
- Automatically handles UVC driver differences across Linux (V4L2), Windows (DirectShow/Media Foundation), macOS (AVFoundation)

**Alternatives Considered**:
- **OpenCV VideoCapture**: Rejected - less Qt-integrated, manual platform-specific handling, no signals/slots integration
- **Platform-native APIs (V4L2, DirectShow, AVFoundation)**: Rejected - violates cross-platform principle, massive maintenance burden
- **Qt 5.x QtMultimedia**: Rejected - deprecated, Qt 6.x is target version

**Implementation Notes**:
- Connect `QVideoSink::videoFrameChanged(QVideoFrame)` signal to receive frames
- Use `QCamera::setCameraFormat()` to select resolution/framerate
- Use `QCamera::cameraDevice()` to get device info for multi-camera switching
- Camera controls (exposure, white balance) via `QCamera::setExposureMode()`, `QCamera::setWhiteBalanceMode()`, `QCamera::setManualExposureTime()`, etc.

**References**:
- Qt Multimedia Documentation: https://doc.qt.io/qt-6/qtmultimedia-index.html
- QCamera Class Reference: https://doc.qt.io/qt-6/qcamera.html
- QMediaDevices Class Reference: https://doc.qt.io/qt-6/qmediadevices.html

---

### 2. Video Encoding with H.264 Codec via Qt

**Question**: How to encode video to H.264/AVC with CRF 23 quality using Qt Multimedia?

**Decision**: Use `QMediaRecorder` with H.264 codec settings via `QMediaFormat`

**Rationale**:
- `QMediaRecorder` integrates with `QCamera` for seamless recording
- `QMediaFormat::VideoCodec::H264` specifies H.264/AVC codec
- `QMediaRecorder::setQuality()` with `QMediaRecorder::HighQuality` approximates CRF 23
- `QMediaRecorder::setVideoFrameRate()` controls output FPS (30fps default per FR-063.1)
- Platform backends (FFmpeg on Linux, Media Foundation on Windows, AVFoundation on macOS) handle encoding

**Alternatives Considered**:
- **FFmpeg CLI via QProcess**: Rejected - complex piping, no Qt integration, manual frame passing overhead
- **libx264 direct integration**: Rejected - not cross-platform, manual container muxing required
- **Qt 5.x QMediaRecorder**: Rejected - different API, Qt 6.x is target

**Implementation Notes**:
- For precise CRF control, may need platform-specific `QMediaRecorder::setEncodingSettings()` with custom parameters
- MP4 container via `QMediaFormat::FileFormat::MPEG4`, AVI via `QMediaFormat::FileFormat::AVI`
- Embed calibration metadata in MP4 using `QMediaMetaData` where possible, or post-process with custom tags

**References**:
- QMediaRecorder Documentation: https://doc.qt.io/qt-6/qmediarecorder.html
- QMediaFormat Documentation: https://doc.qt.io/qt-6/qmediaformat.html

---

### 3. EXIF/TIFF Metadata Embedding for Calibration and Annotations

**Question**: How to embed custom metadata (microscope, objective, µm/pixel scale, annotation layers) in JPEG/PNG/TIFF images using Qt?

**Decision**: Use Qt built-in metadata support with custom EXIF/TIFF tags and PNG text chunks

**Rationale**:
- `QImageWriter::setText()` for PNG text chunks (key-value pairs)
- EXIF UserComment field via platform-specific libraries (exiv2, libexif) for JPEG/TIFF
- Custom TIFF tags via libtiff for scientific metadata
- Qt provides basic EXIF read/write, but custom tags require third-party libraries

**Alternatives Considered**:
- **Sidecar JSON/XML files**: Rejected - fragile (files can separate), user confusion, not embedded
- **OME-TIFF only**: Rejected - doesn't cover JPEG/PNG, overkill for simple metadata
- **Filename encoding**: Rejected - limited capacity, not standards-compliant

**Implementation Notes**:
- For PNG: Use `QImageWriter::setText("uScope.Calibration", jsonString)` with JSON-serialized calibration
- For JPEG/TIFF: Integrate exiv2 library to write custom EXIF/TIFF tags
- Store: microscope name, objective magnification, µm/pixel scale, camera settings, timestamp
- Annotation layers: serialize as JSON array of annotation objects (type, position, text, styling)
- Annotation rendering: All lines/text use outlined style with semi-transparent outline (alpha ~0.6) for visibility on any background (dark to light)
- Text rendering: Draw outline first (stroke), then fill, both with antialiasing enabled
- Validate metadata on load, gracefully handle missing/corrupted tags

**References**:
- QImageWriter Documentation: https://doc.qt.io/qt-6/qimagewriter.html
- Exiv2 Library: https://exiv2.org/
- TIFF Specification: https://www.adobe.io/open/standards/TIFF.html

---

### 4. OpenCV Integration for Image Stitching and EDF

**Question**: What OpenCV modules and algorithms are needed for stitching (P7) and extended depth of focus (P8)?

**Decision**: Use OpenCV Stitcher API for panoramas, custom Laplacian pyramid for EDF

**Rationale**:
- **Stitching**: `cv::Stitcher::create()` provides high-level panorama stitching with feature detection, matching, and blending
- **EDF**: Laplacian pyramid focus stacking - compute Laplacian for each frame, select sharpest regions, blend into composite
- OpenCV is cross-platform (Linux, Windows, macOS, Android, iOS) with CMake integration
- Android SDK already present in repo (OpenCV-android-sdk/)

**Alternatives Considered**:
- **Manual feature matching for stitching**: Rejected - complex, error-prone, reinventing wheel
- **FFT-based EDF**: Rejected - slower than Laplacian, similar quality
- **Hugin/Enblend CLI tools**: Rejected - external dependencies, no API integration

**Implementation Notes**:
- Stitching: Convert QImage → cv::Mat, call `stitcher.stitch(images, pano)`, convert back to QImage
- EDF: For each frame, compute `cv::Laplacian()`, find max response per pixel across stack, blend using mask
- Handle OpenCV exceptions gracefully (insufficient overlap, focus stack quality issues)
- Provide progress callbacks via Qt signals for long operations (30s target per SC-008/SC-009)

**References**:
- OpenCV Stitcher Documentation: https://docs.opencv.org/4.x/d8/d19/tutorial_stitcher.html
- Focus Stacking Tutorial: https://docs.opencv.org/4.x/d5/daf/tutorial_py_histogram_equalization.html
- OpenCV-Qt Integration: https://wiki.qt.io/How_to_use_OpenCV_with_Qt

---

### 5. Tesseract OCR for Scale Bar Detection

**Question**: How to integrate Tesseract OCR for detecting and reading scale bar text (e.g., "100 µm", "50 um")?

**Decision**: Use Tesseract C++ API with custom preprocessing pipeline

**Rationale**:
- Tesseract 4.x provides robust OCR with Unicode support (handles µm, μm variants)
- Preprocessing: Convert to grayscale, adaptive thresholding, morphological operations to isolate scale bar region
- Pattern: Detect horizontal/vertical lines via Hough transform, crop region near line, run OCR on text
- Parse OCR result to extract number + unit (regex: `(\d+(?:\.\d+)?)\s*([µμu]?m|mm|nm|Å)`)

**Alternatives Considered**:
- **Cloud OCR APIs (Google Vision, Azure)**: Rejected - requires internet, privacy concerns, cost
- **Manual pattern matching without OCR**: Rejected - fragile, doesn't handle font variations
- **Template matching for digits**: Rejected - limited to specific fonts/sizes

**Implementation Notes**:
- Install Tesseract via package manager (Linux: apt/yum, Windows: installer, macOS: Homebrew)
- Link against libtesseract via CMake `find_package(Tesseract)`
- Preprocessing: `cv::cvtColor()`, `cv::adaptiveThreshold()`, `cv::HoughLinesP()` for line detection
- Run OCR on cropped region, parse result, calculate confidence score
- If confidence < 70%, prompt user for manual correction (FR-017.5, FR-017.6)

**References**:
- Tesseract Documentation: https://tesseract-ocr.github.io/
- Tesseract C++ API: https://tesseract-ocr.github.io/tessapi/5.x/
- OpenCV Hough Lines: https://docs.opencv.org/4.x/d9/db0/tutorial_hough_lines.html

---

### 6. RTSP Streaming for Classroom Collaboration

**Question**: How to implement RTSP server and client for live video + annotation streaming (P10)?

**Decision**: Use GStreamer with RTSP server plugin for teacher, Qt GStreamer integration for students

**Rationale**:
- GStreamer provides mature RTSP server implementation (`gst-rtsp-server`)
- Supports H.264 encoding (hardware acceleration available), RTP packetization, UDP/TCP transport
- Annotation embedding via custom RTP payload or RTSP metadata track
- Qt can integrate GStreamer via `QMediaPlayer` or custom `QVideoSink` pipeline

**Alternatives Considered**:
- **FFmpeg + libavformat RTSP**: Rejected - complex setup, limited RTSP server support
- **Custom RTSP implementation**: Rejected - massive undertaking, RFC 2326 compliance burden
- **WebRTC**: Rejected - overkill for LAN streaming, NAT traversal not needed, higher latency

**Implementation Notes**:
- **Teacher (server)**: GStreamer pipeline: `appsrc → x264enc → rtph264pay → rtspsink`, feed QVideoFrame data to appsrc
- **Student (client)**: GStreamer pipeline: `rtspsrc → rtph264depay → avdec_h264 → videoconvert → appsink`, emit QVideoFrame from appsink
- **Service Discovery**: UDP broadcast with JSON payload (teacher name, IP, port, stream URL) every 2 seconds
- **mDNS/Bonjour**: Use `avahi-daemon` (Linux), `dns-sd` (macOS), Bonjour SDK (Windows) for subnet traversal
- **Annotations**: Embed in RTSP metadata track or custom RTP payload, parse on client, render as overlay

**References**:
- GStreamer RTSP Server: https://gstreamer.freedesktop.org/documentation/gst-rtsp-server/
- Qt GStreamer Integration: https://doc.qt.io/qt-6/qtmultimedia-gstreamer.html
- mDNS/Bonjour: https://developer.apple.com/bonjour/

---

### 7. Contour-Based Object Detection with OpenCV

**Question**: How to implement automated spore/object detection using contour detection and morphological operations (P5)?

**Decision**: OpenCV pipeline: grayscale → Gaussian blur → Otsu/adaptive thresholding → morphological ops → findContours → filter by size/circularity

**Rationale**:
- Proven workflow for microscopy object detection (spores, cells, particles)
- No training data required, parameters are user-adjustable (size range, circularity, threshold)
- Fast processing (target: <10s for 100 objects per SC-012.3)
- Robust to varying contrast and backgrounds

**Alternatives Considered**:
- **Template matching**: Rejected - requires reference template, assumes rigid shapes
- **Blob detector (SimpleBlobDetector)**: Rejected - limited to circular objects, less flexible
- **Machine learning (YOLO, segmentation models)**: Rejected - requires training data, GPU, overkill for well-contrasted specimens

**Implementation Notes**:
- Pipeline: `cv::cvtColor()` → `cv::GaussianBlur()` → `cv::threshold(OTSU)` or `cv::adaptiveThreshold()`
- Morphological operations: `cv::erode()`, `cv::dilate()`, `cv::morphologyEx(OPEN/CLOSE)` to remove noise
- Contour detection: `cv::findContours()` → filter by area (size range), circularity (`4π×area/perimeter²`), aspect ratio
- User parameters: min/max area (pixels²), circularity threshold (0-1), morphology kernel size
- Output: vector of contours, each with centroid, bounding box, area, perimeter, circularity

**References**:
- OpenCV Contours Tutorial: https://docs.opencv.org/4.x/d4/d73/tutorial_py_contours_begin.html
- Morphological Transformations: https://docs.opencv.org/4.x/d9/d61/tutorial_py_morphological_ops.html

---

### 8. Cross-Platform File Storage with QStandardPaths

**Question**: How to implement cross-platform default save location (~/Documents/uScope/)?

**Decision**: Use `QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)` + "/uScope"

**Rationale**:
- Returns platform-appropriate Documents folder:
  - Linux: `~/Documents/` or `$XDG_DOCUMENTS_DIR`
  - Windows: `C:\Users\<username>\Documents\`
  - macOS: `~/Documents/`
  - Android: app-specific external storage (`/sdcard/Android/data/<package>/files/Documents/`)
  - iOS: app sandbox Documents directory
- Automatically handles localization, user relocation, and permissions
- Qt handles path separator differences (`/` vs `\`)

**Alternatives Considered**:
- **Hardcoded paths**: Rejected - violates cross-platform principle, breaks on non-English systems
- **QDir::homePath() + "/Documents"**: Rejected - doesn't handle XDG, localized folder names, mobile
- **Current directory**: Rejected - unpredictable, not user-friendly

**Implementation Notes**:
```cpp
QString baseDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
QString uScopeDir = baseDir + "/uScope";
QDir().mkpath(uScopeDir); // Create if doesn't exist
QString filename = uScopeDir + "/image_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".png";
```

**References**:
- QStandardPaths Documentation: https://doc.qt.io/qt-6/qstandardpaths.html

---

### 9. Qt Widgets vs Qt Quick for Desktop vs Mobile UI

**Question**: Should we use Qt Widgets (desktop-focused) or Qt Quick/QML (touch-focused) for UI?

**Decision**: **Hybrid approach** - Qt Widgets for desktop (Linux, Windows, macOS), Qt Quick/QML for mobile (Android, iOS), shared C++ backend

**Rationale**:
- **Desktop**: Qt Widgets provides traditional controls optimal for mouse/keyboard, existing codebase uses Widgets (MainWindow.cpp, .ui files)
- **Mobile**: Qt Quick/QML provides touch-optimized controls, better performance on mobile GPUs, declarative UI ideal for responsive layouts
- **Shared Backend**: Camera, image processing, calibration, measurement logic implemented in C++ classes, exposed to QML via `Q_INVOKABLE` methods and `Q_PROPERTY`

**Alternatives Considered**:
- **Widgets only**: Rejected - poor touch experience on mobile, manual DPI scaling complex
- **QML only**: Rejected - redesigning existing desktop UI wasteful, Widgets more mature for complex desktop controls
- **Separate codebases**: Rejected - maintenance burden, duplicated logic

**Implementation Notes**:
- Desktop: Continue using existing .ui files (mainwindow.ui), QMainWindow, QGraphicsView for video preview
- Mobile: Create `qml/` directory with main.qml, components/, pages/ for touch UI
- Backend: Expose C++ classes to QML via `qmlRegisterType()`, connect signals to QML handlers
- Shared resources: Keep images/, sounds/ cross-platform

**References**:
- Qt Widgets Overview: https://doc.qt.io/qt-6/qtwidgets-index.html
- Qt Quick Overview: https://doc.qt.io/qt-6/qtquick-index.html
- Integrating QML and C++: https://doc.qt.io/qt-6/qtqml-cppintegration-overview.html

---

### 10. iNaturalist OAuth Integration

**Question**: How to implement OAuth authentication with iNaturalist API for observation linking (P11)?

**Decision**: Use Qt Network module (`QNetworkAccessManager`, `QOAuthHttpServerReplyHandler`) for OAuth 2.0 flow

**Rationale**:
- Qt provides OAuth support via Qt Network Extras (Qt 6.x includes basic OAuth)
- iNaturalist uses OAuth 2.0 with authorization code flow
- `QOAuthHttpServerReplyHandler` handles callback URL on localhost
- Store access token securely in QSettings (encrypted on mobile platforms)

**Alternatives Considered**:
- **Manual OAuth implementation**: Rejected - error-prone, security risks, reinventing wheel
- **WebView for authentication**: Rejected - heavy dependency, platform differences
- **Third-party OAuth library**: Rejected - additional dependency, Qt Network sufficient

**Implementation Notes**:
- Register app with iNaturalist to get client ID/secret
- OAuth flow: Open browser to authorization URL → user logs in → callback to localhost:PORT → exchange code for token
- Store token in QSettings with expiration timestamp
- Detect expired token (401 response), prompt re-authentication (FR-060)
- API calls: Use `QNetworkAccessManager` with Bearer token in Authorization header
- Upload images: POST to `/observations/:id/photos` endpoint with multipart/form-data

**References**:
- iNaturalist API Documentation: https://www.inaturalist.org/pages/api+reference
- Qt Network Authorization: https://doc.qt.io/qt-6/qtnetworkauth-index.html

---

### 11. Resolution-Independent Tile-Based UI Architecture

**Question**: How to implement a custom tile-based overlay UI system with proportional scaling that maintains consistent appearance across all window sizes and resolutions?

**Decision**: Use QGraphicsView with QGraphicsScene for video background, overlay custom QGraphicsWidget-based tiles with proportional geometry calculated from window height

**Rationale**:
- **QGraphicsView/Scene**: Provides layered rendering (video as background item, tiles as foreground items), built-in coordinate system, hardware-accelerated rendering
- **Proportional Scaling**: Base tile size = window_height / 12 ensures all UI elements scale proportionally when window resizes
- **Tile System**: Tiles are squares or rectangles (sides = N × base_tile_size), anchored to left/right/center of window
- **Transparency**: Tiles have semi-transparent backgrounds (alpha channel) allowing video to show through, state-based colors (idle, hover, active, disabled)
- **Modal Dialogs**: Centered tiles that dim video background, with optional QScrollArea if content exceeds tile area

**Alternatives Considered**:
- **QMainWindow with QDockWidgets**: Rejected - fixed layouts don't scale proportionally, no transparency over video
- **Qt Quick/QML for desktop**: Rejected - existing Widgets codebase, QML overkill for desktop, performance concerns for high-FPS video
- **Fixed pixel sizes with manual DPI scaling**: Rejected - complex to maintain, doesn't handle arbitrary window sizes smoothly
- **CSS-like percentage layouts**: Rejected - Qt Widgets doesn't support percentage-based geometry natively

**Implementation Notes**:

**Base Tile Class Design**:
```cpp
class Tile : public QGraphicsWidget {
    Q_OBJECT
public:
    enum Anchor { Left, Right, Center };
    enum TileState { Idle, Hover, Active, Disabled };
    
    // Constructor: specify tile size in multiples of base tile (e.g., 1×1, 2×1, 3×2)
    Tile(int widthMultiplier, int heightMultiplier, Anchor anchor, QGraphicsItem* parent = nullptr);
    
    // Recalculate geometry when window resizes
    void updateGeometry(int windowWidth, int windowHeight);
    
    // State management (changes background color)
    void setState(TileState state);
    
    // Appearance (corner radius scales with tile size)
    int cornerRadius() const { return _baseTileSize / 8; }
    
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    
private:
    int _widthMult, _heightMult;
    Anchor _anchor;
    TileState _state;
    QColor _backgroundColor;  // State-dependent with alpha transparency
    qreal _transparency = 0.7;  // Constant transparency (0.0-1.0)
};
```

**MainWindow Integration**:
```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    
protected:
    void resizeEvent(QResizeEvent* event) override;
    
private:
    QGraphicsView* _view;
    VideoGraphicsScene* _scene;  // Custom scene with video background
    
    // Tile collections (auto-layout on resize)
    QList<Tile*> _leftTiles;    // Camera selection, settings, etc.
    QList<Tile*> _rightTiles;   // Capture, record, measurement tools
    QList<Tile*> _centerTiles;  // Modal dialogs (calibration, export, etc.)
    
    void updateTileLayout();    // Called on resize
    int calculateBaseTileSize() const { return height() / 12; }
};
```

**Proportional Scaling Logic**:
- On window resize: `baseTileSize = windowHeight / 12`
- Each tile geometry: `QRectF(x, y, widthMult * baseTileSize, heightMult * baseTileSize)`
- Anchoring:
  - **Left**: `x = margin`
  - **Right**: `x = windowWidth - tileWidth - margin`
  - **Center**: `x = (windowWidth - tileWidth) / 2`
- Icons/text scale proportionally using `QFont::setPixelSize(baseTileSize / 4)` and SVG scaling

**Tile Variants**:
- **TileLabel**: Base tile with icon (SVG, scaled to tile size) + text below
- **TileButton**: Clickable tile with pressed state animation
- **TileCombo**: Tile with embedded QComboBox (dropdown for camera/objective selection)
- **TileSlider**: Tile with QSlider (exposure, gain controls)
- **TileDialog**: Modal centered tile (2×2 or 3×3 base tiles) with sub-tiles, optional scroll area

**State-Based Colors** (with alpha transparency):
- **Idle**: `QColor(100, 100, 100, 180)`  // Gray, 70% opaque
- **Hover**: `QColor(120, 120, 150, 200)` // Light blue, 78% opaque
- **Active**: `QColor(80, 150, 80, 220)`  // Green, 86% opaque
- **Disabled**: `QColor(80, 80, 80, 150)` // Dark gray, 59% opaque

**Rendering** (with rounded corners):
```cpp
void Tile::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);
    QRectF rect = boundingRect();
    int radius = cornerRadius();  // baseTileSize / 8
    
    // Draw rounded rectangle background
    painter->setBrush(getStateColor());
    painter->setPen(_borderWidth > 0 ? QPen(_borderColor, _borderWidth) : Qt::NoPen);
    painter->drawRoundedRect(rect, radius, radius);
    
    // Draw icon and text (implementation in derived classes)
}
```

**Modal Dialog Behavior**:
- Dim video background: Add semi-transparent black overlay (`QGraphicsRectItem` with `QColor(0, 0, 0, 180)`)
- Center dialog tile on top of overlay
- Hide overlay + dialog on close (Esc key, close button on dialog)
- If dialog content > tile area: Embed `QGraphicsProxyWidget` with `QScrollArea`

**Performance Considerations**:
- Update tile geometry only on resize events (not every frame)
- Cache scaled SVG icons at current resolution (regenerate on resize)
- Use `QGraphicsItem::ItemIgnoresTransformations` flag for tiles (prevent view zoom affecting tile sizes)
- Limit repaint to dirty tiles only (use `QGraphicsItem::update()` instead of scene-wide `update()`)

**References**:
- QGraphicsView Framework: https://doc.qt.io/qt-6/graphicsview.html
- QGraphicsWidget: https://doc.qt.io/qt-6/qgraphicswidget.html
- Custom Widgets Tutorial: https://doc.qt.io/qt-6/qtwidgets-widgets-customwidgets-example.html

---

### 12. GitHub Actions CI/CD for Cross-Platform Packaging

**Question**: How to implement automated build and packaging workflows for all target platforms (Linux DEB/RPM, Windows installer, macOS DMG, Android APK, iOS IPA) using GitHub Actions?

**Decision**: Multi-workflow GitHub Actions setup with platform-specific runners and packaging tools - separate workflows for PR builds (artifacts only) and release builds (publish to GitHub Releases)

**Rationale**:
- **GitHub Actions**: Free for public repos, provides Linux, Windows, macOS runners, integrates with GitHub Releases
- **Platform-Specific Packaging**:
  - **Linux DEB**: Use `dpkg-deb` with DEBIAN control files, targets Ubuntu/Debian (apt-based distros)
  - **Linux RPM**: Use `rpmbuild` with .spec files, targets Fedora/RHEL/CentOS (yum/dnf-based distros)
  - **Windows**: WiX Toolset 3.x/4.x for MSI installer (professional, signed installers)
  - **macOS**: `create-dmg` tool for DMG disk image (drag-to-Applications UX)
  - **Android**: Gradle with Qt for Android plugin, produces signed APK
  - **iOS**: Xcode with Qt for iOS, produces IPA (requires macOS runner, Apple Developer account for signing)
- **Dependency Management**: Install Qt 6.10+ via aqtinstall (Python tool for Qt binaries), OpenCV via package managers (apt/brew/vcpkg), Tesseract via package managers, GStreamer via package managers
- **Artifact Strategy**: PR builds upload artifacts to GitHub Actions (7-day retention, testing only), release tag builds publish to GitHub Releases (permanent, public download)

**Alternatives Considered**:
- **Travis CI / CircleCI**: Rejected - less integrated with GitHub, limited free tier
- **AppVeyor**: Rejected - Windows-only, less flexible than GitHub Actions multi-platform
- **Self-hosted runners**: Rejected - maintenance overhead, not needed for open-source project
- **Docker for all platforms**: Rejected - iOS requires macOS runner (no Docker), Windows/macOS desktop better with native runners
- **Single monolithic workflow**: Rejected - hard to debug, long runtime, prefer parallel platform builds
- **CMake CPack only**: Rejected - limited packaging customization (e.g., WiX features, DMG background), CPack better suited for simple tarballs

**Implementation Notes**:

**Workflow Structure**:
```
.github/workflows/
├── build-pr.yml           # Trigger: on PR - calls all platform workflows, uploads artifacts
├── build-release.yml      # Trigger: on tag push (v*.*.*) - calls all platform workflows, publishes to GitHub Releases
├── build-linux-deb.yml    # Reusable: builds DEB package
├── build-linux-rpm.yml    # Reusable: builds RPM package
├── build-windows.yml      # Reusable: builds WiX MSI installer
├── build-macos.yml        # Reusable: builds DMG disk image
├── build-android.yml      # Reusable: builds APK
└── build-ios.yml          # Reusable: builds IPA (requires secrets for signing)
```

**Linux DEB Workflow** (`build-linux-deb.yml`):
- Runner: `ubuntu-22.04` (or `ubuntu-latest`)
- Install dependencies: `sudo apt-get install qt6-base-dev qt6-multimedia-dev libopencv-dev tesseract-ocr libtesseract-dev libgstreamer1.0-dev gstreamer1.0-rtsp gstreamer1.0-plugins-good libexiv2-dev`
- Install Qt 6.10 via `aqtinstall` if not in apt repos: `pip install aqtinstall && aqt install-qt linux desktop 6.10.0 gcc_64`
- Build: `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build`
- Create DEBIAN package structure:
  ```
  package/
  ├── DEBIAN/
  │   ├── control  # Package metadata (name, version, dependencies, description)
  │   ├── postinst # Post-install script (desktop entry, icon cache update)
  │   └── prerm    # Pre-remove script (cleanup)
  ├── usr/
  │   ├── bin/
  │   │   └── uscope  # Executable
  │   ├── share/
  │   │   ├── applications/
  │   │   │   └── uscope.desktop  # Desktop entry
  │   │   ├── icons/
  │   │   │   └── hicolor/scalable/apps/uscope.svg  # Icon
  │   │   └── doc/
  │   │       └── uscope/
  │   │           ├── README.md
  │   │           └── LICENSE
  │   └── lib/
  │       └── uscope/  # Shared libraries (if any)
  ```
- Package: `dpkg-deb --build package uscope_<version>_amd64.deb`
- Upload artifact: `actions/upload-artifact@v4` with name `uscope-deb`

**Linux RPM Workflow** (`build-linux-rpm.yml`):
- Runner: `ubuntu-22.04` with `rpm` and `rpmbuild` tools installed
- Alternative: Use Fedora container (`container: fedora:latest`) for native RPM build
- Install dependencies: `dnf install qt6-qtbase-devel qt6-qtmultimedia-devel opencv-devel tesseract-devel gstreamer1-devel gstreamer1-rtsp-server exiv2-devel`
- Build: Same CMake commands
- Create `.spec` file:
  ```spec
  Name:           uscope
  Version:        <version>
  Release:        1%{?dist}
  Summary:        Open-source microscopy platform
  License:        <LICENSE>
  URL:            https://github.com/MatejFranceskin/uScope
  Source0:        %{name}-%{version}.tar.gz
  
  BuildRequires:  cmake, qt6-qtbase-devel, qt6-qtmultimedia-devel, opencv-devel, tesseract-devel, gstreamer1-devel
  Requires:       qt6-qtbase, qt6-qtmultimedia, opencv, tesseract, gstreamer1, gstreamer1-rtsp-server
  
  %description
  μScope is a cross-platform microscopy application for educational institutions...
  
  %build
  cmake -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build
  
  %install
  install -D -m 0755 build/uScope %{buildroot}/usr/bin/uscope
  install -D -m 0644 images/uScope.svg %{buildroot}/usr/share/icons/hicolor/scalable/apps/uscope.svg
  ...
  
  %files
  /usr/bin/uscope
  /usr/share/applications/uscope.desktop
  /usr/share/icons/hicolor/scalable/apps/uscope.svg
  ...
  ```
- Package: `rpmbuild -ba uscope.spec`
- Upload artifact: `actions/upload-artifact@v4` with name `uscope-rpm`

**Windows WiX Workflow** (`build-windows.yml`):
- Runner: `windows-latest`
- Install Qt 6.10: `pip install aqtinstall && aqt install-qt windows desktop 6.10.0 win64_msvc2019_64`
- Install OpenCV: `vcpkg install opencv[core,imgproc,video]:x64-windows`
- Install Tesseract: Download from UB Mannheim installer or vcpkg
- Install GStreamer: Download from gstreamer.freedesktop.org (MSVC runtime bundle)
- Install WiX Toolset: `choco install wixtoolset` or download from wixtoolset.org
- Build: `cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=<Qt_path> && cmake --build build --config Release`
- Create WiX `.wxs` source file:
  ```xml
  <?xml version="1.0"?>
  <Wix xmlns="http://schemas.microsoft.com/wix/2006/wi">
    <Product Id="*" Name="μScope" Version="<version>" Manufacturer="MatejFranceskin" Language="1033">
      <Package InstallerVersion="200" Compressed="yes"/>
      <Media Id="1" Cabinet="uscope.cab" EmbedCab="yes"/>
      
      <Directory Id="TARGETDIR" Name="SourceDir">
        <Directory Id="ProgramFilesFolder">
          <Directory Id="INSTALLFOLDER" Name="uScope">
            <Component Id="MainExecutable" Guid="<GUID>">
              <File Id="uScopeEXE" Source="build\Release\uScope.exe" KeyPath="yes">
                <Shortcut Id="DesktopShortcut" Directory="DesktopFolder" Name="μScope" WorkingDirectory="INSTALLFOLDER" Icon="uScopeIcon.ico"/>
                <Shortcut Id="StartMenuShortcut" Directory="ProgramMenuFolder" Name="μScope" WorkingDirectory="INSTALLFOLDER" Icon="uScopeIcon.ico"/>
              </File>
            </Component>
            <!-- Qt DLLs, OpenCV DLLs, etc. -->
          </Directory>
        </Directory>
      </Directory>
      
      <Icon Id="uScopeIcon.ico" SourceFile="images\uScope.ico"/>
      <Feature Id="Complete" Level="1">
        <ComponentRef Id="MainExecutable"/>
      </Feature>
    </Product>
  </Wix>
  ```
- Compile: `candle.exe uscope.wxs && light.exe uscope.wixobj -out uscope-<version>-x64.msi`
- Upload artifact: `actions/upload-artifact@v4` with name `uscope-windows`

**macOS DMG Workflow** (`build-macos.yml`):
- Runner: `macos-latest`
- Install Qt 6.10: `brew install qt@6` or aqtinstall
- Install dependencies: `brew install opencv tesseract gstreamer gst-rtsp-server exiv2`
- Build: `cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6) && cmake --build build`
- Create .app bundle: `macdeployqt build/uScope.app -always-overwrite -verbose=2`
- Install `create-dmg`: `npm install -g create-dmg` or `brew install create-dmg`
- Create DMG: `create-dmg build/uScope.app --overwrite --dmg-title="μScope" --icon-size=100 --window-size=600 400 --app-drop-link=425 120`
- Upload artifact: `actions/upload-artifact@v4` with name `uscope-macos`

**Android APK Workflow** (`build-android.yml`):
- Runner: `ubuntu-latest`
- Install dependencies: `sudo apt-get install openjdk-11-jdk android-sdk`
- Setup Android SDK/NDK: Use `android-actions/setup-android@v2` action
- Install Qt for Android: `aqt install-qt linux android 6.10.0 android_armv7 android_arm64_v8a`
- Install OpenCV Android SDK: Already in repo at `OpenCV-android-sdk/`
- Build with Gradle:
  ```bash
  cd android
  ./gradlew assembleRelease
  ```
- Sign APK: Use `zipalign` and `apksigner` with keystore (stored in GitHub Secrets)
- Upload artifact: `actions/upload-artifact@v4` with name `uscope-android`

**iOS IPA Workflow** (`build-ios.yml`):
- Runner: `macos-latest` (iOS build requires macOS)
- Install Qt for iOS: `aqt install-qt mac ios 6.10.0 ios`
- Install dependencies: OpenCV iOS framework, Tesseract iOS build (may need custom build or CocoaPods)
- Build with Xcode:
  ```bash
  xcodebuild -project ios/uScope.xcodeproj -scheme uScope -configuration Release -sdk iphoneos archive -archivePath build/uScope.xcarchive
  xcodebuild -exportArchive -archivePath build/uScope.xcarchive -exportPath build -exportOptionsPlist ios/ExportOptions.plist
  ```
- Code signing: Requires Apple Developer certificate and provisioning profile (stored in GitHub Secrets)
- Upload artifact: `actions/upload-artifact@v4` with name `uscope-ios`

**PR Build Workflow** (`build-pr.yml`):
```yaml
name: Build PR
on:
  pull_request:
    branches: [main]

jobs:
  build-all-platforms:
    strategy:
      matrix:
        include:
          - {os: ubuntu-22.04, workflow: build-linux-deb.yml}
          - {os: ubuntu-22.04, workflow: build-linux-rpm.yml}
          - {os: windows-latest, workflow: build-windows.yml}
          - {os: macos-latest, workflow: build-macos.yml}
          - {os: ubuntu-latest, workflow: build-android.yml}
          - {os: macos-latest, workflow: build-ios.yml}
    uses: ./.github/workflows/${{ matrix.workflow }}
    with:
      upload-artifacts: true
```

**Release Build Workflow** (`build-release.yml`):
```yaml
name: Build Release
on:
  push:
    tags:
      - 'v*.*.*'

jobs:
  build-all-platforms:
    strategy:
      matrix:
        include:
          - {os: ubuntu-22.04, workflow: build-linux-deb.yml}
          - {os: ubuntu-22.04, workflow: build-linux-rpm.yml}
          - {os: windows-latest, workflow: build-windows.yml}
          - {os: macos-latest, workflow: build-macos.yml}
          - {os: ubuntu-latest, workflow: build-android.yml}
          - {os: macos-latest, workflow: build-ios.yml}
    uses: ./.github/workflows/${{ matrix.workflow }}
  
  create-release:
    needs: build-all-platforms
    runs-on: ubuntu-latest
    steps:
      - name: Download all artifacts
        uses: actions/download-artifact@v4
      - name: Create GitHub Release
        uses: softprops/action-gh-release@v1
        with:
          files: |
            uscope-deb/*.deb
            uscope-rpm/*.rpm
            uscope-windows/*.msi
            uscope-macos/*.dmg
            uscope-android/*.apk
            uscope-ios/*.ipa
          draft: false
          prerelease: false
```

**Version Extraction**:
- Parse version from `CMakeLists.txt` (`project(uScope VERSION x.y.z)`) or git tag
- Use `grep` or `sed` in workflow: `VERSION=$(grep -oP 'project\(uScope VERSION \K[0-9.]+' CMakeLists.txt)`
- Pass as environment variable to packaging steps

**Secrets Required** (stored in GitHub repository secrets):
- `ANDROID_KEYSTORE`: Base64-encoded keystore file for APK signing
- `ANDROID_KEYSTORE_PASSWORD`: Keystore password
- `ANDROID_KEY_ALIAS`: Key alias
- `IOS_CERTIFICATE`: Base64-encoded Apple Developer certificate (.p12)
- `IOS_CERTIFICATE_PASSWORD`: Certificate password
- `IOS_PROVISIONING_PROFILE`: Base64-encoded provisioning profile

**Testing Strategy**:
- PR builds: Smoke test only (application launches, no crashes)
- Release builds: Manual testing on representative hardware before publishing
- Future: Add automated UI tests (Qt Test framework) to workflows

**References**:
- GitHub Actions Documentation: https://docs.github.com/en/actions
- Qt Installer Framework: https://doc.qt.io/qtinstallerframework/
- WiX Toolset: https://wixtoolset.org/
- create-dmg: https://github.com/create-dmg/create-dmg
- aqtinstall: https://github.com/miurahr/aqtinstall
- Debian Packaging: https://www.debian.org/doc/manuals/maint-guide/
- RPM Packaging: https://rpm-packaging-guide.github.io/

---

## Technology Stack Summary

| Component | Technology | Rationale |
|-----------|-----------|-----------|
| **Language** | C++17 | Qt requirement, performance, cross-platform |
| **UI Framework (Desktop)** | Qt 6.10+ Widgets with custom tile system | Resolution-independent scaling, video overlay support, proportional geometry |
| **UI Architecture** | QGraphicsView/Scene with tile overlays | Layered rendering (video background + transparent tiles), hardware-accelerated |
| **UI Framework (Mobile)** | Qt 6.10+ Quick/QML | Touch-optimized, declarative, GPU-accelerated |
| **Camera API** | Qt Multimedia (QCamera, QMediaDevices) | Cross-platform, unified API, Qt-integrated |
| **Video Encoding** | QMediaRecorder (H.264 via platform backends) | Qt-integrated, H.264 support, CRF control |
| **Image Processing** | Qt (QImage, QPainter) + OpenCV 4.x | Qt for basic ops, OpenCV for advanced (stitching, EDF, detection) |
| **OCR** | Tesseract 4.x | Robust, Unicode support, cross-platform |
| **RTSP Streaming** | GStreamer + gst-rtsp-server | Mature, hardware acceleration, annotation embedding |
| **Service Discovery** | UDP broadcast + mDNS/Bonjour | LAN discovery, multi-subnet support |
| **Object Detection** | OpenCV (contours + morphology) | No training data, user-adjustable, fast |
| **Metadata Embedding** | Qt (PNG text) + exiv2 (EXIF/TIFF) | Standards-compliant, embedded metadata |
| **File Storage** | QStandardPaths + QFile/QDir | Cross-platform, user-friendly locations |
| **OAuth** | Qt Network (QOAuthHttpServerReplyHandler) | Qt-integrated, secure token handling |
| **Build System** | CMake 3.16+ (desktop), Gradle (Android), Xcode (iOS) | Qt-compatible, cross-platform |
| **CI/CD** | GitHub Actions | Free for public repos, multi-platform runners, artifact storage, GitHub Releases integration |
| **Packaging** | dpkg-deb (DEB), rpmbuild (RPM), WiX (MSI), create-dmg (DMG), Gradle (APK), Xcode (IPA) | Platform-native package formats, professional installers |

## Risks and Mitigation

| Risk | Impact | Mitigation |
|------|--------|-----------|
| **Camera compatibility variance** | Some UVC cameras may not expose all controls via Qt Multimedia | Extensive testing with diverse camera models, fallback to basic controls, document supported cameras |
| **OpenCV build complexity** | OpenCV can be complex to build/link on some platforms | Use pre-built packages (Linux: apt, Windows: vcpkg, macOS: Homebrew, Android: SDK in repo) |
| **Tesseract accuracy on low-DPI images** | OCR may fail on blurry/low-contrast scale bars | Provide confidence score, manual correction UI (FR-017.5), fallback to manual calibration (FR-017.6) |
| **RTSP streaming latency on WiFi** | WiFi congestion may exceed 500ms target | Adaptive bitrate (FR-070), connection quality indicator (FR-069), recommend wired LAN for classroom use |
| **Mobile camera API differences** | Android/iOS camera permissions and lifecycle differ | Handle permissions declaratively (AndroidManifest.xml, Info.plist), test lifecycle events (SC-015) |
| **Annotation metadata compatibility** | Third-party viewers won't display embedded annotations | Document clearly (assumptions section), provide export-with-burned-annotations option (FR-025) |
| **GitHub Actions runner limits** | Free tier has usage limits, iOS builds require macOS runners (limited availability) | Use workflow caching for dependencies, optimize build times, consider self-hosted runner for iOS if needed |
| **Code signing complexity** | iOS requires Apple Developer account ($99/year), Windows MSI signing requires certificate | Document signing requirements, provide unsigned builds for testing, store certificates as GitHub Secrets |

## Best Practices Research

### Qt Multimedia Camera Best Practices
- Always check `QCamera::isAvailable()` before starting
- Use `QVideoSink` instead of deprecated `QAbstractVideoSurface`
- Handle `QCamera::errorOccurred()` signal for device disconnect
- Set `QCameraFormat` explicitly to avoid auto-selection issues
- Release camera in `closeEvent()` to prevent exclusive access locks

### OpenCV-Qt Integration Best Practices
- Convert QImage ↔ cv::Mat efficiently: share data when possible (same pixel format)
- Run OpenCV operations in separate thread (`QThread`) to avoid blocking UI
- Use `QFuture` and `QtConcurrent::run()` for async processing with progress updates
- Handle OpenCV exceptions (`cv::Exception`) and translate to Qt-friendly errors

### Annotation Rendering Best Practices
- Always enable antialiasing: `painter->setRenderHint(QPainter::Antialiasing)`
- Use outlined style: Draw outline (stroke) first with semi-transparent color, then fill
- Outline transparency: ~60% opacity (alpha ~150) ensures visibility without obscuring background
- Line width: Outline should be 2-3px wider than main line for good contrast
- Text rendering: Use `QPainterPath` for outlined text (stroke + fill) instead of `drawText()`
- Color contrast: Choose bright fill colors (yellow, cyan, magenta) with dark outlines for maximum visibility

### Cross-Platform File Handling Best Practices
- Always use `QDir::toNativeSeparators()` when displaying paths to users
- Check `QFile::exists()` before operations, handle `QFile::error()` codes
- Use `QFileInfo::isWritable()` to check permissions before saving
- Respect platform storage limits (check free space on mobile)

### Mobile-Specific Best Practices
- Request camera permission before accessing `QMediaDevices::videoInputs()`
- Handle `QGuiApplication::applicationStateChanged()` to pause camera on background
- Use `QScreen::devicePixelRatio()` for DPI-aware layouts
- Test on actual devices (emulators have limited camera functionality)
- Optimize image loading (use scaled versions for thumbnails)

## Next Steps

Phase 1 deliverables:
1. **data-model.md**: Entity definitions with relationships, validation rules, state transitions
2. **contracts/**: API contracts for RTSP streaming, iNaturalist API, internal Qt signals/slots
3. **quickstart.md**: Developer onboarding guide with setup, build, test instructions
4. **Update agent context**: Add OpenCV, Tesseract, GStreamer to technology list for Copilot

---

**Research Status**: ✅ COMPLETE - All technical unknowns resolved, ready for Phase 1 design
