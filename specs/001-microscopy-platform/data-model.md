# Data Model: μScope Microscopy Platform

**Branch**: `001-microscopy-platform` | **Date**: 2025-12-05

## Purpose

This document defines all entities, their attributes, relationships, validation rules, and state transitions for the μScope microscopy platform.

## Entity Definitions

### 1. Camera Profile

**Purpose**: Represents a connected camera device with its capabilities

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `deviceId` | QString | Yes | Non-empty, unique | Qt camera device identifier |
| `name` | QString | Yes | Non-empty | User-friendly camera name (e.g., "USB Camera") |
| `description` | QString | No | - | Camera description from driver |
| `vendor` | QString | No | - | Manufacturer name |
| `model` | QString | No | - | Camera model identifier |
| `supportedResolutions` | QList<QSize> | Yes | Non-empty | Available resolutions (e.g., 1920x1080, 1280x720) |
| `supportedFrameRates` | QList<int> | Yes | Non-empty, > 0 | Available frame rates in fps |
| `supportedPixelFormats` | QList<QVideoFrameFormat::PixelFormat> | Yes | Non-empty | Supported pixel formats |
| `currentFormat` | QCameraFormat | Yes | Must be in supported formats | Active camera format |
| `isAvailable` | bool | Yes | - | Camera connection status |

**Relationships**:
- 1 Camera Profile → N Captured Images (one camera can capture many images)
- 1 Camera Profile → 1 Microscope Profile (optional association)

**State Transitions**:
```
[Disconnected] ──connect──> [Available] ──start──> [Active] ──capture──> [Capturing]
                                ↑            │
                                │            │
                                └──stop──────┘
[Active] ──disconnect──> [Disconnected]
```

**Storage**: Transient (runtime only), repopulated on app launch via `QMediaDevices::videoInputs()`

---

### 2. Microscope Profile

**Purpose**: Represents a physical microscope with its objectives and associated camera

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `id` | int | Yes | Auto-increment | Unique identifier |
| `name` | QString | Yes | Non-empty, max 100 chars | User-defined microscope name |
| `manufacturer` | QString | No | Max 100 chars | Manufacturer (e.g., "Olympus") |
| `model` | QString | No | Max 100 chars | Model identifier |
| `associatedCameraId` | QString | No | Must match existing Camera Profile deviceId | Currently associated camera |
| `objectives` | QList<Objective> | No | - | List of objectives for this microscope |
| `createdAt` | QDateTime | Yes | Valid date | Creation timestamp |
| `updatedAt` | QDateTime | Yes | Valid date | Last modification timestamp |

**Relationships**:
- 1 Microscope Profile → N Objectives (one microscope has many objectives)
- 1 Microscope Profile → 1 Camera Profile (optional association)
- 1 Microscope Profile → N Calibrations (via objectives)

**State Transitions**: N/A (persistent entity, no workflow states)

**Storage**: QSettings (persistent across sessions)
- Key: `microscopes/{id}/name`, `microscopes/{id}/manufacturer`, etc.
- List: `microscopes/count`, `microscopes/ids`

---

### 3. Objective

**Purpose**: Represents a microscope objective lens with magnification

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `id` | int | Yes | Auto-increment | Unique identifier within microscope |
| `microscopeId` | int | Yes | Must match existing Microscope Profile id | Parent microscope |
| `name` | QString | Yes | Non-empty, max 50 chars | User-defined name (e.g., "Plan 10x", "10x") |
| `magnification` | QString | Yes | Non-empty, format: "\d+x" (e.g., "10x", "40x") | Magnification value |
| `hasCalibration` | bool | Yes | - | Whether calibration exists for this objective |
| `createdAt` | QDateTime | Yes | Valid date | Creation timestamp |

**Relationships**:
- 1 Objective → 1 Microscope Profile (parent relationship)
- 1 Objective → 1 Calibration (optional, one-to-one)

**State Transitions**: N/A (persistent entity)

**Storage**: QSettings under parent microscope
- Key: `microscopes/{microscopeId}/objectives/{id}/name`, etc.

---

### 4. Calibration

