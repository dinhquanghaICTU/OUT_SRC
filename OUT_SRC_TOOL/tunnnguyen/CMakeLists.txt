cmake_minimum_required(VERSION 3.5)

project(tunnnguyen VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        
        src/auth/LicenseManager.cpp
        src/auth/LicenseManager.h
        src/auth/HwidHelper.cpp
        src/auth/HwidHelper.h
        src/core/ChromeLauncher.cpp
        src/core/ChromeLauncher.h
        src/core/SyncManager.cpp
        src/core/SyncManager.h
        src/cdp/CdpClient.cpp
        src/cdp/CdpClient.h
        src/ui/LoginDialog.cpp
        src/ui/LoginDialog.h
        src/ui/CustomMessageBox.cpp
        src/ui/CustomMessageBox.h
        src/core/ProxyConfig.h
        src/core/ProxyChecker.cpp
        src/core/ProxyChecker.h
        src/core/ChromeProfileItem.h
        src/ui/ProxyInputWidget.cpp
        src/ui/ProxyInputWidget.h
        src/ui/CreateProfileDialog.cpp
        src/ui/CreateProfileDialog.h
        src/ui/EditProfileDialog.cpp
        src/ui/EditProfileDialog.h
        src/ui/QuickProxyDialog.cpp
        src/ui/QuickProxyDialog.h
        src/ui/ImportProfilesDialog.cpp
        src/ui/ImportProfilesDialog.h
        src/pages/dashboard/DashboardPage.cpp
        src/pages/dashboard/DashboardPage.h
        src/pages/account_parser/AccountParserPage.cpp
        src/pages/account_parser/AccountParserPage.h
        src/pages/chrome_profiles/ChromeProfilesPage.cpp
        src/pages/chrome_profiles/ChromeProfilesPage.h
        src/pages/proxy_pool/ProxyPoolPage.cpp
        src/pages/proxy_pool/ProxyPoolPage.h
        src/pages/task_runner/TaskRunnerPage.cpp
        src/pages/task_runner/TaskRunnerPage.h
        src/pages/task_runner/FreeFireLogic.cpp
        src/pages/task_runner/FreeFireLogic.h
        src/pages/license_key/LicenseKeyPage.cpp
        src/pages/license_key/LicenseKeyPage.h
        src/pages/cookie_manager/CookieManagerPage.cpp
        src/pages/cookie_manager/CookieManagerPage.h
        src/pages/reports_logs/ReportsLogsPage.cpp
        src/pages/reports_logs/ReportsLogsPage.h
        src/pages/settings/SettingsPage.cpp
        src/pages/settings/SettingsPage.h
        resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(tunnnguyen
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET tunnnguyen APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(tunnnguyen SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(tunnnguyen
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(tunnnguyen PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pages
)

target_link_libraries(tunnnguyen PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

set_target_properties(tunnnguyen PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS tunnnguyen
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(tunnnguyen)
endif()
