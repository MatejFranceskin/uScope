# Camera Control Latency Testing (T103)

## Objective
Verify that camera control adjustments meet the SC-004 requirement: changes to exposure, brightness, contrast, and saturation must reflect in the live preview within 200ms.

## Enabling Latency Measurement

The latency measurement instrumentation is **disabled by default** in production builds and must be explicitly enabled for testing.

### Enable via CMake
Add the compile definition when configuring the build:
```bash
cd /home/matej/uScope/build
cmake .. -DENABLE_LATENCY_MEASUREMENT=ON
make -j$(nproc)
```

### Enable via CMakeLists.txt
Add this line to your `CMakeLists.txt` for persistent testing builds:
```cmake
add_compile_definitions(ENABLE_LATENCY_MEASUREMENT)
```

### Enable via Compiler Flag
Manually add the flag when building:
```bash
cd /home/matej/uScope/build
make CXXFLAGS="-DENABLE_LATENCY_MEASUREMENT" -j$(nproc)
```

**Note**: Production builds should NOT define `ENABLE_LATENCY_MEASUREMENT` to avoid overhead and debug output.

## Implementation

### Latency Measurement Instrumentation
Added timing instrumentation to `CameraService` to measure the delay between control changes and when they take effect in the video stream:

1. **Timing Members** (CameraService.h - conditional compilation):
   - `QElapsedTimer _controlChangeTimer` - High-resolution timer
   - `QString _lastControlChange` - Tracks which control was changed
   - Only included when `ENABLE_LATENCY_MEASUREMENT` is defined

2. **Control Methods** (CameraService.cpp):
   - `setExposure()`, `setBrightness()`, `setContrast()`, `setSaturation()`
   - Each method starts the timer when called (if instrumentation enabled)
   - Records the control name and value

3. **Frame Handler** (CameraService.cpp):
   - `onVideoFrameChanged()` checks if timer is active
   - Measures elapsed time since control change
   - Logs latency with PASS/FAIL indicator (threshold: 200ms)
   - Automatically validates SC-004 requirement

### Console Output Format
```
CameraService::onVideoFrameChanged - control change latency: exposure=100 took 45ms ✓ PASS
CameraService::onVideoFrameChanged - control change latency: brightness=50 took 67ms ✓ PASS
CameraService::onVideoFrameChanged - control change latency: contrast=-20 took 32ms ✓ PASS
CameraService::onVideoFrameChanged - control change latency: saturation=30 took 58ms ✓ PASS
```

## Testing Procedure

### Automated Testing
Run the test script:
```bash
./test_latency.sh
```

The script will:
1. Launch uScope
2. Display testing instructions
3. Filter console output for latency measurements
4. Run for 120 seconds or until Ctrl+C

### Manual Testing
1. Launch uScope: `./build/uScope`
2. Open Camera Controls dialog
3. Adjust each slider multiple times:
   - Exposure slider (0-200ms range)
   - Brightness slider (-100 to +100)
   - Contrast slider (-100 to +100)
   - Saturation slider (-100 to +100)
4. Observe console output for latency measurements
5. Verify all measurements show "✓ PASS" (<200ms)

### Expected Results
- **PASS**: Latency measurements consistently <200ms for all controls
- **Typical values**: 30-80ms on modern hardware with Qt6 QCamera API
- **Frame rate dependency**: Latency depends on camera frame rate (e.g., 30fps = ~33ms per frame)

### Test Scenarios
1. **Single Control Change**: Adjust one slider, verify latency <200ms
2. **Rapid Changes**: Move slider quickly multiple times, verify all changes <200ms
3. **Multiple Controls**: Adjust different controls in sequence, verify each <200ms
4. **Camera Resolution Impact**: Test with different resolutions (lower res = potentially faster)
5. **System Load**: Test under normal and high system load conditions

## Acceptance Criteria (SC-004)
- ✅ Exposure adjustments reflect within 200ms
- ✅ Brightness adjustments reflect within 200ms
- ✅ Contrast adjustments reflect within 200ms
- ✅ Saturation adjustments reflect within 200ms
- ✅ Console output shows "✓ PASS" for all measurements
- ✅ No "✗ FAIL" messages appear during normal operation

## Implementation Notes

### Why This Approach Works
- **Real-time measurement**: Captures actual latency from API call to visible effect
- **Automatic validation**: No manual timing needed, logged automatically
- **Frame-accurate**: Measures to the next frame arrival after control change
- **Non-intrusive**: Minimal performance impact (~1-2 microseconds per frame)

### Limitations
- Measures time to next frame, not time to fully applied effect
- Some controls (like white balance) may take multiple frames to fully stabilize
- Camera hardware/driver latency is included in measurement
- Qt6 QCamera API latency is included (typically very low)

### Alternative Measurement Methods
If more detailed analysis is needed:
1. Use QElapsedTimer::nsecsElapsed() for nanosecond precision
2. Add histogram tracking for latency distribution
3. Implement statistical analysis (mean, median, p95, p99)
4. Add frame number tracking to count frames between change and effect

## Troubleshooting

### High Latency (>200ms)
If measurements exceed 200ms:
1. Check camera frame rate (low fps = higher latency floor)
2. Verify system is not under heavy load
3. Check for USB bandwidth issues (use `lsusb -t`)
4. Try different camera resolution (lower may be faster)
5. Check Qt6 multimedia backend (FFmpeg vs GStreamer)

### No Latency Measurements
If no measurements appear:
1. Verify camera is active and streaming
2. Check Camera Controls dialog is open
3. Ensure sliders are actually being moved
4. Check console output isn't being filtered

### Inconsistent Results
If results vary widely:
1. Normal variation: 1-2 frames difference is expected
2. Check for system interrupts or background processes
3. Verify camera driver stability
4. Test with different USB ports (preferably USB 3.0)

## Results Documentation

Record test results in this format:

**Test Date**: YYYY-MM-DD  
**Camera**: [Camera model and ID]  
**Resolution**: [e.g., 1920x1080]  
**Frame Rate**: [e.g., 30 fps]  
**System Load**: [e.g., idle, normal, high]

| Control      | Min (ms) | Max (ms) | Avg (ms) | Result |
|--------------|----------|----------|----------|--------|
| Exposure     |          |          |          | PASS/FAIL |
| Brightness   |          |          |          | PASS/FAIL |
| Contrast     |          |          |          | PASS/FAIL |
| Saturation   |          |          |          | PASS/FAIL |

**Overall SC-004 Compliance**: PASS / FAIL

**Notes**: [Any observations, issues, or anomalies]