**Purpose**: Pixel-to-micrometer scale factor for a specific microscope/objective/camera combination

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `id` | int | Yes | Auto-increment | Unique identifier |
| `microscopeId` | int | Yes | Must match existing Microscope Profile | Associated microscope |
| `objectiveId` | int | Yes | Must match existing Objective | Associated objective |
| `cameraId` | QString | Yes | Must match Camera Profile deviceId | Associated camera |
| `pixelsPerMicrometer` | double | Yes | > 0, typically 0.1 to 100 | Scale factor (µm/pixel) |
| `referenceImagePath` | QString | No | Valid file path | Stage micrometer image used for calibration |
| `referenceLinePixels` | double | No | > 0 | Length of calibration line in pixels |
| `referenceLineMicrometers` | double | No | > 0 | Real-world length of calibration line in µm |
| `createdAt` | QDateTime | Yes | Valid date | Calibration timestamp |
| `source` | QString | Yes | "manual" or "auto_scale_bar" | How calibration was created |

**Relationships**:
- 1 Calibration → 1 Microscope Profile
- 1 Calibration → 1 Objective
- 1 Calibration → 1 Camera Profile

**Validation Rules**:
- `pixelsPerMicrometer = referenceLinePixels / referenceLineMicrometers`
- Combination of (microscopeId, objectiveId, cameraId) must be unique
- If `source == "auto_scale_bar"`, `referenceImagePath` is external image with detected scale bar

**Storage**: QSettings
- Key: `calibrations/{id}/microscopeId`, `calibrations/{id}/pixelsPerMicrometer`, etc.

---

### 5. Detected Scale Bar

**Purpose**: Automatically detected scale bar in external microscopy image

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `boundingBox` | QRect | Yes | Valid rectangle, width/height > 0 | Region containing scale bar |
| `lengthPixels` | double | Yes | > 0 | Scale bar length in pixels |
| `ocrText` | QString | Yes | Non-empty | Raw OCR output |
| `parsedValue` | double | Yes | > 0 | Extracted numeric value (e.g., 100 from "100 µm") |
| `parsedUnit` | QString | Yes | One of: "µm", "um", "μm", "mm", "nm", "Å" | Extracted unit |
| `confidenceScore` | double | Yes | 0.0 to 1.0 | Detection confidence (OCR + pattern matching) |
| `isValidated` | bool | Yes | - | User confirmed/corrected detection |
| `userCorrectedValue` | double | No | > 0 if present | User-provided value if OCR misread |
| `userCorrectedUnit` | QString | No | Valid unit if present | User-provided unit if OCR misread |

**Relationships**: Transient (used during calibration workflow, not persisted)

**State Transitions**:
```
[Detected] ──user reviews──> [Validated] ──accept──> [Calibration Created]
                                   │
                                   └──user corrects──> [Corrected] ──accept──> [Calibration Created]
[Detected] ──confidence < 70%──> [Manual Fallback]
```

**Validation Rules**:
- If `confidenceScore < 0.7`, recommend manual calibration (FR-017.6)
- `parsedUnit` must convert to micrometers: mm=1000µm, nm=0.001µm, Å=0.0001µm

**Storage**: Transient (displayed in UI, then converted to Calibration if accepted)

---

### 6. Captured Image

**Purpose**: Still image captured from camera with embedded metadata

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `filePath` | QString | Yes | Valid file path, writable directory | Full path to image file |
| `filename` | QString | Yes | Format: "image_yyyyMMdd_HHmmss.{png\|jpg\|tiff}" | Auto-generated filename |
| `format` | QString | Yes | One of: "JPEG", "PNG", "TIFF" | Image format |
| `resolution` | QSize | Yes | Width/height > 0 | Image dimensions |
| `timestamp` | QDateTime | Yes | Valid date | Capture timestamp |
| `cameraId` | QString | Yes | Must match Camera Profile | Camera used for capture |
| `exposure` | double | No | ≥ 0 | Exposure time in ms |
| `gain` | int | No | ≥ 0 | Sensor gain/ISO |
| `whiteBalance` | QString | No | - | White balance mode |
| `microscopeId` | int | No | Must match Microscope Profile | Associated microscope |
| `objectiveId` | int | No | Must match Objective | Active objective during capture |
| `calibrationId` | int | No | Must match Calibration | Active calibration |
| `pixelsPerMicrometer` | double | No | > 0 | Scale from calibration (duplicated for quick access) |
| `annotations` | QList<Annotation> | No | - | Annotation layer data |
| `measurements` | QList<Measurement> | No | - | Measurement objects |

