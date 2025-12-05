<!--
Sync Impact Report - Constitution v1.0.0
========================================
Version change: [TEMPLATE] → 1.0.0
Initial ratification: 2025-12-05

Principles defined:
- I. Qt Framework Compliance
- II. Cross-Platform Compatibility (5 platforms: Linux, Windows, macOS, Android, iOS)
- III. Resource Management
- IV. User Experience First
- V. Code Quality & Maintainability

Sections added:
- Technical Stack (desktop + mobile)
- Development Workflow (desktop + mobile)

Templates requiring updates:
✅ plan-template.md - Constitution Check section aligned, mobile structure added
✅ spec-template.md - Requirements structure compatible
✅ tasks-template.md - Task categorization supports Qt/C++ workflow
✅ README.md - Updated with all platform support

Follow-up TODOs: None
-->

# μScope Constitution

## Core Principles

### I. Qt Framework Compliance

All features MUST use Qt framework best practices and idioms:
- Use Qt signals/slots for event communication, never raw callbacks
- Follow Qt naming conventions (camelCase for methods, _prefixedCamelCase for private members)
- Utilize Qt's memory management (parent-child ownership) to prevent leaks
- Leverage Qt's platform abstraction layers (QFile, QDir, etc.) instead of native APIs
- Use Qt's type system (QString, QList, etc.) consistently throughout the codebase

**Rationale**: Qt provides robust cross-platform abstractions and memory safety. Deviating introduces platform-specific bugs and memory leaks.

### II. Cross-Platform Compatibility

Code MUST run on Linux, Windows, macOS, Android, and iOS with minimal platform-specific code:
- Use QStandardPaths for system directories, never hardcoded paths
- Test all geometry calculations with different DPI settings, screen sizes, and orientations
- Design touch-first interfaces that work with mouse/keyboard (not the reverse)
- Avoid platform-specific APIs unless absolutely necessary (then use conditional compilation with clear fallbacks)
- Ensure file paths use QDir::separator() or Qt path APIs
- Validate integer arithmetic to prevent overflow on different architectures
- Request permissions appropriately on mobile (camera, storage, microphone)
- Optimize for battery efficiency on mobile platforms (minimize background processing)

**Rationale**: μScope targets desktop and mobile platforms. Platform-specific code creates maintenance burden and user experience fragmentation. Mobile platforms require additional consideration for touch input, permissions, and power consumption.

### III. Resource Management

Memory and resource lifecycle MUST be explicitly managed:
- All dynamically allocated Qt objects MUST have a parent or explicit deletion
- Camera and video resources MUST be released in closeEvent() or object destructors
- Image buffers MUST be validated before use (check isNull() before QPainter operations)
- Check calculated values (font sizes, geometry dimensions) for validity before assignment
- Prevent integer overflow in arithmetic operations (especially geometry calculations)

**Rationale**: Resource leaks and invalid operations cause crashes and poor user experience, particularly on resource-constrained systems.

### IV. User Experience First

Every feature MUST prioritize real-time responsiveness and visual feedback:
- Video frame processing MUST maintain target framerate (minimum 15 fps)
- UI operations MUST complete within 100ms or show progress indication
- Tile/button interactions MUST provide immediate visual feedback
- Camera settings changes MUST reflect in preview within 200ms
- Touch targets MUST be minimum 44x44 points on mobile devices
- Error conditions MUST be communicated clearly to users (not just console logs)
- App MUST handle mobile lifecycle events (background/foreground, interruptions)

**Rationale**: μScope is a real-time microscopy tool. Delays or unresponsive UI make the application unusable for precision work. Mobile platforms require larger touch targets and proper lifecycle management.

### V. Code Quality & Maintainability

Code MUST be clear, testable, and maintainable:
- Functions MUST have single, clear responsibilities (max 50 lines ideal)
- Magic numbers MUST be named constants with clear units/purpose
- Complex algorithms MUST have explanatory comments
- Public APIs MUST have header documentation
- Changes MUST include verification steps (manual testing or automated tests)

**Rationale**: Single-developer projects require extra discipline to remain maintainable as features accumulate.

## Technical Stack

**Language**: C++17 or later  
**Framework**: Qt 6.10+ (using Qt6 Multimedia, Widgets, SVG modules, Quick for mobile)  
**Build System**: CMake 3.16+ (desktop), Gradle/qmake (Android), Xcode (iOS)  
**Target Platforms**: Linux (primary), Windows, macOS, Android, iOS  
**Video Backend**: Qt Multimedia (QCamera, QMediaRecorder)  
**Image Processing**: Qt built-in (QPainter, QImage, QPixmap)  
**Optional**: OpenCV for advanced processing (Android SDK present in repo)

**Platform-Specific Notes**:
- Linux: Native Qt6, GCC/Clang
- Windows: MSVC or MinGW with Qt6
- macOS: Clang with Qt6 frameworks
- Android: Qt for Android 6.0+, OpenCV SDK available, requires Android SDK/NDK
- iOS: Qt for iOS 13+, requires Xcode and Apple Developer account for device deployment

## Development Workflow

**Build Process**:

Desktop (Linux/Windows/macOS):
1. Configure with CMake: `cmake -B build -DCMAKE_BUILD_TYPE=Release`
2. Build: `cmake --build build`
3. Test on target platform before committing

Mobile:
- Android: Use Qt Creator with Android kit or `gradlew` in android/ directory
- iOS: Use Qt Creator with iOS kit or Xcode project
- Test on actual devices when possible (emulators have camera limitations)

**Code Changes**:
- Test with different window sizes, DPI settings, and orientations (portrait/landscape)
- Verify camera functionality with actual hardware when possible
- Check console output for Qt warnings (all warnings MUST be addressed)
- Validate geometry calculations don't overflow or produce negative values
- Test mobile permissions flow (camera, storage, microphone access)
- Verify mobile lifecycle events (backgrounding, interruptions) handled properly

**Quality Gates**:
- Code MUST compile without warnings on primary platform
- Qt object ownership MUST be verifiable (no orphaned pointers)
- UI interactions MUST be manually tested
- Platform compatibility MUST be considered (document platform-specific code)

## Governance

This constitution supersedes all other development practices. Changes to core principles require:
1. Documentation of rationale and impact
2. Review of affected code and templates
3. Update of this constitution with incremented version
4. Propagation of changes to dependent templates

**Amendment Policy**:
- MAJOR version: Principle removal or incompatible governance change
- MINOR version: New principle added or significant expansion
- PATCH version: Clarifications, typo fixes, non-semantic improvements

**Compliance**:
- All code reviews MUST verify adherence to core principles
- Constitution violations MUST be documented and justified in commit messages
- Template updates MUST follow constitution changes to maintain consistency

**Version**: 1.0.0 | **Ratified**: 2025-12-05 | **Last Amended**: 2025-12-05
