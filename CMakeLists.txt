cmake_minimum_required(VERSION 3.21)

project(uScope VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Multimedia
    Svg
)

# Find OpenCV
if(IOS)
    # For iOS, use the xcframework built by the CI or local build
    # CI builds to: opencv/ios_build/opencv2.xcframework
    if(DEFINED ENV{OPENCV_FRAMEWORK})
        set(OpenCV_FRAMEWORK "$ENV{OPENCV_FRAMEWORK}")
        message(STATUS "Using OpenCV framework from environment: ${OpenCV_FRAMEWORK}")
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/opencv/ios_build/opencv2.xcframework")
        set(OpenCV_FRAMEWORK "${CMAKE_SOURCE_DIR}/opencv/ios_build/opencv2.xcframework")
        message(STATUS "Using OpenCV framework from opencv/ios_build: ${OpenCV_FRAMEWORK}")
    endif()
    
    # For iOS, we need to use the framework directly, not find_package
    # XCFramework contains the actual framework inside ios-arm64_armv7/
    if(EXISTS "${OpenCV_FRAMEWORK}")
        set(OpenCV_FOUND TRUE)
        # XCFramework will be linked automatically by Xcode when we add it
        # We just need to set the include path to the headers inside the xcframework
        set(OpenCV_INCLUDE_DIRS "${OpenCV_FRAMEWORK}/ios-arm64/opencv2.framework/Headers")
        # For linking, we need to tell CMake to use the framework
        set(OpenCV_LIBS "${OpenCV_FRAMEWORK}/ios-arm64/opencv2.framework")
    endif()
endif()

if(NOT IOS OR NOT OpenCV_FOUND)
    # Desktop platforms or if iOS framework not found, use find_package
    find_package(OpenCV REQUIRED COMPONENTS
        core
        imgproc
        videoio
        imgcodecs
    )
endif()

include_directories(${OpenCV_INCLUDE_DIRS})

# Find libgphoto2 for PTP camera support
# libgphoto2 allows access to external digital cameras via USB using PTP protocol
# Note: iOS does not support USB host mode
# Android supports it via USB host API (requires android.hardware.usb.host permission)
if(NOT IOS)
    if(WIN32)
        # On Windows, use MSYS2/MinGW-w64 pre-built binaries
        set(GPHOTO2_ROOT "${CMAKE_SOURCE_DIR}/thirdparty/libgphoto2-windows-x64")
        set(GPHOTO2_INCLUDE_DIRS "${GPHOTO2_ROOT}/include")
        set(GPHOTO2_LIBRARY_DIRS "${GPHOTO2_ROOT}/lib")
        
        # MSYS2 packages use .dll.a import libraries (for dynamic linking with DLLs)
        # Include libgphoto2 and its dependencies
        set(GPHOTO2_LIBRARIES 
            "${GPHOTO2_ROOT}/lib/libgphoto2.dll.a"
            "${GPHOTO2_ROOT}/lib/libgphoto2_port.dll.a"
            "${GPHOTO2_ROOT}/lib/libusb-1.0.dll.a"
            "${GPHOTO2_ROOT}/lib/libexif.dll.a"
            "${GPHOTO2_ROOT}/lib/libjpeg.dll.a"
            "${GPHOTO2_ROOT}/lib/libltdl.dll.a"
            ws2_32  # Windows sockets for network operations
            setupapi  # For USB device enumeration
        )
        
        # Check if the main libraries exist
        if(EXISTS "${GPHOTO2_ROOT}/lib/libgphoto2.dll.a" AND EXISTS "${GPHOTO2_ROOT}/lib/libgphoto2_port.dll.a")
            set(GPHOTO2_FOUND TRUE)
            include_directories(${GPHOTO2_INCLUDE_DIRS})
            message(STATUS "Found libgphoto2 at ${GPHOTO2_ROOT}")
        else()
            message(FATAL_ERROR "libgphoto2 libraries not found at ${GPHOTO2_ROOT}/lib - required for PTP camera support")
        endif()
    elseif(ANDROID)
        # On Android, use our custom NDK build
        set(GPHOTO2_ROOT "${CMAKE_SOURCE_DIR}/thirdparty/libgphoto2-android")
        set(GPHOTO2_INCLUDE_DIRS "${GPHOTO2_ROOT}/include")
        set(GPHOTO2_LIBRARY_DIRS "${GPHOTO2_ROOT}/lib")
        set(GPHOTO2_LIBRARIES 
            "${GPHOTO2_ROOT}/lib/libgphoto2.a"
            "${GPHOTO2_ROOT}/lib/libgphoto2_port.a"
            "${GPHOTO2_ROOT}/lib/libltdl.a"
        )
        
        # Check if the libraries exist
        if(EXISTS "${GPHOTO2_ROOT}/lib/libgphoto2.a" AND EXISTS "${GPHOTO2_ROOT}/lib/libgphoto2_port.a")
            set(GPHOTO2_FOUND TRUE)
            include_directories(${GPHOTO2_INCLUDE_DIRS})
            message(STATUS "Found libgphoto2 for Android at ${GPHOTO2_ROOT}")
        else()
            message(STATUS "libgphoto2 libraries not found for Android - PTP support disabled")
            set(GPHOTO2_FOUND FALSE)
        endif()
    else()
        # On Unix systems (Linux, macOS), use pkg-config
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GPHOTO2 REQUIRED libgphoto2)
        include_directories(${GPHOTO2_INCLUDE_DIRS})
    endif()