**Relationships**:
- 1 Captured Image → 1 Camera Profile
- 1 Captured Image → 1 Microscope Profile (optional)
- 1 Captured Image → 1 Objective (optional)
- 1 Captured Image → 1 Calibration (optional)
- 1 Captured Image → N Annotations
- 1 Captured Image → N Measurements

**Metadata Embedding**:
- **JPEG/TIFF**: EXIF tags (timestamp, camera, exposure, gain), custom EXIF UserComment (JSON: microscope, objective, calibration)
- **PNG**: Text chunks (key: "uScope.Metadata", value: JSON with all metadata)
- **All formats**: Annotation layer serialized as JSON array in custom field

**Validation Rules**:
- If `microscopeId` and `objectiveId` set, `calibrationId` should be valid calibration for that combination
- `annotations` and `measurements` reconstruct from metadata on load (FR-047.4)

**Storage**: Files in `~/Documents/uScope/` (or platform equivalent), metadata embedded in files

---

### 7. Measurement

**Purpose**: Single geometric measurement on an image

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `id` | int | Yes | Auto-increment | Unique identifier |
| `type` | QString | Yes | One of: "line", "angle", "circle", "rectangle", "polygon" | Measurement type |
| `points` | QList<QPointF> | Yes | Non-empty, ≥2 points | Defining points (pixels) |
| `value` | double | Yes | > 0 | Calculated value (length, area, angle) |
| `unit` | QString | Yes | One of: "µm", "mm", "cm", "in", "px", "°" (angle) | Measurement unit |
| `label` | QString | No | Max 100 chars | User-defined label |
| `datasetName` | QString | No | Max 50 chars | Dataset this measurement belongs to |
| `calibrationPixelsPerMicrometer` | double | No | > 0 if unit ≠ "px" | Scale used (for recalculation) |
| `createdAt` | QDateTime | Yes | Valid date | Creation timestamp |

**Relationships**:
- N Measurements → 1 Captured Image (measurements on an image)
- N Measurements → 1 Measurement Dataset (grouped measurements)

**Validation Rules**:
- If `unit ≠ "px"`, `calibrationPixelsPerMicrometer` must be set
- `type == "line"`: 2 points, `value` = Euclidean distance
- `type == "angle"`: 3 points, `value` = angle in degrees
- `type == "circle"`: 2 points (center, perimeter), `value` = diameter, radius, area
- `type == "rectangle"`: 4 points (corners), `value` = area, length, width
- `type == "polygon"`: ≥3 points, `value` = area, perimeter

**Calculation Formulas**:
- Line length: `sqrt((x2-x1)² + (y2-y1)²) / pixelsPerMicrometer`
- Circle area: `π × radius²`, diameter: `2 × radius`
- Rectangle area: `length × width`
- Polygon area: Shoelace formula

**Storage**: Embedded in Captured Image metadata, also in QSettings for datasets

---

### 8. Measurement Dataset

**Purpose**: Named collection of measurements for statistical analysis

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `name` | QString | Yes | Non-empty, unique, max 50 chars | Dataset name |
| `measurements` | QList<Measurement> | Yes | Non-empty | Measurements in dataset |
| `statistics` | MeasurementStatistics | Yes | Computed from measurements | Aggregated stats |
| `createdAt` | QDateTime | Yes | Valid date | Creation timestamp |
| `updatedAt` | QDateTime | Yes | Valid date | Last modification |

**Nested Structure - MeasurementStatistics**:
| Attribute | Type | Description |
|-----------|------|-------------|
| `count` | int | Number of measurements |
| `mean` | double | Average value |
| `standardDeviation` | double | Std dev |
| `minimum` | double | Min value |
| `maximum` | double | Max value |
| `median` | double | Median value |
| `confidenceInterval95` | QPair<double, double> | 95% CI (lower, upper) |

**Relationships**:
- 1 Measurement Dataset → N Measurements

**Validation Rules**:
- All measurements in dataset must have same `unit`
- Statistics recalculated on dataset modification

