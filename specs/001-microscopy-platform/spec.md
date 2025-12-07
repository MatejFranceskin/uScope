# Feature Specification: μScope Microscopy Platform

**Feature Branch**: `001-microscopy-platform`  
**Created**: 2025-12-05  
**Status**: Draft  
**Input**: Educational institutions, hobbyists, small laboratories, and researchers using general-purpose digital microscopy cameras (primarily USB/UVC-compliant devices). Core Goal: stable, cross-platform open-source application that replicates and improves upon proprietary microscopy software with intuitive UI, advanced imaging features (Stitching, EDF), and metrology.

## Clarifications

### Session 2025-12-05

- Q: Default file storage location strategy for captured images and videos? → A: Cross-platform user documents folder (e.g., `~/Documents/uScope/`)
- Q: Video codec and compression strategy for MP4/AVI recording? → A: H.264 with medium preset, CRF 23 (recommended quality)
- Q: RTSP streaming default resolution for classroom collaboration? → A: 720p (1280×720) at 30fps (recommended for bandwidth/quality balance)
- Q: Object detection algorithm for automated spore/object identification? → A: Contour detection with morphological operations (edge-based, flexible, no training required)
- Q: Scale bar OCR processing timing for external images? → A: Process automatically on image load (immediate feedback, user validates before measurements)

## UI Architecture: Tile-Based Overlay System

### Tile Positioning System

The UI uses a custom tile-based overlay system where all interactive elements (buttons, controls, dialogs) are positioned on a proportional grid system that scales with window size.

**Tile Sizing**:
- Each tile has a `widthMultiplier` and `heightMultiplier` (integer values)
- Base tile size = window height / 12
- Actual tile size = multiplier × base tile size
- Example: A 1×1 tile on a 1080px tall window = 90×90 pixels

**Grid Positioning**:
- Each tile has an X and Y position on an imaginary grid
- Y position starts at 0 and always counts from top to bottom
- X position behavior depends on anchor type:
  - **Anchor=Left**: X position means the tile is in the Xth column from the left (X=0 is leftmost)
  - **Anchor=Right**: X position means the tile is in the Xth column from the right (X=0 is rightmost)
  - **Anchor=Center**: X=0 means tile is centered, positive X values are columns to the right of center, negative X values are columns to the left of center

**Dialogs**:
- Dialogs are always centered on screen (ignoring X,Y positioning)
- Dialogs are modal (block interaction with other tiles)
- Dialogs can contain sub-tiles arranged within the dialog area

### Tile Layout and Functions

The following tiles comprise the main UI:

**Fullscreen Toggle** (Anchor: Right, Position: 0,0, Size: 1×1)
- Purpose: Toggles fullscreen mode that hides all other tiles except the video feed
- Icon: Custom fullscreen/expand icon
- State: Active when in fullscreen mode (different icon for exit fullscreen)
- Behavior: Single-state toggle button

**Camera Selection** (Anchor: Right, Position: 0,2, Size: 1×1)
- Purpose: Opens dialog showing list of available camera sources
- Icon: `images/camera.svg`
- Dialog: Displays all detected cameras, highlights currently selected camera
- Dialog Controls: OK button (apply selection), Cancel button (dismiss)
- Behavior: Click opens centered modal dialog with camera list

**Settings** (Anchor: Right, Position: 0,3, Size: 1×1)
- Purpose: Opens settings dialog for application configuration
- Icon: `images/settings.svg`
- Dialog: Contains sub-tiles for various settings categories
- Dialog Controls: Standard dialog with OK/Cancel buttons
- Behavior: Click opens centered modal dialog

**Snapshot** (Anchor: Right, Position: 0,4, Size: 1×1)
- Purpose: Captures a single high-resolution still image from current video feed
- Icon: `images/camera.svg` or dedicated snapshot icon
- Sound: Plays `sounds/camera-shutter.mp3` on capture
- Visual Feedback: Brief white flash overlay on capture
- Behavior: Single-click captures and saves image with timestamp

**Record** (Anchor: Right, Position: 0,5, Size: 1×1)
- Purpose: Starts/stops video recording
- States: Two-state toggle (Start/Stop recording)
  - **Start State**: Icon `images/media-record.svg` (red record icon)
  - **Stop State**: Icon `images/media-playback-stop.svg` (stop icon), tile shows active recording indicator
- Behavior: Click toggles recording state
- Visual Feedback: Recording state shows animated indicator or different background color

**Zoom** (Anchor: Right, Position: 0,6, Size: 1×1)
- Purpose: Shows current zoom factor and opens zoom controls panel
- Display: Shows current zoom level (e.g., "1.5x", "Fit", "100%")
- Icon: Magnifying glass or zoom icon
- State: Normal when zoom controls hidden, Active when zoom controls panel is visible
- Behavior: Click toggles zoom controls panel (3 sub-tiles appear/disappear)
- Zoom Controls Panel (appears when Zoom tile is Active):
  - **1:1 / 100%** (Sub-tile 1): Sets video to 1:1 pixel mapping (1 video pixel = 1 screen pixel), zoom = 1.0x
  - **Fit Width** (Sub-tile 2): Scales video to match viewport width, maintains aspect ratio
  - **Fit Height** (Sub-tile 3): Scales video to match viewport height, maintains aspect ratio
- Panel Behavior:
  - When any zoom control is clicked, zoom changes and panel automatically closes
  - When zoom changes via mouse wheel/pinch, panel automatically closes if open
  - Panel tiles positioned near Zoom tile (e.g., Right, Position: 1,6 through 3,6)
- Mouse Wheel / Touch Pinch: Zoom in/out continuously (1.0x to 10.0x range)
  - Mouse wheel up = zoom in (increase zoom factor by 0.1x per scroll)
  - Mouse wheel down = zoom out (decrease zoom factor by 0.1x per scroll)
  - Touch pinch in/out = zoom out/in proportionally
  - Zoom always centers on current viewport center
- Visual Feedback: Zoom factor updates in Zoom tile display in real-time

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Live Camera Preview and Basic Capture (Priority: P1) 🎯 MVP

A researcher connects their USB microscope camera, sees a live video feed immediately, and captures high-quality still images of their specimen with one click.

**Why this priority**: This is the fundamental workflow that all microscopy users require. Without reliable live preview and image capture, the application has no value.

**Independent Test**: Connect a UVC camera, verify smooth video feed appears automatically, click snap button, confirm high-resolution image is saved to disk with timestamp.

**Acceptance Scenarios**:

