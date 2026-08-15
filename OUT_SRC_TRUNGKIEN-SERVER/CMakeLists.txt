cmake_minimum_required(VERSION 3.16)

project(OUT_SRC_TRUNGKIEN_SERVER VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Sql HttpServer)

qt_add_executable(OUT_SRC_TRUNGKIEN-SERVER
    main.cpp
    api/ApiServer.h
    api/ApiServer.cpp
    database/Database.h
    database/Database.cpp
    mqtt/MqttDiscoveryService.h
    mqtt/MqttDiscoveryService.cpp
)

target_include_directories(OUT_SRC_TRUNGKIEN-SERVER PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(OUT_SRC_TRUNGKIEN-SERVER PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    Qt6::HttpServer
)

install(TARGETS OUT_SRC_TRUNGKIEN-SERVER RUNTIME DESTINATION bin)