**Storage**: QSettings (`datasets/{name}/measurements`, `datasets/{name}/statistics`)

---

### 9. Detected Object Set

**Purpose**: Collection of automatically detected objects from object detection algorithm

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `imageId` | QString | Yes | Must match Captured Image filePath | Source image |
| `algorithm` | QString | Yes | "contour_detection" (default) | Detection algorithm used |
| `parameters` | DetectionParameters | Yes | Valid parameter set | Algorithm configuration |
| `objects` | QList<DetectedObject> | Yes | - | Detected objects |
| `createdAt` | QDateTime | Yes | Valid date | Detection timestamp |

**Nested Structure - DetectionParameters**:
| Attribute | Type | Validation | Description |
|-----------|------|------------|-------------|
| `minArea` | double | > 0 | Minimum object area (pixels²) |
| `maxArea` | double | > minArea | Maximum object area (pixels²) |
| `minCircularity` | double | 0.0 to 1.0 | Minimum circularity (4π×area/perimeter²) |
| `contrastThreshold` | int | 0 to 255 | Otsu/adaptive threshold value |
| `morphologyKernelSize` | int | Odd, > 0 | Morphological operation kernel size |
| `gaussianBlurSize` | int | Odd, > 0 | Gaussian blur kernel size |

**Relationships**:
- 1 Detected Object Set → 1 Captured Image
- 1 Detected Object Set → N Detected Objects

**Storage**: Embedded in Captured Image metadata (if saved), also transient for UI editing

---

### 10. Detected Object

**Purpose**: Individual object identified by detection algorithm

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `id` | int | Yes | Auto-increment | Unique identifier within set |
| `contour` | QPolygonF | Yes | ≥3 points | Object boundary points |
| `boundingBox` | QRectF | Yes | Valid rectangle | Bounding box |
| `centroid` | QPointF | Yes | - | Object center (mean of contour points) |
| `area` | double | Yes | > 0 | Area (pixels²) |
| `perimeter` | double | Yes | > 0 | Perimeter (pixels) |
| `circularity` | double | Yes | 0.0 to 1.0 | 4π×area/perimeter² |
| `aspectRatio` | double | Yes | > 0 | Width/height of bounding box |
| `minorAxis` | double | Yes | > 0 | Fitted ellipse minor axis |
| `majorAxis` | double | Yes | > 0 | Fitted ellipse major axis |
| `detectionConfidence` | double | Yes | 0.0 to 1.0 | Confidence score |
| `userValidated` | bool | Yes | - | User confirmed (not false positive) |
| `isManuallyAdded` | bool | Yes | - | User added (not auto-detected) |
| `measurements` | ObjectMeasurements | Yes | Computed from geometry | Calibrated measurements |

**Nested Structure - ObjectMeasurements**:
| Attribute | Type | Description |
|-----------|------|-------------|
| `lengthMicrometers` | double | Major axis in µm |
| `widthMicrometers` | double | Minor axis in µm |
| `areaMicrometers` | double | Area in µm² |
| `perimeterMicrometers` | double | Perimeter in µm |

**Relationships**:
- 1 Detected Object → 1 Detected Object Set (parent)

**Validation Rules**:
- `circularity = 4π × area / perimeter²` (1.0 = perfect circle)
- `aspectRatio = boundingBox.width / boundingBox.height`
- If `isManuallyAdded == true`, `detectionConfidence = 1.0`

**State Transitions**:
```
[Auto-Detected] ──user reviews──> [Validated] ──user accepts──> [Confirmed]
                                         │
                                         └──user removes──> [Removed]
[Not Detected] ──user adds manually──> [Manually Added] ──> [Confirmed]
```

**Storage**: Part of Detected Object Set, embedded in image metadata or CSV export

---

### 11. Annotation

