# Developer Quickstart: μScope Microscopy Platform

**Branch**: `001-microscopy-platform` | **Last Updated**: 2025-12-05

## Purpose

Get development environment set up and running in under 30 minutes. Build, run, and test μScope on your platform.

## Prerequisites

### All Platforms
- **Qt 6.10+**: Framework (Widgets, Multimedia, SVG, Quick modules)
- **CMake 3.16+**: Build system
- **C++17+ Compiler**: GCC 7+, Clang 5+, MSVC 2019+, or Xcode 10+
- **Git**: Version control

### Optional Dependencies (for advanced features)
- **OpenCV 4.x**: Image stitching, EDF, object detection (P5, P7, P8)
- **Tesseract 4.x**: OCR for scale bar detection (P3 scenarios 11-14)
- **GStreamer 1.x + gst-rtsp-server**: RTSP streaming for classroom mode (P10)
- **Avahi/Bonjour**: mDNS service discovery for RTSP

---

## Quick Setup (Linux)

### 1. Install Dependencies

**Ubuntu/Debian**:
```bash
# Core dependencies
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    qt6-base-dev \
    qt6-multimedia-dev \
    qt6-svg-dev \
    qt6-declarative-dev \
    libqt6multimedia6-plugins

# Optional: OpenCV for advanced features
sudo apt-get install -y \
    libopencv-dev

# Optional: Tesseract for OCR
sudo apt-get install -y \
    tesseract-ocr \
    libtesseract-dev

# Optional: GStreamer for RTSP streaming
sudo apt-get install -y \
    gstreamer1.0-tools \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-rtsp \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-rtsp

# Optional: Avahi for mDNS
sudo apt-get install -y \
    avahi-daemon \
    libavahi-client-dev
```

**Fedora/RHEL**:
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    git \
    qt6-qtbase-devel \
    qt6-qtmultimedia-devel \
    qt6-qtsvg-devel \
    qt6-qtdeclarative-devel \
    opencv-devel \
    tesseract-devel \
    gstreamer1-plugins-base-devel \
    avahi-devel
```

### 2. Clone Repository

```bash
git clone https://github.com/MatejFranceskin/uScope.git
cd uScope
git checkout 001-microscopy-platform
```

### 3. Build

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build (use -j for parallel build)
cmake --build build -j$(nproc)
```

### 4. Run

```bash
./build/uScope
```

**Expected behavior**: Application launches, camera dropdown appears, live preview starts if camera connected.

---

## Quick Setup (Windows)

### 1. Install Dependencies

**Using vcpkg** (recommended):
```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# Install Qt6
.\vcpkg install qt6-base:x64-windows qt6-multimedia:x64-windows qt6-svg:x64-windows

# Optional: OpenCV, Tesseract
.\vcpkg install opencv4:x64-windows tesseract:x64-windows
```

**OR download Qt from official installer**:
- https://www.qt.io/download-qt-installer
- Install Qt 6.10+ with MSVC 2019 64-bit

### 2. Clone Repository

```powershell
git clone https://github.com/MatejFranceskin/uScope.git
cd uScope
git checkout 001-microscopy-platform
```

### 3. Build

**Using Qt Creator**:
1. Open `CMakeLists.txt` in Qt Creator
2. Configure kit (MSVC 2019 64-bit with Qt 6.10)
3. Build → Build Project "uScope"

**Using command line**:
```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```

### 4. Run

```powershell
.\build\Release\uScope.exe
```

---

## Quick Setup (macOS)

### 1. Install Dependencies

**Using Homebrew**:
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Core dependencies
brew install qt@6 cmake

# Optional: OpenCV, Tesseract
brew install opencv tesseract

# Optional: GStreamer
brew install gstreamer gst-plugins-base gst-plugins-good gst-rtsp-server
```

### 2. Clone and Build

```bash
git clone https://github.com/MatejFranceskin/uScope.git
cd uScope
git checkout 001-microscopy-platform

# Configure (specify Qt path)
cmake -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)

# Build
cmake --build build -j$(sysctl -n hw.ncpu)
```

### 3. Run

```bash
./build/uScope.app/Contents/MacOS/uScope
```

---

## Quick Setup (Android)

### 1. Prerequisites

- **Android Studio**: Latest version with SDK 24+ (Android 6.0+)
- **Qt for Android**: Qt 6.10+ with Android support
- **Android NDK**: r21+ (included with Android Studio)
- **JDK**: OpenJDK 11 or later

### 2. Setup Qt Creator for Android

1. Open Qt Creator → Tools → Options → Devices → Android
2. Set Android SDK location (usually `~/Android/Sdk`)
3. Set Android NDK location
4. Set JDK location
5. Create Android Kit (Qt 6.10 for Android ARM64)

### 3. Build and Deploy

```bash
# Using Qt Creator:
# 1. Open CMakeLists.txt
# 2. Select Android ARM64 kit
# 3. Build → Deploy (deploys to connected device/emulator)

