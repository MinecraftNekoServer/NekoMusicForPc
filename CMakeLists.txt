cmake_minimum_required(VERSION 3.20)

project(NekoMusic
    VERSION 2026.429.13
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia Network SvgWidgets Sql Concurrent)

# For cross-compilation, disable AUTOMOC/AUTOUIC/AUTORCC and handle manually
# because Qt6 CMake hardcodes Windows .exe paths for moc/uic/rcc
if(CMAKE_CROSSCOMPILING)
    set(CMAKE_AUTOMOC OFF)
    set(CMAKE_AUTORCC OFF)
    set(CMAKE_AUTOUIC OFF)

    # Use Linux Qt tools for code generation
    if(DEFINED ENV{QT_HOST_ROOT})
        set(QT_HOST_MOC "$ENV{QT_HOST_ROOT}/libexec/moc")
        set(QT_HOST_RCC "$ENV{QT_HOST_ROOT}/libexec/rcc")
        set(QT_HOST_UIC "$ENV{QT_HOST_ROOT}/libexec/uic")
    else()
        set(QT_HOST_MOC "/usr/lib/qt6/libexec/moc")
        set(QT_HOST_RCC "/usr/lib/qt6/libexec/rcc")
        set(QT_HOST_UIC "/usr/lib/qt6/libexec/uic")
    endif()

    # Manually run moc on headers that contain Q_OBJECT
    set(MOC_OUTPUT_DIR "${CMAKE_BINARY_DIR}/moc")
    file(MAKE_DIRECTORY ${MOC_OUTPUT_DIR})

    # Find all headers with Q_OBJECT macro
    file(GLOB_RECURSE ALL_HEADERS "${CMAKE_SOURCE_DIR}/src/*.h")
    foreach(header ${ALL_HEADERS})
        file(READ "${header}" header_content)
        if(header_content MATCHES "Q_OBJECT")
            file(RELATIVE_PATH rel_header "${CMAKE_SOURCE_DIR}" "${header}")
            get_filename_component(name "${header}" NAME_WE)
            get_filename_component(subdir "${header}" DIRECTORY)
            file(RELATIVE_PATH rel_subdir "${CMAKE_SOURCE_DIR}/src" "${subdir}")
            set(moc_out "${MOC_OUTPUT_DIR}/${rel_subdir}/moc_${name}.cpp")
            get_filename_component(moc_dir "${moc_out}" DIRECTORY)
            file(MAKE_DIRECTORY "${moc_dir}")
            add_custom_command(
                OUTPUT "${moc_out}"
                COMMAND "${QT_HOST_MOC}" "${header}" -o "${moc_out}"
                DEPENDS "${header}"
                COMMENT "MOC ${rel_header}"
            )
            list(APPEND MOC_SOURCES "${moc_out}")
        endif()
    endforeach()

    # Manually compile resources
    foreach(qrc ${RESOURCES})
        get_filename_component(name "${qrc}" NAME_WE)
        set(rcc_out "${CMAKE_BINARY_DIR}/qrc_${name}.cpp")
        add_custom_command(
            OUTPUT "${rcc_out}"
            COMMAND "${QT_HOST_RCC}" "${CMAKE_SOURCE_DIR}/${qrc}" -o "${rcc_out}"
            DEPENDS "${CMAKE_SOURCE_DIR}/${qrc}"
            COMMENT "RCC ${qrc}"
        )
        list(APPEND RCC_SOURCES "${rcc_out}")
    endforeach()

    # Add generated sources to the target
    set(GENERATED_SOURCES ${MOC_SOURCES} ${RCC_SOURCES})