**Purpose**: Non-destructive visual overlay (text, arrows, shapes) on image

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `id` | int | Yes | Auto-increment | Unique identifier |
| `type` | QString | Yes | One of: "text", "arrow", "line", "rectangle", "circle", "polygon" | Annotation type |
| `position` | QPointF | Yes | - | Anchor point (pixels) |
| `points` | QList<QPointF> | No | Required for multi-point types | Defining points for shapes |
| `text` | QString | No | Required if type == "text", max 500 chars | Text content |
| `fontSize` | int | No | > 0, default 12 | Font size (points) |
| `fontFamily` | QString | No | Default "Arial" | Font family |
| `color` | QColor | Yes | Valid color | Annotation color (main line/text) |
| `lineWidth` | int | No | > 0, default 2 | Line/border width (pixels) |
| `fillColor` | QColor | No | Valid color, alpha for transparency | Fill color for shapes |
| `outlineWidth` | int | No | > 0, default 5 | Outline width for visibility on any background |
| `outlineOpacity` | qreal | No | 0.0-1.0, default 0.6 | Outline transparency (~60% for contrast) |
| `zOrder` | int | Yes | ≥ 0 | Z-order for layering |
| `burnIn` | bool | Yes | Default false | Whether to burn into exported image |
| `createdAt` | QDateTime | Yes | Valid date | Creation timestamp |

**Relationships**:
- N Annotations → 1 Captured Image

**Validation Rules**:
- If `type == "text"`, `text` must be non-empty
- If `type == "arrow" || "line"`, `points` must have 2 elements
- If `type == "rectangle" || "polygon"`, `points` must have ≥3 elements

**Rendering Style** (see contracts/annotation-rendering.md):
- All annotations use outlined style: semi-transparent outline (black/white, ~60% opacity) + bright main color
- Antialiasing enabled for smooth edges
- Text rendered via QPainterPath for outline support
- Ensures visibility on any background (dark to light)

**Storage**: Embedded in Captured Image metadata (JSON array)

---

### 12. Video Recording

**Purpose**: Recorded video file with embedded metadata

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `filePath` | QString | Yes | Valid file path, writable directory | Full path to video file |
| `filename` | QString | Yes | Format: "video_yyyyMMdd_HHmmss.mp4" | Auto-generated filename |
| `format` | QString | Yes | "MP4" or "AVI" | Video container format |
| `codec` | QString | Yes | "H.264" | Video codec (FR-006.1) |
| `resolution` | QSize | Yes | Width/height > 0 | Video dimensions |
| `frameRate` | int | Yes | > 0 | Frames per second |
| `duration` | int | Yes | ≥ 0 | Duration in seconds |
| `startTimestamp` | QDateTime | Yes | Valid date | Recording start time |
| `endTimestamp` | QDateTime | Yes | Valid date, ≥ startTimestamp | Recording end time |
| `cameraId` | QString | Yes | Must match Camera Profile | Camera used |
| `microscopeId` | int | No | Must match Microscope Profile | Associated microscope |
| `objectiveId` | int | No | Must match Objective | Active objective during recording |
| `calibrationId` | int | No | Must match Calibration | Active calibration |
| `pixelsPerMicrometer` | double | No | > 0 | Scale from calibration |
| `isTimeLapse` | bool | Yes | - | Whether time-lapse or continuous |
| `timeLapseInterval` | int | No | > 0 if isTimeLapse | Interval in seconds |

**Relationships**:
- 1 Video Recording → 1 Camera Profile
- 1 Video Recording → 1 Microscope Profile (optional)
- 1 Video Recording → 1 Objective (optional)
- 1 Video Recording → 1 Calibration (optional)

**Metadata Embedding**:
- MP4: Metadata atoms (udta/meta boxes) with JSON payload
- AVI: INFO chunk with JSON payload
- Includes: microscope, objective, calibration, camera settings

**Storage**: Files in `~/Documents/uScope/`, metadata embedded in container

---

### 13. Focus Stack

**Purpose**: Collection of images at different focus depths for EDF processing

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `id` | int | Yes | Auto-increment | Unique identifier |
| `name` | QString | Yes | Non-empty, max 100 chars | User-defined stack name |
| `frames` | QList<QString> | Yes | ≥2 image paths, all must exist | Paths to images in stack |
| `zPositions` | QList<double> | No | Same length as frames | Z-axis positions (µm, if motorized) |
| `createdAt` | QDateTime | Yes | Valid date | Stack creation timestamp |
| `edfResultPath` | QString | No | Valid file path | Path to generated EDF composite |

**Relationships**:
- 1 Focus Stack → N Captured Images (stack frames)

