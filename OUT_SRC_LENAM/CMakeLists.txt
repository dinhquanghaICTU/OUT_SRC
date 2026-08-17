cmake_minimum_required(VERSION 3.16)

project(OUT_SRC_LENAM VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Network
    Sql
    HttpServer
    Charts
)

qt_add_executable(OUT_SRC_LENAM
    main.cpp
    VirtualKeyboard.h
    VirtualKeyboard.cpp
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

    config/AppConfig.h

    api/ApiClient.h
    api/ApiClient.cpp

    models/SensorReading.h
    models/Alert.h
    models/DeviceConfig.h
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

    ui/widgets/PressureCard.h
    ui/widgets/PressureCard.cpp
    ui/widgets/DistanceCard.h
    ui/widgets/DistanceCard.cpp

    resources/resources.qrc

)

target_include_directories(OUT_SRC_LENAM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/server
)

target_link_libraries(OUT_SRC_LENAM PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::HttpServer
    Qt6::Charts
)

install(TARGETS OUT_SRC_LENAM
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
