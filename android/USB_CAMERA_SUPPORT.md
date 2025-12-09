# Android USB Camera Support via libgphoto2

## Overview

uScope on Android supports external USB cameras (DSLRs, mirrorless cameras) via libgphoto2 2.5.33+, which added Android support through file descriptor passing.

## Requirements

- Android device with **USB Host mode** (USB OTG)
- Android API level 23+ (Android 6.0+)
- Compatible PTP/USB camera connected via USB OTG adapter
- USB permissions granted by user

## How It Works

Unlike desktop platforms where libgphoto2 directly accesses USB devices through libusb, Android requires a different approach:

1. **Android USB Host API**: The app detects USB devices through Android's UsbManager
2. **Permission Request**: User must grant USB permission for the connected camera
3. **File Descriptor Passing**: Android provides a file descriptor for the USB device
4. **libgphoto2 Integration**: The FD is passed to libgphoto2 via `gp_port_usb_set_sys_device()`

## Current Implementation Status

### ✅ Completed
- libgphoto2 2.5.33 cross-compiled for Android arm64-v8a
- USB host permissions added to AndroidManifest.xml
- PTP camera service conditionally compiled for Android
- Static libraries linked into APK

### ⚠️ Partially Implemented
- Java/Kotlin USB permission handling layer (TODO)
- File descriptor passing to native code (TODO)
- USB device detection and enumeration (TODO)

### 📋 To Do
1. Create Java USB manager class in `android/src/`
2. Implement USB device detection and permission requests
3. Pass file descriptors from Java to C++ via JNI
4. Call `gp_port_usb_set_sys_device()` with received FD
5. Handle USB device attachment/detachment events
6. Test with various camera models

## Building

The Android build automatically cross-compiles libgphoto2:

```bash
# The CI workflow does this automatically
export ANDROID_NDK_ROOT=/path/to/ndk
./build_libgphoto2_android.sh
```

This creates static libraries at `thirdparty/libgphoto2-android/` which are linked into the APK.

## Testing

1. Connect a compatible PTP camera via USB OTG
2. Launch uScope
3. Grant USB permissions when prompted
4. Camera should appear in camera list

## Supported Cameras

libgphoto2 2.5.33 supports 2000+ camera models including:

- Canon EOS R3, R5, R6 Mark II, R7, R8, R10, R50
- Nikon Z6 III, Z8, Zf, Z30, Z fc
- Sony A1, A7R V, A7C II, A6700, ZV-E10 Mark II
- Fujifilm X-H2, X-H2S, X-S20, X-T5
- Panasonic GH6, GH7, G9 II, S5 II

See libgphoto2 documentation for full camera list.

## Limitations

- **USB Host Required**: Not all Android devices support USB host mode
- **Power Consumption**: USB cameras may drain battery faster
- **Permissions**: User must manually grant USB access for each camera
- **API Level**: Requires Android 6.0+ for reliable USB host support

## References

- [libgphoto2 2.5.33 Release](https://github.com/gphoto/libgphoto2/releases/tag/v2.5.33)
- [Android USB Host API](https://developer.android.com/guide/topics/connectivity/usb/host)
- [File Descriptor Passing in libgphoto2](https://github.com/gphoto/libgphoto2/blob/master/NEWS)