**Validation Rules**:
- All frames must have same resolution
- If `zPositions` provided, length must match `frames`

**Storage**: QSettings (`focusStacks/{id}/name`, `focusStacks/{id}/frames`)

---

### 14. Stitched Panorama

**Purpose**: Composite image from multiple overlapping frames

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `id` | int | Yes | Auto-increment | Unique identifier |
| `name` | QString | Yes | Non-empty, max 100 chars | User-defined panorama name |
| `sourceFrames` | QList<QString> | Yes | ≥2 image paths, all must exist | Paths to source images |
| `gridSize` | QSize | Yes | Width/height > 0 | Grid dimensions (e.g., 3x3) |
| `overlapPercent` | int | Yes | 10 to 90 | Overlap percentage between frames |
| `resultPath` | QString | No | Valid file path | Path to stitched panorama |
| `createdAt` | QDateTime | Yes | Valid date | Stitching timestamp |
| `alignmentMetadata` | QString | No | JSON | OpenCV stitcher alignment data |

**Relationships**:
- 1 Stitched Panorama → N Captured Images (source frames)

**Validation Rules**:
- `sourceFrames.length == gridSize.width × gridSize.height`
- All frames must have same resolution and calibration

**Storage**: QSettings (`panoramas/{id}/name`, `panoramas/{id}/sourceFrames`)

---

### 15. RTSP Streaming Session

