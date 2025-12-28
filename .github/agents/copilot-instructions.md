# uScope Development Guidelines

Auto-generated from all feature plans. Last updated: 2025-12-05

## Active Technologies

- C++17 or later + Qt 6.10+ (Multimedia, Widgets, SVG, Quick for mobile), OpenCV 4.x (image stitching, EDF, object detection), Tesseract OCR 4.x (scale bar text recognition), FFmpeg/libx264 (H.264 video encoding via Qt Multimedia), GStreamer + gst-rtsp-server (RTSP streaming) (001-microscopy-platform)

## UI Architecture

- **Desktop**: Custom tile-based overlay system with QGraphicsView/QGraphicsScene - video background with transparent tile widgets, resolution-independent proportional scaling (base tile = window_height/12)
- **Mobile**: Qt Quick/QML with shared C++ backend (controllers, models, services)
- **Tiles**: Square/rectangle widgets anchored left/right/center, state-based colors (Idle/Hover/Active/Disabled), semi-transparent backgrounds

## Project Structure

```text
src/
tests/
```

## Commands

# Add commands for C++17 or later

## Code Style

C++17 or later: Follow standard conventions
- Qt naming: camelCase methods, `_privateMember` for private members
- Signals: Past tense (`frameReady`, `cameraConnected`)
- Slots: Imperative (`startCamera`, `setExposure`)
- Constants: `const int BASE_TILE_DIVISOR = 12;`
- Tile geometry: Proportional scaling based on window height
- All Qt objects must have parent ownership (prevent memory leaks)

## Recent Changes

- 001-microscopy-platform: Added tile-based overlay UI architecture with resolution-independent scaling
- 001-microscopy-platform: Added GStreamer for RTSP streaming support
- 001-microscopy-platform: Updated to use QGraphicsView/Scene instead of traditional Qt Designer .ui files
- 001-microscopy-platform: Added C++17 or later + Qt 6.10+ (Multimedia, Widgets, SVG, Quick for mobile), OpenCV 4.x (image stitching, EDF, object detection), Tesseract OCR 4.x (scale bar text recognition), FFmpeg/libx264 (H.264 video encoding via Qt Multimedia)

<!-- MANUAL ADDITIONS START -->
<!-- MANUAL ADDITIONS END -->
