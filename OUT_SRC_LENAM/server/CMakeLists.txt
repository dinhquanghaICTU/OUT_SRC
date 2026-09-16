cmake_minimum_required(VERSION 3.16)

project(OUT_SRC_LENAM_SERVER VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
# Auto-detect local Qt6 installation paths
if(EXISTS "/home/quanghaictu/Qt6Cross/qt6/host")
    list(APPEND CMAKE_PREFIX_PATH "/home/quanghaictu/Qt6Cross/qt6/host")
endif()
if(EXISTS "/home/quanghaictu/Qt6Cross/qt6/pi")
    list(APPEND CMAKE_PREFIX_PATH "/home/quanghaictu/Qt6Cross/qt6/pi")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Network Sql HttpServer)

qt_add_executable(OUT_SRC_LENAM-SERVER
    main.cpp
    api/ApiServer.h
    api/ApiServer.cpp
    database/Database.h
    database/Database.cpp
    mqtt/MqttDiscoveryService.h
    mqtt/MqttDiscoveryService.cpp
    system/HotspotManager.h
    system/HotspotManager.cpp
)

target_include_directories(OUT_SRC_LENAM-SERVER PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(OUT_SRC_LENAM-SERVER PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    Qt6::HttpServer
)

install(TARGETS OUT_SRC_LENAM-SERVER RUNTIME DESTINATION bin)
