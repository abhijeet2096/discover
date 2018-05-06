add_subdirectory(tests)

include_directories(${FWUPD_INCLUDE_DIRS})

set(fwupd-backend_SRCS
    FwupdResource.cpp
    FwupdBackend.cpp
    FwupdReviewsBackend.cpp
    FwupdTransaction.cpp
    FwupdSourcesBackend.cpp
)

add_library(fwupd-backend MODULE ${fwupd-backend_SRCS})
target_link_libraries(fwupd-backend Qt5::Core Qt5::Widgets KF5::CoreAddons KF5::ConfigCore Discover::Common ${FWUPD_LIBRARIES} )

install(TARGETS fwupd-backend DESTINATION ${PLUGIN_INSTALL_DIR}/discover)
install(FILES fwupd-backend-categories.xml DESTINATION ${DATA_INSTALL_DIR}/libdiscover/categories)

add_library(FwupdNotifier MODULE FwupdNotifier.cpp )
target_link_libraries(FwupdNotifier Discover::Notifiers ${FWUPD_LIBRARIES} )
set_target_properties(FwupdNotifier PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}/plasma-discover)

install(TARGETS FwupdNotifier DESTINATION ${PLUGIN_INSTALL_DIR}/discover-notifier)