1. **Given** a UVC camera is connected, **When** application launches, **Then** camera is auto-detected and live preview starts at maximum supported frame rate
2. **Given** live preview is running, **When** user clicks "Snap Image" button, **Then** a full-resolution still image is captured and saved with timestamp filename
3. **Given** multiple cameras are connected, **When** user opens camera dropdown, **Then** all detected cameras are listed and user can switch between them
4. **Given** live preview is running, **When** user adjusts preview window size, **Then** video scales appropriately while maintaining aspect ratio
5. **Given** camera supports multiple resolutions, **When** user selects different resolution, **Then** preview updates without interruption

---

### User Story 2 - Zoom and Pan for Detailed Inspection (Priority: P2)

A researcher zooms into specific regions of the live video feed or captured images to inspect fine details, using mouse wheel or touch gestures for intuitive control.

**Why this priority**: Digital zoom is essential for examining specimen details without changing physical objectives. Provides quick inspection capability that complements physical magnification.

**Independent Test**: With live preview running, scroll mouse wheel to zoom in 3x, verify video magnifies smoothly. Use mouse to pan around zoomed view. Click "Fit Width" button, verify video scales to viewport width.

**Acceptance Scenarios**:

1. **Given** live preview or captured image is displayed, **When** user scrolls mouse wheel up, **Then** video zooms in by 0.1x increment, centered on current viewport center
2. **Given** video is zoomed in, **When** user scrolls mouse wheel down, **Then** video zooms out by 0.1x decrement, minimum zoom is "Fit Width" or "Fit Height" (whichever is smaller)
3. **Given** video is zoomed in beyond viewport, **When** user drags with mouse, **Then** video pans smoothly in drag direction
4. **Given** touch screen device is used, **When** user performs pinch gesture, **Then** video zooms in/out proportionally to pinch distance
5. **Given** zoom controls panel is hidden, **When** user clicks Zoom tile, **Then** panel appears showing three buttons: "1:1 (100%)", "Fit Width", "Fit Height", and Zoom tile state becomes Active
6. **Given** zoom controls panel is visible, **When** user clicks "1:1 (100%)" button, **Then** video scales to 1:1 pixel mapping (1 video pixel = 1 screen pixel, zoom factor = 1.0x), panel closes, Zoom tile returns to Normal state
7. **Given** zoom controls panel is visible, **When** user clicks "Fit Width" button, **Then** video scales to match viewport width while maintaining aspect ratio, panel closes, Zoom tile displays "Fit W"
8. **Given** zoom controls panel is visible, **When** user clicks "Fit Height" button, **Then** video scales to match viewport height while maintaining aspect ratio, panel closes, Zoom tile displays "Fit H"
9. **Given** zoom controls panel is visible, **When** user zooms via mouse wheel, **Then** panel automatically closes and Zoom tile displays current zoom factor (e.g., "2.3x")
10. **Given** video is at 1:1 zoom, **When** Zoom tile is displayed, **Then** tile shows "100%" or "1:1" label
11. **Given** video is zoomed to 2.5x, **When** user views Zoom tile, **Then** tile displays "2.5x"
12. **Given** zoom factor is set, **When** user captures image, **Then** captured image is at full camera resolution (not affected by display zoom)

---

### User Story 3 - Manual Camera Controls for Image Quality (Priority: P3)

A researcher adjusts exposure, white balance, and color settings to achieve optimal image quality for their specific specimen and lighting conditions.

**Why this priority**: Different specimens and lighting conditions require manual control. Auto-settings rarely work perfectly for microscopy.

**Independent Test**: With live preview running, adjust exposure slider and verify image brightness changes in real-time. Perform one-click white balance and confirm color correction.

**Acceptance Scenarios**:

1. **Given** live preview is running, **When** user drags exposure slider, **Then** image brightness updates in real-time (within 200ms)
2. **Given** specimen has color cast, **When** user clicks on white/clear area for auto white balance, **Then** image colors are corrected
3. **Given** image needs adjustment, **When** user modifies brightness/contrast/saturation sliders, **Then** changes appear immediately on live feed
4. **Given** camera is mounted inverted, **When** user toggles horizontal/vertical flip, **Then** image orientation corrects instantly
5. **Given** low-light specimen, **When** user increases gain/ISO, **Then** image becomes brighter with acceptable noise levels

---

### User Story 4 - Advanced Camera Support via PTP/libgphoto2 (Priority: P4)

A researcher connects a professional DSLR or mirrorless camera (e.g., Canon, Nikon, Sony) with microscope adapter, detects it automatically alongside UVC cameras, accesses manufacturer-specific controls, uses live view for framing, and captures high-resolution images or videos directly on camera.

**Why this priority**: Professional cameras offer superior image quality, larger sensors, and advanced features compared to USB webcams. Many research setups use DSLR/mirrorless cameras with microscope adapters for publication-quality imaging. This extends the application's camera support beyond basic UVC devices.

**Independent Test**: Connect Canon DSLR via USB, verify it appears in camera list alongside UVC cameras. Select PTP camera, verify live view appears. Adjust ISO and shutter speed, capture image to camera SD card, verify settings applied. Switch to on-board recording, verify video saves to camera.

**Acceptance Scenarios**:

1. **Given** PTP-compatible camera (Canon/Nikon/Sony DSLR or mirrorless) is connected via USB, **When** application starts or camera is hot-plugged, **Then** camera is detected by libgphoto2 and appears in camera selection list with manufacturer/model name
2. **Given** multiple cameras are connected (mix of UVC and PTP), **When** user opens camera selection dialog, **Then** all cameras are listed with clear indication of type (UVC vs PTP) and current connection status
3. **Given** PTP camera is selected, **When** connection is established, **Then** camera-specific capabilities are queried (supported ISO values, shutter speeds, aperture values, image formats, video modes)
4. **Given** PTP camera supports live view, **When** live view is activated, **Then** real-time preview frames are captured from camera and displayed in video feed area at camera's maximum live view frame rate (typically 10-30 fps)
5. **Given** PTP camera live view is active, **When** user adjusts camera settings (ISO, shutter speed, aperture, white balance), **Then** changes are sent to camera and reflected in live view within 500ms
6. **Given** camera-specific settings panel is open, **When** user views available controls, **Then** panel displays camera's native settings organized by category (Exposure: ISO, Shutter, Aperture; Image: Format, Quality, White Balance; Advanced: Focus mode, Drive mode, Metering)
7. **Given** PTP camera is active, **When** user clicks "Snapshot" button, **Then** camera captures full-resolution image to camera's storage (SD card) AND/OR transfers image to computer depending on camera's capture target setting
8. **Given** image capture target is set to "Camera Storage", **When** image is captured, **Then** thumbnail preview is shown with option to download full-resolution image from camera
9. **Given** image capture target is set to "Computer", **When** image is captured, **Then** full-resolution image is transferred via USB and saved to ~/Documents/uScope/ with timestamp
10. **Given** PTP camera supports video recording, **When** user clicks "Record" button, **Then** camera starts on-board video recording (video stored on camera's SD card), recording indicator shows in UI
11. **Given** on-board recording is active, **When** user clicks "Stop Recording", **Then** camera stops recording and finalizes video file on SD card
12. **Given** PTP camera is disconnected during operation, **When** connection is lost, **Then** application detects disconnection, shows error notification, automatically falls back to next available camera or test pattern
13. **Given** PTP camera supports autofocus, **When** user triggers autofocus via button or half-press simulation, **Then** camera performs AF operation and live view shows focused result
14. **Given** multiple PTP cameras from different manufacturers are available, **When** user switches between them, **Then** settings panel dynamically adapts to show manufacturer-specific controls for selected camera
15. **Given** PTP camera settings are adjusted, **When** user captures image or video, **Then** EXIF metadata includes all camera settings (ISO, shutter speed, aperture, focal length, white balance, capture time)

**Technical Requirements**:
- **Library**: libgphoto2 (cross-platform camera control library)
- **Supported Protocols**: PTP (Picture Transfer Protocol), PTP/IP for network cameras
- **Camera Detection**: Use `gp_camera_autodetect()` for USB-connected cameras, enumerate alongside V4L2 devices
- **Live View**: Use `gp_camera_capture_preview()` in loop for frame capture, decode JPEG preview to QImage
- **Settings Control**: Use `gp_camera_get_config()` and `gp_camera_set_config()` for camera parameter access
- **Capture Modes**: Support both "Capture to Camera" (SD card) and "Capture to Computer" (USB transfer) via capture target setting
- **Error Handling**: Detect and handle camera busy states, battery warnings, storage full conditions

**Known Limitations**:
- Live view frame rate limited by camera (typically 10-30 fps vs 60+ fps for UVC)
- Not all camera models support live view via PTP (older DSLRs may require mirror lock-up)
- Video recording to computer (live streaming) not supported by most cameras via PTP - only on-camera recording
- Some cameras require specific USB modes to be enabled in camera menu

---

### User Story 5 - Accurate Measurement and Calibration (Priority: P5)

A researcher calibrates the system using a stage micrometer, then measures specimen features (length, area, angles) with scientific accuracy.

**Why this priority**: Quantitative analysis is what differentiates scientific microscopy from casual photography. Essential for laboratory and educational use.

**Independent Test**: Load stage micrometer image, draw calibration line on known distance (e.g., 1mm), save calibration for 10x objective. Load specimen image, draw line measurement, verify result displays in micrometers.

**Acceptance Scenarios**:

1. **Given** user has multiple microscopes, **When** user creates microscope profile (name, manufacturer, model), **Then** profile is saved and available for selection
2. **Given** microscope profile is selected, **When** user connects camera, **Then** camera is associated with selected microscope
3. **Given** microscope profile is active, **When** user adds new objective (name, magnification like "10x" or "40x"), **Then** objective is added to microscope's objective list
4. **Given** stage micrometer image is loaded, **When** user selects objective from dropdown (e.g., "10x") and draws calibration line on known distance with real-world value (e.g., 100µm), **Then** calibration is saved linked to current microscope and selected objective
5. **Given** multiple objectives are calibrated, **When** user manually changes objective on physical microscope and selects matching objective in software dropdown, **Then** appropriate calibration is activated and scale bar updates
6. **Given** calibration exists for current microscope/objective combination, **When** user captures or loads specimen image, **Then** scale bar automatically appears showing correct scale for active objective
7. **Given** calibrated image is displayed, **When** user draws line measurement tool, **Then** length is displayed in micrometers (or selected unit)
8. **Given** user needs to measure circular object, **When** user draws circle tool, **Then** diameter, radius, and area are calculated and displayed
9. **Given** multiple measurements are made, **When** user adds measurements to a dataset, **Then** statistical summary (mean, std dev, min/max) is automatically calculated
10. **Given** user switches to different microscope, **When** new microscope profile is selected, **Then** objective list and calibrations update to match selected microscope
11. **Given** user loads image from external software without calibration metadata, **When** system analyzes image, **Then** scale bar is automatically detected using OCR and pattern recognition
12. **Given** scale bar is detected, **When** system extracts scale value (e.g., "100 µm") and bar length in pixels, **Then** calibration is automatically calculated and applied
13. **Given** automatic scale detection is uncertain or fails, **When** user reviews detected scale, **Then** user can manually correct scale value or bar length before accepting calibration
14. **Given** no scale bar is detected in external image, **When** user is notified, **Then** user can manually calibrate using manual measurement tools

---

### User Story 6 - Video Recording and Time-Lapse (Priority: P6)

A researcher records dynamic processes (e.g., cell movement, crystal formation) as video or time-lapse sequences for later analysis.

**Why this priority**: Documenting dynamic processes is a common need, but not required for basic static microscopy workflows.

**Independent Test**: Start video recording, verify video file is created and recording indicator shows. Stop recording, confirm video plays back correctly with proper compression.

**Acceptance Scenarios**:

1. **Given** live preview is running, **When** user clicks "Start Recording", **Then** video recording begins to MP4 file with real-time compression
2. **Given** recording is active, **When** user clicks "Stop Recording", **Then** video file is finalized and saved with timestamp filename
3. **Given** user wants time-lapse, **When** user configures interval (e.g., 5 seconds) and duration (e.g., 1 hour), **Then** system automatically captures images at intervals and compiles into video
4. **Given** recording is active, **When** user adjusts camera settings, **Then** changes are reflected in recorded video without interruption
5. **Given** limited storage space, **When** recording approaches disk limit, **Then** user receives warning before stopping

---

### User Story 7 - Automated Object Detection and Measurement (Priority: P7)

A mycologist or researcher photographs spores or other similar microscopic objects, runs automated detection to identify all instances, manually corrects any missed or false detections, then generates comprehensive measurement statistics across all detected objects.

**Why this priority**: Measuring dozens or hundreds of similar objects manually is tedious and error-prone. Automated detection with manual correction dramatically speeds up quantitative analysis while maintaining accuracy.

**Independent Test**: Load calibrated image of fungal spores, run automated detection algorithm, verify most spores are detected. Manually remove false positive and add missed spore. Run measurement statistics, confirm accurate dimensions for all objects.

**Acceptance Scenarios**:

1. **Given** calibrated image with multiple similar objects is loaded, **When** user selects "Detect Similar Objects" and configures detection parameters (size range, circularity, contrast threshold), **Then** algorithm identifies and highlights candidate objects
2. **Given** objects are detected, **When** user reviews detection results, **Then** each detected object is outlined with bounding box or contour overlay
3. **Given** detection includes false positive (non-spore object detected), **When** user clicks on false detection and selects "Remove", **Then** object is removed from detection set
4. **Given** detection missed a valid object, **When** user clicks on missed object and selects "Add Object", **Then** object is manually added to detection set with similar outline
5. **Given** user manually draws region around missed object, **When** user confirms addition, **Then** object is added to measurement set with same properties as auto-detected objects
6. **Given** all objects are correctly identified, **When** user runs "Measure All Objects", **Then** system automatically measures each object (length, width, area, perimeter, circularity) and compiles into measurement dataset
7. **Given** measurements are complete, **When** user views statistics, **Then** mean, standard deviation, min/max, and distribution histogram are displayed for each measurement type
8. **Given** measurement dataset exists, **When** user exports results, **Then** data is saved as CSV with one row per object and statistical summary included

---

### User Story 8 - Image Enhancement and Analysis (Priority: P8)

A researcher applies filters (sharpen, edge detection, histogram equalization) and performs basic segmentation to highlight specimen features.

**Why this priority**: Image processing helps reveal details but is not essential for basic imaging workflow. Can be done in post-processing with other tools.

**Independent Test**: Load specimen image, apply sharpen filter, verify edges are enhanced. Apply Otsu thresholding, confirm binary image is generated for particle counting.

**Acceptance Scenarios**:

1. **Given** captured image is loaded, **When** user applies sharpen filter, **Then** edge details are enhanced without introducing artifacts
2. **Given** image needs contrast enhancement, **When** user applies histogram equalization, **Then** tonal distribution is optimized
3. **Given** user wants to detect edges, **When** Sobel edge detection is applied, **Then** specimen boundaries are highlighted
4. **Given** image has histogram displayed, **When** user adjusts black/white clipping points, **Then** image contrast updates in real-time
5. **Given** user needs binary image, **When** Otsu thresholding is applied, **Then** optimal threshold is calculated and binary mask is generated

---

### User Story 9 - Large Area Imaging with Stitching (Priority: P9)

A researcher captures multiple overlapping frames of a large specimen and automatically stitches them into a single high-resolution panorama.

**Why this priority**: Addresses field-of-view limitations but requires significant implementation complexity. Most users can work with single-frame images.

**Independent Test**: Capture 3x3 grid of overlapping images of stage micrometer. Run stitching algorithm, verify seamless panorama is produced with correct scale bar.

**Acceptance Scenarios**:

1. **Given** large specimen exceeds field of view, **When** user enters stitching mode and defines grid size (e.g., 3x3), **Then** application guides user through capturing overlapping frames
2. **Given** all frames are captured, **When** user clicks "Stitch Images", **Then** frames are automatically aligned and blended into seamless panorama
3. **Given** stitched image is complete, **When** user reviews result, **Then** overlap regions show no visible seams or brightness discontinuities
4. **Given** stitched panorama has calibration, **When** user makes measurements, **Then** scale is consistent across entire stitched area
5. **Given** stitching fails due to insufficient overlap, **When** algorithm completes, **Then** user receives clear error message with guidance

---

### User Story 10 - Extended Depth of Focus (EDF) (Priority: P10)

A researcher captures multiple images at different focus depths and fuses them into a single all-in-focus image, overcoming depth-of-field limitations.

**Why this priority**: Very useful for thick specimens but requires Z-axis motorization or manual coordination. Complex algorithm implementation.

**Independent Test**: Manually capture 5-10 images at different focus positions of thick specimen. Run EDF algorithm, verify all layers appear sharp in final composite.

**Acceptance Scenarios**:

1. **Given** specimen has significant depth, **When** user enters EDF mode and captures images at multiple Z-positions, **Then** all frames are stored in focus stack
2. **Given** focus stack is complete, **When** user runs EDF fusion algorithm, **Then** all specimen layers are combined with sharp focus throughout
3. **Given** EDF image is generated, **When** user compares to individual frames, **Then** composite shows no focus-related blur in any region
4. **Given** user reviews EDF parameters, **When** algorithm settings are adjusted, **Then** fusion quality improves for specific specimen type
5. **Given** EDF processing fails, **When** algorithm encounters error, **Then** user receives diagnostic message about stack quality

---

### User Story 11 - Professional Documentation and Export (Priority: P11)

A researcher generates publication-ready reports containing images, measurements, statistics, and metadata in standard scientific formats.

**Why this priority**: Important for professional work but can initially be handled by manual documentation. Export formats are refinements.

**Independent Test**: Capture calibrated image with measurements and annotations. Generate PDF report, verify it contains image, measurement table, statistics, and metadata. Export as OME-TIFF, confirm metadata is preserved.

**Acceptance Scenarios**:

1. **Given** calibrated image with measurements, **When** user clicks "Generate Report", **Then** PDF is created with image, measurement table, statistics, and acquisition metadata
2. **Given** image needs scientific archiving, **When** user exports as OME-TIFF, **Then** all calibration data and metadata are embedded in standard format
3. **Given** image has calibration and annotation layers, **When** user saves as JPEG/PNG/TIFF, **Then** calibration data (microscope, objective, µm/pixel) and annotation layer data are embedded in metadata
4. **Given** previously saved image with embedded annotations, **When** user opens image in μScope, **Then** annotation layers are reconstructed and editable
5. **Given** video recording is active with calibration, **When** video is saved, **Then** calibration metadata is embedded in video file container
6. **Given** multiple measurement datasets exist, **When** user includes them in report, **Then** comparative statistics and visualizations are generated
7. **Given** report is generated, **When** user opens PDF, **Then** all images are high-resolution and text is searchable
8. **Given** EXIF metadata is important, **When** user is saved as JPEG/PNG/TIFF, **Then** camera settings, timestamp, calibration, and custom annotation data are preserved

---

### User Story 10 - Classroom Collaboration with RTSP Streaming (Priority: P10)

A teacher connects a microscope to their computer, enables classroom mode which broadcasts the live video feed and annotations via RTSP over the local network. Students on mobile devices or laptops discover and connect to the teacher's stream, viewing the same microscope feed with real-time annotations.

**Why this priority**: Enables effective classroom instruction and remote learning scenarios. Transforms individual microscopy into collaborative learning experience without expensive hardware replication.

**Independent Test**: Teacher enables RTSP streaming on desktop application connected to microscope. Student opens mobile app on same LAN, sees teacher's stream advertised, connects to stream, verifies live video and annotations appear in real-time.

**Acceptance Scenarios**:

1. **Given** teacher has microscope connected and live preview running, **When** teacher enables "Classroom Mode", **Then** RTSP server starts and broadcasts availability via UDP on local network
2. **Given** RTSP server is running, **When** teacher makes annotations (arrows, text, shapes) on live feed, **Then** annotations are embedded in RTSP stream metadata
3. **Given** student app is on same LAN, **When** student opens camera source selection, **Then** teacher's RTSP stream appears as available source with teacher name/identifier
4. **Given** student selects teacher's RTSP stream, **When** connection is established, **Then** student sees live microscope feed with same quality as teacher (resolution adjusted for bandwidth)
5. **Given** teacher draws measurement or annotation, **When** change is made, **Then** annotation appears on all connected student devices within 500ms
6. **Given** multiple students are connected, **When** teacher adjusts camera settings (exposure, zoom, focus), **Then** all students see updated feed in real-time
7. **Given** teacher captures still image, **When** image is saved, **Then** students receive notification and can optionally download captured image
8. **Given** network connection is unstable, **When** bandwidth drops, **Then** student app automatically reduces stream quality while maintaining connectivity
9. **Given** teacher disables classroom mode, **When** RTSP server stops, **Then** students receive disconnection notification with graceful fallback
10. **Given** student device has poor network, **When** latency exceeds threshold, **Then** student sees connection quality indicator and buffering status

---

### User Story 11 - iNaturalist Integration (Priority: P11)

A naturalist or mycologist links their imaging session to an iNaturalist observation, captures specimen images, and automatically populates observation fields (like spore dimensions) from measured statistics.

**Why this priority**: Valuable for citizen science and biodiversity research but optional feature. Core microscopy workflow works independently.

**Independent Test**: Authenticate with iNaturalist, link session to existing observation, capture calibrated images with measurements, push images to observation, verify spore dimension observation field is populated with measurement statistics.

**Acceptance Scenarios**:

1. **Given** user enables iNaturalist integration, **When** user authenticates with iNaturalist credentials (OAuth), **Then** authentication is successful and stored securely
2. **Given** user is authenticated, **When** user starts new session and selects "Link to iNaturalist Observation", **Then** user can search for existing observation or create new one
3. **Given** session is linked to observation, **When** user captures and measures specimen images, **Then** images are marked for upload to linked observation
4. **Given** images are ready for upload, **When** user clicks "Push to iNaturalist", **Then** images are uploaded to observation with embedded metadata
5. **Given** user has measurement statistics (e.g., spore dimensions: mean 8.5µm ± 1.2µm), **When** user selects "Add to Observation Fields", **Then** appropriate observation fields are populated (e.g., "Spore Length" = "8.5µm ± 1.2µm")
6. **Given** multiple measurement datasets exist, **When** user maps datasets to observation fields, **Then** each field receives correctly formatted statistical summary
7. **Given** session is linked to observation, **When** user ends session, **Then** link is preserved and can be resumed later
8. **Given** network is unavailable, **When** user tries to push images, **Then** images are queued for upload when connection is restored

---

### Edge Cases

- What happens when camera is disconnected during live preview? System must detect disconnect, show clear error message, and attempt reconnection when camera returns.
- How does system handle unsupported camera formats? Display informative error and list supported camera types (UVC-compliant devices).
- What happens when user forgets to select correct objective after physically changing it? Measurements will be incorrect - system should display prominent objective indicator and optionally warn if image characteristics suggest wrong magnification.
- What happens when user tries to calibrate without selecting a microscope? System should prompt user to create/select microscope profile first.
- What happens when user deletes microscope profile with existing calibrations? System should warn and offer to export calibrations or confirm deletion.
- What happens when multiple scale bars are detected in external image? Highlight all detected scale bars, prompt user to select correct one or manually calibrate.
- How does system handle scale bars with non-standard units (e.g., "0.5 mm" or "50 nm")? Support common unit variants (µm, um, μm, mm, nm, Å) and convert to standard units.
- What happens when OCR misreads scale bar text (e.g., "100 um" as "l00 um")? Show detected text to user for manual correction before applying calibration.
- How does system handle images with burned-in scale bars at various positions (corner, bottom, embedded)? Use computer vision to detect horizontal/vertical bars in common locations, validate with text proximity.
- What happens when scale bar is partially obscured or low contrast? Provide confidence score with detection, allow user to manually trace scale bar if automatic detection fails.
- How does system handle extremely large stitched images (>1GB)? Implement memory-efficient processing with progress indication and chunked saving.
- What happens when measurement units are changed after measurements are made? All existing measurements should recalculate and display in new units.
- How does system handle missing dark-field frame during correction? Skip correction or use previously captured dark frame with timestamp warning.
- What happens when disk space runs out during video recording? Stop recording gracefully, finalize video file, and show clear storage warning.
- How does system handle overlapping annotations on measurement layer? Allow layering with Z-order control or transparency.
- What happens when binning mode is unavailable on camera? Hide or disable binning controls for cameras that don't support hardware binning.
- How does system handle invalid threshold values in segmentation? Clamp values to valid range (0-255) and provide visual feedback.
- What happens when automated object detection finds no objects? Display message indicating no objects detected, suggest adjusting detection parameters (size range, threshold).
- How does system handle overlapping detected objects? Provide options to merge overlapping detections or keep as separate objects, show overlap percentage.
- What happens when user manually adds object but doesn't draw complete boundary? Prompt user to complete boundary or use automatic edge detection to complete contour.
- How does system handle images with hundreds of detected objects? Implement efficient rendering with object count limit warning, allow batch operations for review/removal.
- What happens when detection parameters produce too many false positives? Allow user to adjust sensitivity, provide "Train from Selection" option to refine detection based on user-confirmed objects.
- What happens when loading image with embedded annotations in external image viewer? Annotations are invisible (stored in metadata) but preserved; reopening in μScope reconstructs them.
- How does system handle corrupted or incomplete annotation metadata? Display warning, load partial annotations if possible, or skip corrupted entries gracefully.
- What happens when image format doesn't support custom metadata (e.g., some JPEG variants)? Warn user and offer to save annotations as separate sidecar file (.json or .xml).
- How does system handle images with calibration metadata from different software? Attempt to import if compatible, otherwise prompt user to recalibrate or ignore existing calibration.
- What happens when teacher starts RTSP server but no students connect? Server runs normally, no performance impact, teacher sees "0 connected clients" indicator.
- How does system handle RTSP stream discovery on networks with multiple subnets? Use multicast DNS (mDNS/Bonjour) as fallback to UDP broadcast for better subnet traversal.
- What happens when multiple teachers enable classroom mode on same network? Each RTSP stream is uniquely identified by teacher name + device ID, students see multiple available streams.
- How does system handle student connection when teacher's network bandwidth is limited? Limit maximum concurrent student connections (configurable, default 30), queue additional connections with wait notification.
- What happens when RTSP stream is interrupted mid-session? Student app attempts automatic reconnection with exponential backoff, shows reconnection status to user.
- How does system handle annotation synchronization latency over slow networks? Implement differential updates (only send changed annotations), prioritize critical updates, show sync status indicator.
- What happens when student tries to make annotations on teacher's stream? Student annotations are local-only by default, optionally can be sent back to teacher as "student questions" overlay in separate layer.
- How does system secure RTSP stream from unauthorized access? Implement optional password protection, MAC address filtering, or session tokens for stream access.
- What happens when teacher switches microscope objectives during classroom session? Calibration update is broadcast to all students, scale bars update automatically on student devices.
- What happens when iNaturalist authentication expires? Detect expired token, prompt user to re-authenticate before upload operations.
- How does system handle network failures during iNaturalist image upload? Queue images locally, retry automatically when connection is restored, show upload status.
- What happens when observation is deleted on iNaturalist while session is linked? Detect deleted observation on next sync, notify user, offer to unlink or link to different observation.
- How does system map measurement statistics to iNaturalist observation fields? Provide user interface to map measurement datasets to field names (e.g., "Spore Length" field), format statistics appropriately.
- What happens when user tries to push images without linking to observation? Prompt user to link session to observation first or create new observation.

## Requirements *(mandatory)*

### Functional Requirements

**Live View and Acquisition**

- **FR-001**: System MUST automatically detect and list all connected UVC (USB Video Class) cameras on application launch
- **FR-002**: System MUST provide smooth live video preview at the maximum frame rate supported by the selected camera (minimum 15 fps target)
- **FR-003**: System MUST allow user to select camera from dropdown when multiple cameras are connected
- **FR-004**: System MUST capture full-resolution still images with single button click while maintaining live preview
- **FR-005**: System MUST support independent resolution settings for live preview (low-res/high-fps) and capture (high-res/full-quality)
- **FR-006**: System MUST record video to standard formats (MP4, AVI) with real-time compression options
- **FR-006.1**: System MUST encode video using H.264/AVC codec with medium preset and CRF 23 quality setting for optimal balance of file size, quality, and compatibility
- **FR-007**: System MUST support time-lapse capture with configurable intervals and total duration
- **FR-008**: System MUST save captured images with automatic timestamp-based filenames
- **FR-008.1**: System MUST use cross-platform user documents folder as default save location (`~/Documents/uScope/` on Linux/macOS, `%USERPROFILE%\Documents\uScope\` on Windows, app-specific storage on Android/iOS)

**Camera and Imaging Controls**

- **FR-009**: System MUST provide manual controls for exposure time and sensor gain/ISO with real-time preview updates (<200ms response time per FR-015)
- **FR-010**: System MUST offer white balance presets (Tungsten, Daylight, Auto) and one-click auto white balance (click on white area)
- **FR-011**: System MUST provide sliders for brightness, contrast, saturation, and gamma correction on live feed with real-time updates (<200ms response time per FR-015)
- **FR-012**: System MUST support hardware binning modes (2x2, 4x4) if exposed by camera API
- **FR-013**: System MUST provide dark-field correction tool to capture and subtract sensor noise
- **FR-014**: System MUST offer image flip/mirror controls (horizontal/vertical) for optical correction
- **FR-015**: System MUST apply all image adjustments within 200ms of user input

**Quantitative Analysis and Metrology**

- **FR-016**: System MUST allow users to create and manage microscope profiles with name, manufacturer, and model information
- **FR-016.1**: System MUST allow users to define multiple objectives for each microscope profile with name and magnification value
- **FR-016.2**: System MUST provide calibration tool to define pixel-to-micrometer scale by drawing reference line on stage micrometer image
- **FR-017**: System MUST store calibration data linked to specific microscope profile, objective, and camera combination
- **FR-017.1**: System MUST automatically detect scale bars in loaded images using computer vision (edge detection, line detection, OCR)
- **FR-017.1.1**: System MUST trigger scale bar detection automatically when user loads external image without embedded calibration metadata, displaying detected scale with confidence score for user validation
- **FR-017.2**: System MUST extract scale bar length in pixels and associated text label using OCR (supporting µm, um, μm, mm, nm, Å units)
- **FR-017.3**: System MUST calculate pixel-to-micrometer ratio from detected scale bar and apply as temporary calibration
- **FR-017.4**: System MUST display confidence score for automatic scale detection and highlight detected scale bar region
- **FR-017.5**: System MUST allow users to manually correct detected scale value, bar length, or units before accepting calibration
- **FR-017.6**: System MUST provide fallback to manual calibration when automatic scale detection fails or confidence is low (<70%)
- **FR-018**: System MUST display dynamic scale bar on live and captured images based on active calibration
- **FR-019**: System MUST provide measurement tools for: line segments (length), angles, circles (radius/diameter/area), rectangles (length/width/area), and polygons (perimeter/area)
- **FR-020**: System MUST support manual object counting with tally tool
- **FR-021**: System MUST allow measurement unit selection: micrometers (µm), millimeters (mm), centimeters (cm), inches (in), pixels (px)
- **FR-022**: System MUST group measurements into named datasets for statistical analysis
- **FR-023**: System MUST calculate statistics for measurement datasets: mean, standard deviation, minimum, maximum, confidence intervals
- **FR-024**: System MUST provide non-destructive annotation layer with text, arrows, and shapes
- **FR-025**: System MUST allow annotations to be saved separately or burned into exported image

**Automated Object Detection and Measurement**

- **FR-026**: System MUST provide automated object detection algorithm with configurable parameters (size range, circularity, contrast threshold, morphology filters)
- **FR-027**: System MUST detect and outline similar objects in calibrated images using computer vision algorithms (contour detection, blob analysis, or template matching)
- **FR-027.1**: System MUST implement contour-based detection using OpenCV (grayscale conversion, Gaussian blur, adaptive/Otsu thresholding, morphological operations for noise removal, findContours for boundary detection) as primary detection method
- **FR-028**: System MUST display detected objects with visual indicators (bounding boxes, contours, or masks) overlaid on image
- **FR-029**: System MUST allow users to manually remove false positive detections from the detection set
- **FR-030**: System MUST allow users to manually add missed objects by clicking or drawing regions
- **FR-031**: System MUST automatically measure geometric properties for all detected objects (length, width, area, perimeter, circularity, aspect ratio)
- **FR-032**: System MUST compile measurements from all objects into a measurement dataset with per-object and aggregate statistics
- **FR-033**: System MUST export object detection results as CSV with one row per object and summary statistics
- **FR-034**: System MUST display histogram distribution of measurements across all detected objects
- **FR-035**: System MUST save detection parameters and results with image for reproducibility

**Advanced Compositing and Automation**

- **FR-036**: System MUST provide guided multi-frame capture for image stitching with overlap indicators
- **FR-037**: System MUST automatically stitch overlapping frames into panoramic image using feature matching
- **FR-038**: System MUST provide Z-stack capture interface for Extended Depth of Focus (EDF)
- **FR-039**: System MUST fuse multiple focus images into single all-in-focus composite using focus stacking algorithm
- **FR-040**: System SHOULD include image gallery panel to review recently captured images and videos (Priority: P12 - Future Enhancement; MVP uses file manager integration; gallery would appear as right-anchored tile panel with thumbnail grid, click to load into main view)

**Image Processing and Filtering**

- **FR-041**: System MUST apply all image adjustments non-destructively and save as metadata
- **FR-042**: System MUST provide image filters: Sharpen, Blur (Gaussian), Edge Detection (Sobel), Histogram Equalization
- **FR-043**: System MUST display real-time histogram (RGB and Luminance) with adjustable black/white clipping points
- **FR-044**: System MUST provide thresholding tool (Otsu's method) for binary image conversion
- **FR-045**: System MUST support basic particle counting and area calculation from binary images

**Data Management and Export**

- **FR-046**: System MUST save still images in JPEG (lossy), PNG (lossless), and TIFF (lossless) formats
- **FR-047**: System MUST embed acquisition metadata in EXIF/TIFF fields: exposure, gain, timestamp, camera model
- **FR-047.1**: System MUST embed calibration metadata in image files: microscope name, objective name/magnification, pixel-to-micrometer scale factor
- **FR-047.2**: System MUST embed annotation layer data in custom metadata fields: annotation type, position, text content, styling, shape parameters
- **FR-047.3**: System MUST use non-standard metadata fields (e.g., EXIF UserComment, TIFF custom tags, PNG text chunks) to store annotation data in vendor-neutral format
- **FR-047.4**: System MUST reconstruct annotation layers when loading previously saved images with embedded annotation metadata
- **FR-047.5**: System MUST embed calibration metadata in video file containers (MP4 metadata atoms, AVI INFO chunks)
- **FR-048**: System MUST support OME-TIFF export format for scientific compatibility
- **FR-049**: System MUST generate PDF reports containing: captured images, measurement tables, annotations, acquisition metadata, and statistical summaries
- **FR-050**: System MUST preserve measurement datasets across application sessions

**iNaturalist Integration (Optional)**

- **FR-051**: System MUST support OAuth authentication with iNaturalist API
- **FR-052**: System MUST allow users to link imaging session to existing iNaturalist observation by observation ID or search
- **FR-053**: System MUST allow users to create new iNaturalist observation from within application
- **FR-054**: System MUST upload captured images to linked iNaturalist observation via API
- **FR-055**: System MUST populate iNaturalist observation fields with measurement statistics (e.g., "Spore Length", "Spore Width")
- **FR-056**: System MUST provide mapping interface to link measurement datasets to observation field names
- **FR-057**: System MUST format statistical data appropriately for observation fields (e.g., "8.5µm ± 1.2µm" or "mean: 8.5µm, range: 6.2-10.8µm")
- **FR-058**: System MUST queue images for upload when network is unavailable and retry automatically when connection is restored
- **FR-059**: System MUST persist iNaturalist session links across application restarts
- **FR-060**: System MUST detect and handle authentication token expiration with re-authentication prompt

**Classroom Collaboration and RTSP Streaming (Optional)**

- **FR-061**: System MUST provide "Classroom Mode" toggle to enable/disable RTSP video server
- **FR-062**: System MUST broadcast RTSP stream availability on local network using UDP broadcast messages with service discovery information (teacher name, device ID, stream URL)
- **FR-063**: System MUST encode live camera feed to RTSP stream with configurable quality settings (resolution, bitrate, frame rate)
- **FR-063.1**: System MUST default RTSP stream to 720p (1280×720) resolution at 30fps, providing optimal balance of clarity for microscopy viewing, bandwidth efficiency (~3-5 Mbps with H.264), and broad device compatibility
- **FR-064**: System MUST embed real-time annotation data in RTSP stream using metadata tracks or custom RTP extensions
- **FR-065**: System MUST implement mDNS/Bonjour service discovery as fallback for multi-subnet network environments
- **FR-066**: System MUST display list of discovered RTSP streams from teachers on same local network in student app camera source selector
- **FR-067**: System MUST connect to selected RTSP stream and decode video with annotation overlay rendering
- **FR-068**: System MUST synchronize teacher annotations (measurements, shapes, text) to all connected students within 500ms
- **FR-069**: System MUST display connection quality indicator showing latency, bandwidth, and number of connected students (teacher view) or stream health (student view)
- **FR-070**: System MUST implement adaptive bitrate streaming to adjust quality based on available bandwidth
- **FR-071**: System MUST limit maximum concurrent student connections to configurable value (default 30)
- **FR-072**: System MUST provide optional stream authentication via password or session token
- **FR-073**: System MUST notify students when teacher captures still image and optionally allow image download
- **FR-074**: System MUST gracefully handle disconnections with automatic reconnection attempts (exponential backoff)
- **FR-075**: System MUST allow students to optionally send local annotations back to teacher as "student questions" layer

### Key Entities

- **Camera Profile**: Represents connected camera with properties (name, resolution, frame rate capabilities, supported formats, vendor/model ID)
- **Microscope Profile**: Represents physical microscope with properties (user-defined name, manufacturer, model, list of objectives, associated camera)
- **Objective**: Represents microscope objective with properties (name like "Plan 10x" or "10x", magnification value, parent microscope profile)
- **Calibration**: Pixel-to-real-world scale factor (µm/pixel) linked to specific microscope profile, objective, and camera combination
- **Detected Scale Bar**: Automatically detected scale bar in external image with properties (bar region bounding box, length in pixels, OCR text, parsed scale value, units, confidence score, user validation status)
- **Captured Image**: Still image with embedded metadata including: timestamp, camera settings (exposure, gain, white balance), active calibration (microscope, objective, µm/pixel), annotation layer data (positions, types, text, styling), and measurements. Metadata stored in EXIF/TIFF fields and custom tags for μScope-specific data.
- **Measurement**: Geometric measurement with type (line/circle/angle/polygon), value, units, and reference to parent image
- **Measurement Dataset**: Named collection of measurements with computed statistics (mean, std dev, range, confidence intervals)
- **Detected Object Set**: Collection of automatically detected objects with properties (detection algorithm, parameters used, list of object contours/bounding boxes, user-confirmed/rejected flags, per-object measurements)
- **Detected Object**: Individual object identified by detection algorithm with properties (contour/boundary, centroid position, geometric measurements, detection confidence, user validation status)
- **Annotation**: Non-destructive overlay with type (text/arrow/shape), position, styling, and optional burn-in flag
- **Video Recording**: Video file with embedded metadata including: format, compression settings, frame rate, start/end timestamps, active calibration data (microscope, objective, µm/pixel) stored in video container metadata
- **Focus Stack**: Collection of images captured at different Z-positions for EDF processing
- **Stitched Panorama**: Composite image created from multiple overlapping frames with alignment metadata
- **RTSP Streaming Session**: Classroom collaboration session with properties (teacher identifier, RTSP server URL, authentication token, list of connected students, stream quality settings, annotation synchronization state)
- **Connected Student**: Student client connected to RTSP stream with properties (device ID, device name, connection timestamp, network quality metrics, permissions)
- **iNaturalist Session**: Link between imaging session and iNaturalist observation with properties (observation ID, observation URL, authentication token, upload queue, field mappings from measurement datasets to observation field names)

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Users can connect a UVC camera and see live preview within 3 seconds of application launch
- **SC-002**: Live video preview maintains minimum 15 fps on reference hardware (Intel Core i5 or equivalent) with 1920x1080 camera
- **SC-003**: Still image capture completes within 1 second from button click to file saved on disk
- **SC-004**: Camera control adjustments (exposure, white balance, color) reflect in live preview within 200ms
- **SC-005**: Users can create microscope profile, add 4 objectives, and complete calibration for one objective in under 3 minutes
- **SC-005.1**: Users can switch between objectives in under 2 seconds (select from dropdown)
- **SC-005.2**: Automatic scale bar detection successfully extracts calibration from 80% of external microscopy images with clear, high-contrast scale bars
- **SC-005.3**: Scale bar detection and OCR processing completes within 3 seconds for typical microscopy images (up to 4K resolution)
- **SC-006**: Measurement tools display results instantly (<100ms) after drawing completion
- **SC-007**: Statistical calculations for 100-measurement dataset complete in under 500ms
- **SC-008**: Image stitching of 9 overlapping frames (3x3 grid) completes within 30 seconds on reference hardware
- **SC-009**: EDF fusion of 10-frame focus stack completes within 20 seconds on reference hardware
- **SC-010**: PDF report generation with 5 images and full measurement data completes within 10 seconds
- **SC-011**: Application successfully detects and operates with 90% of UVC-compliant USB cameras
- **SC-012**: Users can perform basic imaging workflow (connect camera, capture image, make measurement, save) within 5 minutes on first use
- **SC-012.1**: Automated object detection identifies 90% of similar objects (e.g., spores) with <10% false positive rate on well-contrasted specimens
- **SC-012.2**: Users can manually correct detection results (add/remove objects) and generate statistics for 100 objects in under 3 minutes
- **SC-012.3**: Object detection and measurement of 100 objects completes within 10 seconds on reference hardware
- **SC-013**: 95% of captured images retain full metadata (EXIF/TIFF) when saved in standard formats
- **SC-013.1**: 100% of images with annotations successfully reconstruct annotation layers when reopened in μScope
- **SC-013.2**: Calibration metadata (microscope, objective, scale) is preserved in 100% of saved images and videos
- **SC-014**: Touch targets meet 44x44 point minimum requirement on mobile platforms for accessibility
- **SC-015**: Application handles backgrounding/foregrounding on mobile without losing camera connection or unsaved data
- **SC-015.1**: Students discover teacher's RTSP stream on local network within 5 seconds of teacher enabling classroom mode
- **SC-015.2**: RTSP stream maintains <500ms end-to-end latency for video and annotations on typical LAN (1 Gbps)
- **SC-015.3**: System supports minimum 30 concurrent student connections with 720p stream quality on reference hardware (Intel Core i5, 1 Gbps network)
- **SC-015.4**: Annotation synchronization completes within 500ms from teacher action to student display on stable network
- **SC-016**: Users can authenticate with iNaturalist, link session to observation, and upload images in under 2 minutes
- **SC-017**: Measurement statistics are correctly formatted and populate iNaturalist observation fields with 95% accuracy

## Assumptions

- Users have access to UVC-compliant USB cameras or supported legacy cameras with driver documentation
- Users possess basic microscopy knowledge (understand magnification, calibration, focus, objective changing)
- Stage micrometers or calibration standards are available for measurement accuracy
- Desktop platforms have sufficient processing power for real-time video and image processing (Intel Core i5 or equivalent, 8GB RAM minimum recommended)
- Mobile platforms support Qt Multimedia and have adequate camera API access (Android 6.0+, iOS 13+)
- Users understand that calibration must be performed separately for each microscope and objective combination
- Users manually change objectives on physical microscope and remember to update objective selection in software
- External microscopy images typically include burned-in scale bars with readable text labels (e.g., "100 µm", "50 um")
- Scale bars in external images are generally horizontal or vertical lines with high contrast against background
- OCR library (e.g., Tesseract) is available for text recognition in scale bar detection, or can be integrated as dependency
- Users will verify automatically detected scale calibration before making critical measurements
- Most users work with 1-3 microscopes and 3-6 objectives per microscope (typical educational/hobbyist setup)
- Annotation metadata embedded in images may not be visible in third-party image viewers but will be preserved
- Users understand that μScope-specific metadata (annotations, calibration) requires μScope to fully reconstruct
- Standard image viewers will display images correctly but won't show annotation overlays
- Storage space is sufficient for high-resolution images and video files (minimum 1GB free recommended)
- OpenCV library is available for advanced features (object detection, stitching, EDF) or can be integrated as dependency
- Automated object detection works best with well-contrasted specimens on uniform backgrounds (e.g., spores on stage micrometer)
- Users understand that detection accuracy varies with specimen type and may require parameter tuning
- Network connectivity is not required for core functionality (offline-capable)
- Classroom collaboration feature requires local network (LAN or Wi-Fi) connectivity for teacher and students
- Teacher's computer has sufficient network bandwidth to stream video to multiple students (minimum 10 Mbps upload recommended for 10 students at 720p)
- Students' devices support RTSP client libraries and have adequate network bandwidth (minimum 2 Mbps per student)
- Network infrastructure allows UDP broadcast or multicast traffic for service discovery
- Typical classroom setup has 5-30 students on same local network with teacher
- Users understand that RTSP streaming introduces minimal latency (<500ms on LAN) which is acceptable for educational purposes
- Optional stream authentication is configured by teacher/administrator for security in shared network environments
- iNaturalist integration requires internet connection for authentication and upload operations
- Users have iNaturalist accounts if they choose to use integration features
- iNaturalist API is available and observation field naming conventions are documented
- Users understand that iNaturalist integration is optional and core microscopy features work independently
- Users can provide feedback on camera compatibility for continuous improvement of device support
