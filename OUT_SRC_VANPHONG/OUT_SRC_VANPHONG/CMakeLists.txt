cmake_minimum_required(VERSION 3.16)

project(OUT_SRC_VANPHONG VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# Auto-detect local Qt6 installation paths
if(EXISTS "/home/quanghaictu/Qt6Cross/qt6/host")
    list(APPEND CMAKE_PREFIX_PATH "/home/quanghaictu/Qt6Cross/qt6/host")
endif()
if(EXISTS "/home/quanghaictu/Qt6Cross/qt6/pi")
    list(APPEND CMAKE_PREFIX_PATH "/home/quanghaictu/Qt6Cross/qt6/pi")
endif()

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Network
    Sql
    HttpServer
    Charts
)

qt_add_executable(OUT_SRC_VANPHONG
    main.cpp
    VirtualKeyboard.h
    VirtualKeyboard.cpp
    EmbeddedServer.h
    EmbeddedServer.cpp
    
    server/api/ApiServer.h
    server/api/ApiServer.cpp
    server/database/Database.h
    server/database/Database.cpp
    server/mqtt/MqttDiscoveryService.h
    server/mqtt/MqttDiscoveryService.cpp
    server/system/HotspotManager.h
    server/system/HotspotManager.cpp

    config/AppConfig.h

    api/ApiClient.h
    api/ApiClient.cpp

    models/SensorReading.h
    models/DeviceConfig.h
    models/Alert.h
    models/User.h

    services/AuthService.h
    services/AuthService.cpp
    services/SensorService.h
    services/SensorService.cpp

    ui/MainWindow.h
    ui/MainWindow.cpp
    ui/MainWindow.ui

    ui/dialogs/ErrorDialog.h
    ui/dialogs/ErrorDialog.cpp
    ui/dialogs/SelectOnlineDeviceDialog.h
    ui/dialogs/SelectOnlineDeviceDialog.cpp

    ui/widgets/PlantSoilVisualizerWidget.h
    ui/widgets/PlantSoilVisualizerWidget.cpp

    ui/pages/LoginPage.h
    ui/pages/LoginPage.cpp
    ui/pages/LoginPage.ui
    ui/pages/DashboardPage.h
    ui/pages/DashboardPage.cpp
    ui/pages/DashboardPage.ui
    ui/pages/DeviceManagementPage.h
    ui/pages/DeviceManagementPage.cpp
    ui/pages/HistoryPage.h
    ui/pages/HistoryPage.cpp
    ui/pages/HistoryPage.ui
    ui/pages/AlertPage.h
    ui/pages/AlertPage.cpp
    ui/pages/AlertPage.ui
    ui/pages/UserManagementPage.h
    ui/pages/UserManagementPage.cpp
    ui/pages/UserManagementPage.ui

    resources/resources.qrc
)

target_include_directories(OUT_SRC_VANPHONG PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/server
)

target_link_libraries(OUT_SRC_VANPHONG PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::HttpServer
    Qt6::Charts
)

include(GNUInstallDirs)
install(TARGETS OUT_SRC_VANPHONG
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
