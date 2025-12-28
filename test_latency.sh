#!/bin/bash
# Test script for T103 - Verify <200ms latency for camera control adjustments

echo "========================================"
echo "Camera Control Latency Testing (T103)"
echo "========================================"
echo ""

# Check if latency measurement is enabled
if ! strings /home/matej/uScope/build/uScope | grep -q "control change latency"; then
    echo "WARNING: Latency measurement instrumentation is NOT enabled!"
    echo ""
    echo "To enable it, rebuild with:"
    echo "  cd /home/matej/uScope/build"
    echo "  cmake .. -DENABLE_LATENCY_MEASUREMENT=ON"
    echo "  make -j\$(nproc)"
    echo ""
    echo "Or add to CMakeLists.txt:"
    echo "  add_compile_definitions(ENABLE_LATENCY_MEASUREMENT)"
    echo ""
    exit 1
fi

echo "Latency measurement instrumentation: ENABLED ✓"
echo ""
echo "Starting uScope to test camera control latency (SC-004 requirement)..."
echo "Please perform the following actions:"
echo "1. Open Camera Controls dialog"
echo "2. Move the Exposure slider several times"
echo "3. Move the Brightness slider several times"
echo "4. Move the Contrast slider several times"
echo "5. Move the Saturation slider several times"
echo ""
echo "Watch the console output for latency measurements."
echo "All measurements should show <200ms for SC-004 compliance."
echo ""
echo "Press Ctrl+C when done testing."
echo ""

timeout 120 /home/matej/uScope/build/uScope 2>&1 | grep -E "control change latency|PASS|FAIL"
