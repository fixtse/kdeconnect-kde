kdeconnect_add_plugin(kdeconnect_sendnotifications)

target_sources(kdeconnect_sendnotifications PRIVATE
    sendnotificationsplugin.cpp
    notificationslistener.cpp
    notifyingapplication.cpp
)

add_subdirectory(pw_libnotificationmanager)

target_link_libraries(kdeconnect_sendnotifications
    kdeconnectcore
    KF6::I18n
    Qt::Gui
    KF6::IconThemes
    KF6::ConfigCore
)


if(WIN32)
target_sources(kdeconnect_sendnotifications PRIVATE windowsnotificationslistener.cpp)
target_link_libraries(kdeconnect_sendnotifications runtimeobject windowsapp)
else()
target_sources(kdeconnect_sendnotifications PRIVATE dbusnotificationslistener.cpp notificationimagehint.cpp)
target_link_libraries(kdeconnect_sendnotifications PkgConfig::DBus)
endif()

install(FILES "kdeconnect_sendnotifications_config.qml" DESTINATION ${KDE_INSTALL_DATADIR}/kdeconnect)

if(BUILD_TESTING AND NOT WIN32)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    ecm_add_test(notificationimagetest.cpp notificationslistener.cpp notifyingapplication.cpp
        pw_libnotificationmanager/notification.cpp notificationimagehint.cpp
        TEST_NAME notificationimagetest
        LINK_LIBRARIES kdeconnectcore Qt::Gui Qt::Test KF6::ConfigCore KF6::Service PkgConfig::DBus)
endif()
