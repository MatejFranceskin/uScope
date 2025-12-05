# μScope

A cross-platform microscope camera application built with Qt 6 for desktop and mobile devices.

## Overview

μScope is a Qt-based application for viewing, capturing, and recording video from microscope cameras. It provides a tile-based interface for camera control, settings management, and media capture.

## Features

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

## License

MIT License - see LICENSE file for details.