# OR using command line:
export ANDROID_SDK_ROOT=~/Android/Sdk
export ANDROID_NDK_ROOT=~/Android/Sdk/ndk/26.1.10909125

cmake -B build-android \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-24 \
    -DQT_HOST_PATH=/usr/lib/qt6 \
    -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/android_arm64_v8a

cmake --build build-android --target apk
```

**Deploy to device**:
```bash
adb install build-android/android-build/build/outputs/apk/debug/android-build-debug.apk
```

---

## Quick Setup (iOS)

### 1. Prerequisites

- **macOS**: Required for iOS development
- **Xcode**: 12+ with iOS SDK 13+
- **Qt for iOS**: Qt 6.10+ with iOS support
- **Apple Developer Account**: For device deployment (free account works for testing)

### 2. Build and Deploy

**Using Qt Creator**:
1. Open CMakeLists.txt
2. Select iOS Kit (Qt 6.10 for iOS)
3. Connect iPhone/iPad via USB
4. Build → Run (deploys to device)

**Using Xcode**:
```bash
cmake -B build-ios -GXcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DCMAKE_PREFIX_PATH=~/Qt/6.10.0/ios

open build-ios/uScope.xcodeproj
# In Xcode: Select device → Product → Run
```

---

## Project Structure

```
uScope/
├── CMakeLists.txt              # Build configuration
├── main.cpp                    # Application entry point
├── MainWindow.{h,cpp}          # Main window with tile overlay system (no .ui file)
├── VideoSource.{h,cpp}         # Camera backend
├── VideoGraphicsScene.{h,cpp}  # Video preview with tile overlay support
├── Tile*.{h,cpp}               # Tile-based UI components (resolution-independent)
├── images/                     # SVG/PNG resources
├── sounds/                     # Audio files
├── android/                    # Android-specific files
│   ├── AndroidManifest.xml
│   └── res/                    # Android resources
├── ios/                        # iOS-specific files
│   └── Info.plist.in
├── controllers/                # Business logic (NEW - to be implemented)
├── models/                     # Data models (NEW - to be implemented)
├── services/                   # Service layer (NEW - to be implemented)
├── ui/                         # Feature dialogs (NEW - to be implemented)
├── qml/                        # Qt Quick/QML (mobile UI - future)
├── specs/                      # Feature specifications
│   └── 001-microscopy-platform/
│       ├── spec.md             # Feature specification
│       ├── plan.md             # Implementation plan
│       ├── research.md         # Technology research (includes UI architecture)
│       ├── data-model.md       # Entity definitions
│       ├── quickstart.md       # This file
│       └── contracts/          # API contracts
└── build/                      # Build output (gitignored)
```

---

## Manual Testing Checklist

### User Story 1: Live Camera Preview and Basic Capture (P1 - MVP)

1. **Camera Detection**:
   - [ ] Launch application → Camera auto-detected and listed in dropdown
   - [ ] Live preview starts automatically
   - [ ] Frame rate ≥15 fps (check status bar or console output)

2. **Image Capture**:
   - [ ] Click "Snap Image" button
   - [ ] Image saved to `~/Documents/uScope/image_YYYYMMDD_HHMMSS.png`
   - [ ] Capture completes within 1 second
   - [ ] Live preview continues without interruption

3. **Multiple Cameras**:
   - [ ] Connect second camera (USB webcam, phone camera)
   - [ ] Both cameras listed in dropdown
   - [ ] Switch between cameras → Preview updates

### User Story 2: Manual Camera Controls (P2)

1. **Exposure Control**:
   - [ ] Drag exposure slider → Image brightness changes within 200ms
   - [ ] Verify real-time preview update (no lag)

2. **White Balance**:
   - [ ] Click on white area of specimen
   - [ ] Image color cast corrected

3. **Image Flip**:
   - [ ] Toggle horizontal flip → Preview flips instantly
   - [ ] Toggle vertical flip → Preview flips instantly

### User Story 3: Calibration and Measurement (P3)

1. **Microscope Profile**:
   - [ ] Create microscope profile (name, manufacturer, model)
   - [ ] Add objective "10x" with magnification "10x"
   - [ ] Profile saved (check QSettings or restart app)

2. **Manual Calibration**:
   - [ ] Load stage micrometer image
   - [ ] Select "10x" objective
   - [ ] Draw calibration line on 100µm mark
   - [ ] Enter real-world value: 100
   - [ ] Calibration saved

3. **Measurement**:
   - [ ] Load specimen image
   - [ ] Draw line measurement → Length displayed in µm
   - [ ] Value ≈expected (verify with known specimen size)

4. **Scale Bar Detection** (if Tesseract installed):
   - [ ] Load external microscopy image with scale bar
   - [ ] System auto-detects scale bar on load
   - [ ] OCR text displayed (e.g., "100 µm")
   - [ ] Confidence score shown
   - [ ] Accept or manually correct → Calibration created

### Optional Features (if dependencies installed)

**Object Detection (P5)** - Requires OpenCV:
- [ ] Load calibrated image with spores
- [ ] Click "Detect Objects"
- [ ] Set parameters (min/max area, circularity)
- [ ] Run detection → Objects outlined
- [ ] Manually remove false positive
- [ ] Manually add missed object
- [ ] Export statistics to CSV

**RTSP Streaming (P10)** - Requires GStreamer:
- [ ] Enable "Classroom Mode" on teacher instance
- [ ] Check console for RTSP URL (e.g., `rtsp://192.168.1.100:8554/...`)
- [ ] Open student instance on same network
- [ ] Student sees teacher's stream in camera list
- [ ] Connect to stream → Video appears
- [ ] Teacher draws annotation → Appears on student within 500ms