endif()

# Resources
# Generate platform-specific icons from SVG
# Modern ImageMagick (v7+) uses 'magick' command on all platforms
find_program(IMAGEMAGICK_MAGICK magick)
if(IMAGEMAGICK_MAGICK)
    set(CONVERT_COMMAND ${IMAGEMAGICK_MAGICK} convert)
else()
    # Fallback to older 'convert' command for ImageMagick v6
    find_program(IMAGEMAGICK_CONVERT convert)
    set(CONVERT_COMMAND ${IMAGEMAGICK_CONVERT})
endif()

set(ICO_FILE ${CMAKE_SOURCE_DIR}/images/uScope.ico)

if((IMAGEMAGICK_MAGICK OR IMAGEMAGICK_CONVERT) AND EXISTS ${CMAKE_SOURCE_DIR}/images/uScope.svg)
    # Generate .ico from SVG for Windows/Qt
    set(ICO_FILE ${CMAKE_CURRENT_BINARY_DIR}/uScope.ico)
    add_custom_command(
        OUTPUT ${ICO_FILE}
        COMMAND ${CONVERT_COMMAND} ${CMAKE_SOURCE_DIR}/images/uScope.svg 
                -define icon:auto-resize=256,128,96,64,48,32,16 
                ${ICO_FILE}
        DEPENDS ${CMAKE_SOURCE_DIR}/images/uScope.svg
        COMMENT "Generating uScope.ico from SVG"
    )
    add_custom_target(generate_ico DEPENDS ${ICO_FILE})
    
    # Configure resources.qrc with generated icon path
    configure_file(
        ${CMAKE_SOURCE_DIR}/resources.qrc.in 
        ${CMAKE_CURRENT_BINARY_DIR}/resources.qrc 
        @ONLY
    )
    set(PROJECT_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/resources.qrc)
else()
    message(STATUS "Using pre-generated uScope.ico (ImageMagick not found or SVG missing)")
    set(PROJECT_RESOURCES ${CMAKE_SOURCE_DIR}/resources.qrc)
endif()

# Project sources
set(PROJECT_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    VideoGraphicsScene.cpp
    VideoGraphicsScene.h
    ZoomableGraphicsView.cpp
    ZoomableGraphicsView.h
    Tile.cpp
    Tile.h
    TileLabel.cpp
    TileLabel.h
    TileButton.cpp
    TileButton.h
    TileDialog.cpp
    TileDialog.h
    # Models
    models/CameraProfile.cpp
    models/CameraProfile.h
    models/CapturedImage.cpp
    models/CapturedImage.h
    models/ZoomState.cpp
    models/ZoomState.h
    # Services
    services/CameraService.cpp
    services/CameraService.h
    services/CaptureThread.cpp
    services/CaptureThread.h
    # Controllers
    controllers/CameraController.cpp
    controllers/CameraController.h
    controllers/ZoomController.cpp
    controllers/ZoomController.h
    # UI
    ui/CameraControlsPanel.cpp
    ui/CameraControlsPanel.h
    ui/ZoomTile.cpp
    ui/ZoomTile.h
)

# Add PTP camera support for platforms with libgphoto2
if(NOT IOS)
    # Only add if GPHOTO2 was found (or Windows which builds it)
    if(GPHOTO2_FOUND OR WIN32)
        list(APPEND PROJECT_SOURCES
            models/PTPCamera.cpp
            models/PTPCamera.h
            services/PTPCameraService.cpp
            services/PTPCameraService.h
        )
        
        # Add Android USB helper for Android platform
        if(ANDROID)
            list(APPEND PROJECT_SOURCES
                services/AndroidUsbHelper.cpp
                services/AndroidUsbHelper.h
            )
        endif()
    endif()
endif()

# Create executable using Qt's helper (handles Android/iOS as shared libraries automatically)
qt_add_executable(uScope
    ${PROJECT_SOURCES}
    ${PROJECT_RESOURCES}
)

# Ensure icon is generated before building
if(TARGET generate_ico)
    add_dependencies(uScope generate_ico)
endif()

# Link Qt libraries
target_link_libraries(uScope PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Svg
    ${OpenCV_LIBS}
)

# Link libgphoto2 for platforms that support it
if(NOT IOS)
    if(GPHOTO2_FOUND OR WIN32)
        target_link_libraries(uScope PRIVATE ${GPHOTO2_LIBRARIES})
        if(NOT WIN32 AND NOT ANDROID)
            target_link_directories(uScope PRIVATE ${GPHOTO2_LIBRARY_DIRS})
            target_compile_options(uScope PRIVATE ${GPHOTO2_CFLAGS_OTHER})
        endif()
    endif()
