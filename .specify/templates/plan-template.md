# Implementation Plan: [FEATURE]

**Branch**: `[###-feature-name]` | **Date**: [DATE] | **Spec**: [link]
**Input**: Feature specification from `/specs/[###-feature-name]/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/commands/plan.md` for the execution workflow.

## Summary

[Extract from feature spec: primary requirement + technical approach from research]

## Technical Context

<!--
  ACTION REQUIRED: Replace the content in this section with the technical details
  for the project. The structure here is presented in advisory capacity to guide
  the iteration process.
-->

**Language/Version**: [e.g., C++17, C++20, Python 3.11, Swift 5.9, Rust 1.75 or NEEDS CLARIFICATION]  
**Primary Dependencies**: [e.g., Qt 6.10 (Widgets, Multimedia, SVG, Quick), FastAPI, UIKit, LLVM or NEEDS CLARIFICATION]  
**Storage**: [if applicable, e.g., QSettings, SQLite, PostgreSQL, CoreData, files or N/A]  
**Testing**: [e.g., Manual verification, Qt Test, pytest, XCTest, cargo test or NEEDS CLARIFICATION]  
**Target Platform**: [e.g., Linux/Windows/macOS desktop, Android 6.0+, iOS 13+, WASM or NEEDS CLARIFICATION]
**Project Type**: [e.g., Qt desktop app, single/web/mobile - determines source structure]  
**Performance Goals**: [domain-specific, e.g., 15+ fps video, 1000 req/s, 10k lines/sec, 60 fps or NEEDS CLARIFICATION]  
**Constraints**: [domain-specific, e.g., <200ms UI response, <100MB memory, offline-capable or NEEDS CLARIFICATION]  
**Scale/Scope**: [domain-specific, e.g., Single user desktop, 10k users, 1M LOC, 50 screens or NEEDS CLARIFICATION]

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify compliance with constitution principles:

- [ ] **Qt Framework Compliance**: Uses Qt signals/slots, follows Qt naming conventions, uses parent-child ownership
- [ ] **Cross-Platform Compatibility**: Uses QStandardPaths, Qt path APIs, validates for different DPI/screen sizes/orientations, touch-first design, handles mobile permissions
- [ ] **Resource Management**: All allocations have clear ownership, resources released properly, validates before use
- [ ] **User Experience First**: Maintains target framerate, UI operations <100ms, provides visual feedback, 44x44pt touch targets on mobile, handles app lifecycle
- [ ] **Code Quality**: Functions are focused, constants are named, APIs documented, verification steps included

[Additional project-specific gates if needed]

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
<!--
  ACTION REQUIRED: Replace the placeholder tree below with the concrete layout
  for this feature. Delete unused options and expand the chosen structure with
  real paths. The delivered plan must not include Option labels.
-->

```text
# [REMOVE IF UNUSED] Option 1: Qt/C++ Desktop Application (DEFAULT for μScope)
<FeatureName>.h            # Header files (class declarations)
<FeatureName>.cpp          # Implementation files
<FeatureName>.ui           # Qt Designer UI files (if applicable)
images/                    # SVG/PNG resources for UI
sounds/                    # Audio resources
resources.qrc              # Qt resource file (if new resources added)
CMakeLists.txt             # Build configuration (update if new files)

tests/ (if applicable)
├── manual/                # Manual test procedures
└── integration/           # Automated integration tests (if needed)

# [REMOVE IF UNUSED] Option 2: Single project (generic - non-Qt)
src/
├── models/
├── services/
├── cli/
└── lib/

tests/
├── contract/
├── integration/
└── unit/

# [REMOVE IF UNUSED] Option 3: Web application (when "frontend" + "backend" detected)
backend/
├── src/
│   ├── models/
│   ├── services/
│   └── api/
└── tests/

frontend/
├── src/
│   ├── components/
│   ├── pages/
│   └── services/
└── tests/

# [REMOVE IF UNUSED] Option 4: Mobile Application (Android/iOS with Qt)
android/                   # Android-specific build files
├── AndroidManifest.xml
├── res/                   # Android resources
└── [gradle files]

ios/                       # iOS-specific build files  
├── Info.plist.in
└── [Xcode project files]

qml/                       # Qt Quick/QML UI (if using Quick for mobile)
├── main.qml
├── components/
└── pages/

shared/                    # Shared C++ code for all platforms
├── *.h                    # Headers
├── *.cpp                  # Implementation
└── resources.qrc          # Qt resources

images/                    # Cross-platform image resources
sounds/                    # Cross-platform audio resources
├── src/
│   ├── models/
│   ├── services/
│   └── api/
└── tests/

frontend/
├── src/
│   ├── components/
│   ├── pages/
│   └── services/
└── tests/

# [REMOVE IF UNUSED] Option 3: Mobile + API (when "iOS/Android" detected)
api/
└── [same as backend above]

ios/ or android/
└── [platform-specific structure: feature modules, UI flows, platform tests]
```

**Structure Decision**: [Document the selected structure and reference the real
directories captured above]

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [e.g., 4th project] | [current need] | [why 3 projects insufficient] |
| [e.g., Repository pattern] | [specific problem] | [why direct DB access insufficient] |