**iNaturalist (P11)** - Requires network:
- [ ] Authenticate with iNaturalist
- [ ] Search for existing observation
- [ ] Link session to observation
- [ ] Capture image → Marked for upload
- [ ] Click "Push to iNaturalist" → Image uploaded
- [ ] Verify image appears in observation on website

---

## Troubleshooting

### Camera not detected

**Linux**:
```bash
# Check camera permissions
ls -l /dev/video*
# If permission denied:
sudo usermod -a -G video $USER
# Logout and login again

# Test camera with v4l2
v4l2-ctl --list-devices
v4l2-ctl --device=/dev/video0 --all
```

**Windows**:
- Check Device Manager → Cameras → Verify device listed
- Update camera drivers
- Restart application

**macOS**:
- System Preferences → Security & Privacy → Camera → Allow uScope

### Build errors

**Qt not found**:
```bash
# Linux: Set Qt6_DIR
export Qt6_DIR=/usr/lib/cmake/Qt6

# macOS: Set CMAKE_PREFIX_PATH
cmake -B build -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)

# Windows: Use Qt Creator or set CMAKE_PREFIX_PATH to Qt installation
```

**OpenCV not found** (optional dependency):
```bash
# Linux
sudo apt-get install libopencv-dev

# macOS
brew install opencv

# Windows
vcpkg install opencv4:x64-windows
```

### Runtime errors

**Missing Qt plugins**:
```bash
# Linux: Install multimedia plugins
sudo apt-get install libqt6multimedia6-plugins

# Check loaded plugins
QT_DEBUG_PLUGINS=1 ./build/uScope
```

**GStreamer errors** (RTSP streaming):
```bash
# Verify GStreamer installation
gst-inspect-1.0 --version
gst-inspect-1.0 rtspsrc

# Test RTSP stream manually
gst-launch-1.0 rtspsrc location=rtsp://192.168.1.100:8554/test ! fakesink
```

---

## Development Workflow

### Making Changes

1. **Create feature branch**:
   ```bash
   git checkout -b feature/my-improvement
   ```

2. **Make changes** (follow Constitution principles):
   - Use Qt signals/slots for events
   - Follow Qt naming conventions
   - Validate all Qt objects have parent ownership
   - Check DPI-awareness for geometry calculations

3. **Build and test**:
   ```bash
   cmake --build build
   ./build/uScope
   # Manual testing with real camera
   ```

4. **Check for warnings**:
   ```bash
   cmake --build build 2>&1 | grep -i warning
   # Constitution requires zero warnings
   ```

5. **Commit and push**:
   ```bash
   git add .
   git commit -m "Add feature: ..."
   git push origin feature/my-improvement
   ```

### Code Style

- **Qt conventions**: camelCase methods, `_privateMember` for private members
- **Constants**: `const int DEFAULT_EXPOSURE_MS = 100;`, `const int BASE_TILE_DIVISOR = 12;`
- **Signals**: Past tense (`frameReady`, `cameraConnected`)
- **Slots**: Imperative (`startCamera`, `setExposure`)
- **Tile geometry**: Proportional scaling based on window height (base_tile = height/12)
- **Tile anchoring**: Use enum `Tile::Anchor` (Left, Right, Center) for consistent positioning

### Adding New Dependencies

1. Update `CMakeLists.txt`:
   ```cmake
   find_package(NewLibrary REQUIRED)
   target_link_libraries(uScope PRIVATE NewLibrary::NewLibrary)
   ```

2. Update `quickstart.md` (this file) with installation instructions

3. Mark as optional if not required for P1-P4 (core features)

---

## Next Steps

- Read [spec.md](./spec.md) for full feature requirements
- Review [data-model.md](./data-model.md) for entity definitions
- Check [contracts/](./contracts/) for API specifications
- Browse existing code to understand architecture

**Ready to start implementing!** 🚀

---

**Quickstart Status**: ✅ COMPLETE - Setup instructions for all platforms with testing checklist
