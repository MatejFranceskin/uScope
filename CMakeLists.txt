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
)

# Resources
set(PROJECT_RESOURCES
    resources.qrc
)

# Executable
add_executable(uScope
    ${PROJECT_SOURCES}
    ${PROJECT_RESOURCES}
)

# Link Qt6 libraries
target_link_libraries(uScope PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Svg
)

# Platform-specific configuration
if(UNIX AND NOT APPLE)
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
    install(FILES images/uScope.ico
        DESTINATION share/pixmaps
        RENAME uscope.ico
    )
elseif(WIN32)
    # Windows configuration
    set_target_properties(uScope PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
elseif(APPLE)
    # macOS configuration
    set_target_properties(uScope PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/macos/Info.plist.in
    )
endif()
