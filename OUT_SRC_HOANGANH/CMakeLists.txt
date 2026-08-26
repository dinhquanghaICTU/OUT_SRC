cmake_minimum_required(VERSION 3.16)

project(OUT_SRC_HOANGANH VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Auto-detect local Qt6 installation paths
if(EXISTS "/home/quanghaictu/Qt6Cross/qt6/host")
    list(APPEND CMAKE_PREFIX_PATH "/home/quanghaictu/Qt6Cross/qt6/host")
endif()
if(EXISTS "/home/quanghaictu/Qt6Cross/qt6/pi")
    list(APPEND CMAKE_PREFIX_PATH "/home/quanghaictu/Qt6Cross/qt6/pi")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql HttpServer Charts)

qt_add_executable(OUT_SRC_HOANGANH
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    VirtualKeyboard.cpp
    VirtualKeyboard.h
    EmbeddedServer.cpp
    EmbeddedServer.h
    server/api/ApiServer.cpp
    server/api/ApiServer.h
    server/database/Database.cpp
    server/database/Database.h
    server/mqtt/MqttDiscoveryService.cpp
    server/mqtt/MqttDiscoveryService.h
    server/system/HotspotManager.cpp
    server/system/HotspotManager.h
)

target_include_directories(OUT_SRC_HOANGANH PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/server
)

target_link_libraries(OUT_SRC_HOANGANH PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::HttpServer
    Qt6::Charts
)
include(GNUInstallDirs)
install(TARGETS OUT_SRC_HOANGANH
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
