# μScope - Microscopy Platform

A cross-platform open-source microscopy application for educational institutions, hobbyists, and small laboratories using USB/UVC-compliant cameras.

## Features

- Live camera preview and high-quality image capture
- Manual camera controls (exposure, white balance, brightness, contrast)
- Calibration and measurement tools with scientific accuracy
- Video recording and time-lapse sequences
- Automated object detection and analysis
- Image enhancement filters
- Image stitching for panoramas
- Extended depth of focus (EDF)
- Professional export formats (PDF, OME-TIFF)
- RTSP streaming for classroom collaboration
- iNaturalist integration for species identification

## Build Requirements

### Dependencies

- **Qt 6.10+** with modules: Core, Widgets, Multimedia, Svg
- **OpenCV 4.x** (for camera processing and image analysis)
- **libgphoto2** (for PTP camera support - DSLR/mirrorless cameras)
- **Tesseract OCR 4.x** (optional, for scale bar detection)
- **GStreamer** (optional, for RTSP streaming)
- **CMake 3.21+**
- **C++17** compatible compiler

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt update
sudo apt install -y \
    cmake \
    build-essential \
    qt6-base-dev \
    qt6-multimedia-dev \
    qt6-svg-dev \
    libqt6multimedia6 \
    libopencv-dev \
    libgphoto2-dev \
    tesseract-ocr \
    libtesseract-dev \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-rtsp

# Build
cmake -B build
cmake --build build

# Run
./build/uScope
```

### Linux (Fedora/RHEL)

```bash
# Install dependencies
sudo dnf install -y \
    cmake \
    gcc-c++ \
    qt6-qtbase-devel \
    qt6-qtmultimedia-devel \
    qt6-qtsvg-devel \
    opencv-devel \
    libgphoto2-devel \
    tesseract-devel \
    gstreamer1-plugins-base-devel

# Build
cmake -B build
cmake --build build

# Run
./build/uScope
```

### Windows

1. Install [Qt 6.10+](https://www.qt.io/download-qt-installer) with MSVC 2019 or later
2. Install [CMake](https://cmake.org/download/)
3. Install [vcpkg](https://vcpkg.io/en/getting-started.html)
4. Install dependencies via vcpkg:

```cmd
vcpkg install opencv:x64-windows tesseract:x64-windows libgphoto2:x64-windows
```

5. Build:

```cmd
cmake -B build -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```

### macOS

```bash
# Install dependencies via Homebrew
brew install qt@6 opencv tesseract gstreamer libgphoto2

# Build
cmake -B build -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)
cmake --build build

# Run
./build/uScope.app/Contents/MacOS/uScope
```

## Usage

1. Connect a USB/UVC-compliant microscope camera
2. Launch uScope - the camera should auto-detect and display live preview
3. Use the tile-based UI to access features:
   - **Camera controls**: Exposure, white balance, brightness, contrast
   - **Capture**: Click snap button to save high-resolution images
   - **Calibration**: Load stage micrometer, draw calibration line
   - **Measurement**: Draw lines, circles, polygons on calibrated images
   - **Recording**: Start/stop video recording or time-lapse

## Project Structure

- `main.cpp` - Application entry point
- `MainWindow.h/cpp` - Main window with tile-based UI
- `VideoGraphicsScene.h/cpp` - Video preview rendering
- `Tile*.h/cpp` - Custom tile UI components
- `controllers/` - Business logic controllers
- `models/` - Data models
- `services/` - External service integrations
- `ui/` - Feature-specific UI dialogs

## License

See [LICENSE](LICENSE) file for details.

## Contributing

Contributions welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## Documentation

For detailed specifications and implementation plans, see the `specs/001-microscopy-platform/` directory.

- Real-time camera preview with zoom and pan controls
- Image capture (snapshot) with timestamped filenames
- Video recording to MP4 format
- Camera settings configuration
- Touch-optimized tile interface
- Cross-platform support (Linux, Windows, macOS, Android, iOS)

## Building

### Prerequisites

- Qt 6.10 or later
- CMake 3.16+ (for desktop)
- C++17 compatible compiler

**Additional for Android:**
- Android SDK and NDK
- Qt for Android

**Additional for iOS:**
- Xcode
- Qt for iOS
- Apple Developer account (for device deployment)

### Build Instructions

#### Desktop (Linux/Windows/macOS)

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Run
./build/uScope
```

#### Android

Use Qt Creator with Android kit configured, or:

```bash
cd android
./gradlew assembleRelease
```

#### iOS

Use Qt Creator with iOS kit configured, or open the Xcode project in `ios/` directory.

## Project Governance

This project follows principles defined in `.specify/memory/constitution.md`:

- **Qt Framework Compliance**: Uses Qt best practices and idioms
- **Cross-Platform Compatibility**: Runs on Linux, Windows, macOS, Android, and iOS
- **Resource Management**: Explicit memory and resource lifecycle management
- **User Experience First**: Real-time responsiveness, touch-optimized UI, and visual feedback
- **Code Quality**: Maintainable, clear, and well-documented code

See the constitution for detailed development guidelines.

## CI/CD and GitHub Actions

This project uses GitHub Actions for automated builds and releases across all platforms.

### Required GitHub Secrets

For automated builds to work, configure the following secrets in your repository settings:

#### Android Build Secrets (Optional)
- `ANDROID_KEYSTORE_BASE64` - Base64-encoded keystore file for signing APKs
- `ANDROID_KEYSTORE_PASSWORD` - Keystore password
- `ANDROID_KEY_ALIAS` - Key alias in the keystore
- `ANDROID_KEY_PASSWORD` - Key password

#### iOS Build Secrets (Optional)
- `IOS_CERTIFICATE_BASE64` - Base64-encoded developer certificate (.p12)
- `IOS_CERTIFICATE_PASSWORD` - Certificate password
- `IOS_PROVISIONING_PROFILE_BASE64` - Base64-encoded provisioning profile
- `APPLE_ID` - Apple ID for app notarization
- `APPLE_APP_SPECIFIC_PASSWORD` - App-specific password for notarization

**Note:** The workflows will create unsigned builds if these secrets are not configured. Signed builds are only required for distribution to users.

### Workflow Overview

- **Pull Requests**: Builds all platforms (Linux DEB/RPM, Windows, macOS, Android, iOS)
- **Tagged Releases**: Builds all platforms and creates GitHub Release with downloadable artifacts
- **Caching**: Qt installations and build dependencies are cached to speed up builds

To create a release:
```bash
git tag -a v0.1.0 -m "Initial release"
git push origin v0.1.0
```

## License

MIT License - see LICENSE file for details.