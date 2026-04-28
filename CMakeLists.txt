cmake_minimum_required(VERSION 3.20)

project(NekoMusic
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia Network SvgWidgets Sql)

# Platform-specific settings
if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_SOURCE_DIR}/src/resources/windows/app.rc")
endif()

if(APPLE)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER com.nekomusic.app)
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
    set(APP_ICON_RESOURCE_MACOS "${CMAKE_SOURCE_DIR}/src/resources/macos/AppIcon.icns")
endif()

# Sources
set(SOURCES
    src/main.cpp
    src/ui/mainwindow.cpp
    src/ui/glasswidget.cpp
    src/ui/titlebar.cpp
    src/ui/sidebar.cpp
    src/ui/carousel.cpp
    src/ui/homepage.cpp
    src/ui/playlistcard.cpp
    src/ui/playerbar.cpp
    src/ui/svgicon.cpp
    src/ui/settingspage.cpp
    src/ui/favoritespage.cpp
    src/ui/recentpage.cpp
    src/ui/logindialog.cpp
    src/ui/forgotpassworddialog.cpp
    src/ui/musiclistpage.cpp
    src/ui/uploadpage.cpp
    src/ui/playerpage.cpp
    src/core/playerengine.cpp
    src/core/apiclient.cpp
    src/core/i18n.cpp
    src/core/covercache.cpp
    src/core/usermanager.cpp
    src/core/musicdownloader.cpp
    src/core/playlistdb.cpp
    src/core/playlistmanager.cpp
    src/ui/playlistlistitem.cpp
    src/ui/playlistdetailpage.cpp
    src/ui/addtoplaylistdialog.cpp
    src/ui/playlistpanel.cpp
    src/ui/searchpage.cpp
)

set(HEADERS
    src/ui/mainwindow.h
    src/ui/glasswidget.h
    src/ui/titlebar.h
    src/ui/sidebar.h
    src/ui/carousel.h
    src/ui/homepage.h
    src/ui/playlistcard.h
    src/ui/playerbar.h
    src/ui/svgicon.h
    src/ui/settingspage.h
    src/ui/favoritespage.h
    src/ui/recentpage.h
    src/ui/logindialog.h
    src/ui/forgotpassworddialog.h
    src/ui/musiclistpage.h
    src/ui/uploadpage.h
    src/ui/playerpage.h
    src/core/playerengine.h
    src/core/apiclient.h
    src/core/i18n.h
    src/core/covercache.h
    src/core/usermanager.h
    src/theme/theme.h
    src/core/musicinfo.h
    src/core/playlistdb.h
    src/core/playlistmanager.h
    src/ui/playlistlistitem.h
    src/ui/playlistdetailpage.h
    src/ui/addtoplaylistdialog.h
    src/ui/playlistpanel.h
    src/ui/searchpage.h
)

# Resources
set(RESOURCES
    src/resources/resources.qrc
)

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${RESOURCES} ${APP_ICON_RESOURCE_WINDOWS})
elseif(APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${RESOURCES} ${APP_ICON_RESOURCE_MACOS})
    set_source_files_properties(${APP_ICON_RESOURCE_MACOS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
else()
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Network
    Qt6::SvgWidgets
    Qt6::Sql
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Install
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Tests
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