**Purpose**: Classroom collaboration session for teacher streaming to students

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `sessionId` | QString | Yes | UUID | Unique session identifier |
| `teacherName` | QString | Yes | Non-empty, max 50 chars | Teacher identifier |
| `teacherDeviceId` | QString | Yes | Non-empty | Device ID (MAC address or UUID) |
| `rtspUrl` | QString | Yes | Valid RTSP URL (rtsp://IP:PORT/stream) | Stream URL |
| `resolution` | QSize | Yes | Default 1280x720 (FR-063.1) | Stream resolution |
| `frameRate` | int | Yes | Default 30 fps | Stream frame rate |
| `bitrate` | int | Yes | 3000-5000 kbps | Target bitrate |
| `authToken` | QString | No | UUID if auth enabled | Optional authentication token |
| `connectedStudents` | QList<ConnectedStudent> | Yes | - | Currently connected students |
| `maxStudents` | int | Yes | Default 30 (FR-071) | Maximum concurrent connections |
| `isActive` | bool | Yes | - | Whether stream is running |
| `startedAt` | QDateTime | No | Valid date if isActive | Session start time |

**Relationships**:
- 1 RTSP Streaming Session → N Connected Students

**Validation Rules**:
- `connectedStudents.length ≤ maxStudents`
- `rtspUrl` format: `rtsp://{IP}:{port}/{sessionId}`

**State Transitions**:
```
[Inactive] ──teacher enables classroom mode──> [Starting] ──server ready──> [Active] ──students connect──> [Streaming]
                                                                                ↑            │
                                                                                │            │
                                                                                └──last student disconnects──┘
[Active] ──teacher disables classroom mode──> [Stopping] ──> [Inactive]
```

**Storage**: Transient (runtime only, session ends on app close)

---

### 16. Connected Student

**Purpose**: Student client connected to teacher's RTSP stream

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `deviceId` | QString | Yes | Non-empty | Student device identifier |
| `deviceName` | QString | Yes | Non-empty, max 50 chars | Student device name |
| `ipAddress` | QString | Yes | Valid IP address | Student IP |
| `connectedAt` | QDateTime | Yes | Valid date | Connection timestamp |
| `latency` | int | Yes | ≥ 0 | Current latency in milliseconds |
| `bandwidth` | double | Yes | ≥ 0 | Current bandwidth usage (Mbps) |
| `connectionQuality` | QString | Yes | One of: "excellent", "good", "fair", "poor" | Quality indicator |
| `permissions` | QStringList | Yes | - | Granted permissions (e.g., "download_images") |

**Relationships**:
- N Connected Students → 1 RTSP Streaming Session

**Validation Rules**:
- `connectionQuality` based on latency: <100ms=excellent, <300ms=good, <500ms=fair, ≥500ms=poor

**Storage**: Transient (part of RTSP Streaming Session)

---

### 17. iNaturalist Session

**Purpose**: Link between imaging session and iNaturalist observation

**Attributes**:
| Attribute | Type | Required | Validation | Description |
|-----------|------|----------|------------|-------------|
| `observationId` | int | Yes | > 0 | iNaturalist observation ID |
| `observationUrl` | QString | Yes | Valid URL | iNaturalist observation URL |
| `accessToken` | QString | Yes | Non-empty | OAuth access token |
| `tokenExpiration` | QDateTime | Yes | Valid date | Token expiration timestamp |
| `uploadQueue` | QList<QString> | Yes | Valid file paths | Images queued for upload |
| `fieldMappings` | QMap<QString, QString> | No | - | Dataset → observation field mappings |
| `linkedAt` | QDateTime | Yes | Valid date | When session was linked |
| `lastSyncAt` | QDateTime | No | Valid date | Last successful sync |

**Nested Structure - Field Mapping**:
- Key: Measurement dataset name (e.g., "Spore Lengths")
- Value: iNaturalist observation field name (e.g., "Spore Length")

**Relationships**:
- 1 iNaturalist Session → N Measurement Datasets (via fieldMappings)
- 1 iNaturalist Session → N Captured Images (via uploadQueue)

**Validation Rules**:
- If `QDateTime::currentDateTime() > tokenExpiration`, must re-authenticate (FR-060)
- `observationUrl` format: `https://www.inaturalist.org/observations/{observationId}`

**State Transitions**:
```
[Unauthenticated] ──OAuth login──> [Authenticated] ──link observation──> [Linked] ──upload images──> [Syncing]
                                                                              ↑            │
                                                                              │            │
                                                                              └──sync complete──┘
[Authenticated] ──token expired──> [Token Expired] ──re-authenticate──> [Authenticated]
```

**Storage**: QSettings (access token, expiration, observation ID), transient upload queue

---

## Entity Relationship Diagram

```
Camera Profile ──1:N──> Captured Image
      │
      └──1:1 (optional)──> Microscope Profile ──1:N──> Objective
                                  │                        │
                                  │                        └──1:1 (optional)──> Calibration
                                  │
                                  └──1:N──> Calibration

Captured Image ──1:N──> Measurement ──N:1──> Measurement Dataset
      │
      ├──1:N──> Annotation
      │
      └──1:1 (optional)──> Detected Object Set ──1:N──> Detected Object

Captured Image ──N:1 (optional)──> Focus Stack
      │
      └──N:1 (optional)──> Stitched Panorama

RTSP Streaming Session ──1:N──> Connected Student

iNaturalist Session ──N:M──> Measurement Dataset (via fieldMappings)
      │
      └──N:M──> Captured Image (via uploadQueue)
```

## Validation Summary

### Cross-Entity Constraints
1. **Calibration Uniqueness**: Combination of (microscopeId, objectiveId, cameraId) must be unique
2. **Measurement Units**: All measurements in a dataset must have same unit
3. **Focus Stack Consistency**: All frames in a focus stack must have same resolution
4. **Panorama Grid**: Number of source frames must equal gridSize.width × gridSize.height
5. **RTSP Connection Limit**: Connected students cannot exceed maxStudents

### Data Integrity Rules
1. **Orphan Prevention**: Deleting a Microscope Profile must handle child Objectives and Calibrations (cascade or prevent)
2. **Reference Integrity**: All foreign keys (microscopeId, objectiveId, cameraId, calibrationId) must reference existing entities
3. **File Path Validation**: All file paths (images, videos) must be checked for existence and readability before use
4. **Token Expiration**: iNaturalist sessions must validate token expiration before API calls

### Performance Considerations
1. **Measurement Recalculation**: When unit changes, recalculate all measurements using stored calibration
2. **Statistics Caching**: Cache Measurement Dataset statistics, invalidate on dataset modification
3. **Metadata Serialization**: Use efficient JSON serialization for embedded metadata (avoid excessive nesting)
4. **RTSP Annotation Sync**: Use differential updates (only send changed annotations) to minimize bandwidth

---

**Data Model Status**: ✅ COMPLETE - All entities defined with attributes, relationships, validation rules, and state transitions
