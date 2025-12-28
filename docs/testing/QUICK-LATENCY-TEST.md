# Quick Testing Guide for Latency Measurement

## Enable and Test

```bash
# 1. Enable latency measurement and rebuild
cd /home/matej/uScope/build
cmake .. -DENABLE_LATENCY_MEASUREMENT=ON
make -j$(nproc)

# 2. Run the test script
cd /home/matej/uScope
./test_latency.sh

# 3. In the application:
#    - Open Camera Controls dialog
#    - Move each slider multiple times
#    - Observe console output showing latency measurements

# 4. Verify all measurements show "✓ PASS" (<200ms)
```

## Disable for Production

```bash
# 1. Rebuild without the flag
cd /home/matej/uScope/build
cmake ..
make -j$(nproc)

# 2. Verify no latency output appears when running
./uScope
```

## Expected Output (When Enabled)

```
CameraService::onVideoFrameChanged - control change latency: exposure=100 took 45ms ✓ PASS
CameraService::onVideoFrameChanged - control change latency: brightness=50 took 67ms ✓ PASS
CameraService::onVideoFrameChanged - control change latency: contrast=-20 took 32ms ✓ PASS
CameraService::onVideoFrameChanged - control change latency: saturation=30 took 58ms ✓ PASS
```

## CI/CD Integration

For automated testing in CI:
```yaml
# .github/workflows/test.yml example
- name: Build with latency measurement
  run: |
    cmake -B build -DENABLE_LATENCY_MEASUREMENT=ON
    cmake --build build -j

- name: Run latency tests
  run: |
    timeout 60 ./build/uScope &
    # Automated UI interaction here
    # Parse output for PASS/FAIL
```
