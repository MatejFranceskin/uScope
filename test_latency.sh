#!/bin/bash
# Test script for T103 - Verify <200ms latency for camera control adjustments

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
