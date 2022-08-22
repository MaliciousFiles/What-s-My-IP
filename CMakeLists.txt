cmake_minimum_required(VERSION 3.23)
project(WhatsMyIP)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.3.1/msvc2019_64")
elseif (APPLE)
    set(CMAKE_PREFIX_PATH "~/Qt/6.3.1/macos")
else()
#    set(CMAKE_PREFIX_PATH "C:/Qt/6.3.1/msvc2019_64")
endif()

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Network
        REQUIRED)

if (WIN32)
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/resources/whatsmyip.rc")
    qt_add_executable(${PROJECT_NAME} WIN32 resources/resources.qrc main.cpp mainwindow.h mainwindow.cpp ${app_icon_resource_windows})
elseif (APPLE)
    set(MACOSX_BUNDLE_ICON_FILE icon.icns)

    set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.icns")
    set_source_files_properties(${app_icon_macos} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")

    qt_add_executable(${PROJECT_NAME} MACOSX_BUNDLE resources/resources.qrc main.cpp mainwindow.h mainwindow.cpp ${app_icon_macos})
else()
    qt_add_executable(${PROJECT_NAME} resources/resources.qrc main.cpp mainwindow.h mainwindow.cpp)
endif()

#add_executable(${PROJECT_NAME} resources/resources.qrc main.cpp mainwindow.h mainwindow.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Network
        )

if (WIN32)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