endif()

# Platform-specific configuration
if(ANDROID)
    # Android configuration
    set_target_properties(uScope PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android"
        QT_ANDROID_TARGET_SDK_VERSION "30"
        QT_ANDROID_MIN_SDK_VERSION "23"
    )
    
    # Qt6 requires qt6_android_generate_deployment_settings (not qt_)
    qt6_android_generate_deployment_settings(uScope)
elseif(UNIX AND NOT APPLE)
    # Linux installation
    install(TARGETS uScope
        RUNTIME DESTINATION bin
    )
    install(FILES uScope.desktop
        DESTINATION share/applications
    )
    install(FILES images/uScope.svg
        DESTINATION share/icons/hicolor/scalable/apps
    )
elseif(WIN32)
    # Windows configuration
    set_target_properties(uScope PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    # Icon is embedded via resources.qrc
    
    # Install libgphoto2 DLLs to output directory
    if(GPHOTO2_FOUND)
        file(GLOB GPHOTO2_DLLS "${GPHOTO2_ROOT}/bin/*.dll")
        if(GPHOTO2_DLLS)
            # Copy DLLs to build directory for running from IDE
            foreach(DLL_FILE ${GPHOTO2_DLLS})
                add_custom_command(TARGET uScope POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${DLL_FILE}"
                    $<TARGET_FILE_DIR:uScope>
                )
            endforeach()
            
            # Install DLLs for deployment
            install(FILES ${GPHOTO2_DLLS} DESTINATION bin)
        endif()
        
        # Copy camera driver plugins (camlibs) and port drivers (iolibs)
        if(EXISTS "${GPHOTO2_ROOT}/lib/libgphoto2")
            add_custom_command(TARGET uScope POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${GPHOTO2_ROOT}/lib/libgphoto2"
                "$<TARGET_FILE_DIR:uScope>/libgphoto2"
            )
            install(DIRECTORY "${GPHOTO2_ROOT}/lib/libgphoto2" DESTINATION bin)
        endif()
        
        if(EXISTS "${GPHOTO2_ROOT}/lib/libgphoto2_port")
            add_custom_command(TARGET uScope POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${GPHOTO2_ROOT}/lib/libgphoto2_port"
                "$<TARGET_FILE_DIR:uScope>/libgphoto2_port"
            )
            install(DIRECTORY "${GPHOTO2_ROOT}/lib/libgphoto2_port" DESTINATION bin)
        endif()
    endif()
elseif(APPLE)
    # macOS configuration
    
    # Generate .icns from SVG for macOS
    if(IMAGEMAGICK_MAGICK OR IMAGEMAGICK_CONVERT)
        # Create iconset from SVG
        set(ICONSET_DIR ${CMAKE_CURRENT_BINARY_DIR}/uScope.iconset)
        file(MAKE_DIRECTORY ${ICONSET_DIR})
        
        # Generate all required icon sizes
        foreach(SIZE 16 32 128 256 512)
            math(EXPR SIZE2 "${SIZE} * 2")
            add_custom_command(
                OUTPUT ${ICONSET_DIR}/icon_${SIZE}x${SIZE}.png
                COMMAND ${CONVERT_COMMAND} ${CMAKE_SOURCE_DIR}/images/uScope.svg 
                        -resize ${SIZE}x${SIZE} ${ICONSET_DIR}/icon_${SIZE}x${SIZE}.png
                DEPENDS ${CMAKE_SOURCE_DIR}/images/uScope.svg
            )
            add_custom_command(
                OUTPUT ${ICONSET_DIR}/icon_${SIZE}x${SIZE}@2x.png
                COMMAND ${CONVERT_COMMAND} ${CMAKE_SOURCE_DIR}/images/uScope.svg 
                        -resize ${SIZE2}x${SIZE2} ${ICONSET_DIR}/icon_${SIZE}x${SIZE}@2x.png
                DEPENDS ${CMAKE_SOURCE_DIR}/images/uScope.svg
            )
            list(APPEND ICONSET_FILES ${ICONSET_DIR}/icon_${SIZE}x${SIZE}.png)
            list(APPEND ICONSET_FILES ${ICONSET_DIR}/icon_${SIZE}x${SIZE}@2x.png)
        endforeach()
        
        # Convert iconset to icns
        find_program(ICONUTIL iconutil)
        if(ICONUTIL)
            add_custom_command(
                OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/uScope.icns
                COMMAND ${ICONUTIL} -c icns ${ICONSET_DIR} -o ${CMAKE_CURRENT_BINARY_DIR}/uScope.icns
                DEPENDS ${ICONSET_FILES}
                COMMENT "Generating uScope.icns from SVG"
            )
            add_custom_target(generate_icns ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/uScope.icns)
            
            set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/uScope.icns PROPERTIES
                MACOSX_PACKAGE_LOCATION Resources
            )
            target_sources(uScope PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/uScope.icns)
        endif()
    endif()
    
    set_target_properties(uScope PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/macos/Info.plist.in
        MACOSX_BUNDLE_ICON_FILE uScope.icns
    )
endif()