else()
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    set(GENERATED_SOURCES "")
endif()

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
    src/ui/toast.cpp
    src/ui/recentpage.cpp
    src/ui/logindialog.cpp
    src/ui/forgotpassworddialog.cpp
    src/ui/musiclistpage.cpp
    src/ui/uploadpage.cpp
    src/ui/playerpage.cpp
    src/ui/desktoplrc.cpp
    src/ui/favoritespage.cpp
    src/core/playerengine.cpp
    src/core/apiclient.cpp
    src/core/i18n.cpp
    src/core/covercache.cpp
    src/core/usermanager.cpp
    src/core/musicdownloader.cpp
    src/core/playlistdb.cpp
    src/core/playlistmanager.cpp
    src/core/updatechecker.cpp
    src/ui/playlistlistitem.cpp
    src/ui/playlistdetailpage.cpp
    src/ui/addtoplaylistdialog.cpp
    src/ui/playlistpanel.cpp
    src/ui/updatedialog.cpp
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
    src/ui/recentpage.h
    src/ui/logindialog.h
    src/ui/forgotpassworddialog.h
    src/ui/musiclistpage.h
    src/ui/uploadpage.h
    src/ui/playerpage.h
    src/ui/desktoplrc.h
    src/ui/favoritespage.h
    src/core/playerengine.h
    src/core/apiclient.h
    src/core/i18n.h
    src/core/covercache.h
    src/core/usermanager.h
    src/theme/theme.h
    src/core/musicinfo.h
    src/core/playlistdb.h
    src/core/playlistmanager.h
    src/core/updatechecker.h
    src/ui/playlistlistitem.h
    src/ui/playlistdetailpage.h
    src/ui/addtoplaylistdialog.h
    src/ui/playlistpanel.h
    src/ui/updatedialog.h
    src/ui/searchpage.h
)

# Resources
set(RESOURCES
    src/resources/resources.qrc
)

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${RESOURCES} ${APP_ICON_RESOURCE_WINDOWS} ${GENERATED_SOURCES})
elseif(APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${RESOURCES} ${APP_ICON_RESOURCE_MACOS} ${GENERATED_SOURCES})
    set_source_files_properties(${APP_ICON_RESOURCE_MACOS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
else()
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCES} ${GENERATED_SOURCES})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Network
    Qt6::SvgWidgets
    Qt6::Sql
    Qt6::Concurrent
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Generate version header from CMake project version
configure_file(
    ${CMAKE_SOURCE_DIR}/src/core/version.h.in
    ${CMAKE_BINARY_DIR}/generated/version.h
)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/generated)

# Install - platform-specific rules
if(LINUX)
    # Linux: install to /opt/Neko云音乐 (matches auto-update code)
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION /opt/Neko云音乐
    )
    
    # Desktop entry file
    install(FILES "${CMAKE_SOURCE_DIR}/packaging/nekomusic.desktop"
        DESTINATION share/applications
    )
    
    # Symlink in /usr/local/bin for terminal access
    install(CODE "
        file(MAKE_DIRECTORY \"\$ENV{DESTDIR}/usr/local/bin\")
        execute_process(
            COMMAND \${CMAKE_COMMAND} -E create_symlink
            \"/opt/Neko云音乐/NekoMusic\"
            \"\$ENV{DESTDIR}/usr/local/bin/nekomusic\"
        )
    ")
    
    # Icon theme (hicolor)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/packaging/icons/hicolor/"
        DESTINATION share/icons/hicolor
        PATTERN ".*" EXCLUDE
    )
elseif(WIN32)
    # Windows: install to root (NSIS handles packaging)
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION .
    )
else()
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        BUNDLE DESTINATION .
    )
endif()

# CPack configuration
set(CPACK_PACKAGE_NAME "Neko云音乐")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION}${PROJECT_VERSION_SUFFIX})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "梦幻网络CN免费云音乐平台的官方客户端")
set(CPACK_PACKAGE_VENDOR "FantasyNetwork")
set(CPACK_PACKAGE_CONTACT "support@cnmsb.xin")
set(CPACK_PACKAGE_FILE_NAME "Neko云音乐-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")

if(LINUX)
    # Debian package settings
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_NAME "nekomusic")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "FantasyNetwork <support@cnmsb.xin>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "multimedia")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6, libqt6gui6, libqt6widgets6, libqt6multimedia6, libqt6network6, libqt6sql6")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_DEBIAN_PACKAGE_POSTINST "${CMAKE_SOURCE_DIR}/packaging/debian/postinst")
    set(CPACK_DEBIAN_PACKAGE_POSTRM "${CMAKE_SOURCE_DIR}/packaging/debian/postrm")
    
    # Output filename format matching version.json URL pattern
    set(CPACK_PACKAGE_FILE_NAME "Neko云音乐-${CPACK_PACKAGE_VERSION}-linux-amd64")
    
elseif(WIN32)
    # Windows uses NSIS (external script, not CPack)
    # CPack not used for Windows - build_windows.ps1 handles NSIS directly

elseif(APPLE)
    set(CPACK_GENERATOR "productbuild")
    set(CPACK_PACKAGE_FILE_NAME "Neko云音乐-${CPACK_PACKAGE_VERSION}-macos-${CMAKE_SYSTEM_PROCESSOR}")
endif()

include(CPack)

# Tests
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
