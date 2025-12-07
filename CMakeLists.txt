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
    Tile.cpp
    Tile.h
    TileLabel.cpp
    TileLabel.h
    TileButton.cpp
    TileButton.h
    TileCombo.cpp
    TileCombo.h
    TileSlider.cpp
    TileSlider.h
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
    # Controllers
    controllers/CameraController.cpp
    controllers/CameraController.h
    # UI
    ui/CameraControlsPanel.cpp
    ui/CameraControlsPanel.h
)

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
)

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
