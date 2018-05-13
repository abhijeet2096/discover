add_subdirectory(tests)

include_directories(${FWUPD_INCLUDE_DIR1})

set(fwupd-backend_SRCS
    FwupdResource.cpp
    FwupdBackend.cpp
    FwupdReviewsBackend.cpp
    FwupdTransaction.cpp
    FwupdSourcesBackend.cpp
)




find_package(GIO)
find_package(Soup)
find_package(GLib)


MESSAGE( STATUS "FWUPD_INCLUDE_DIR variable is: " ${FWUPD_INCLUDE_DIR} )
MESSAGE( STATUS "FWUPD_INCLUDE_DIR1 variable is: " ${FWUPD_INCLUDE_DIR1} )
MESSAGE( STATUS "FWUPD_INCLUDE_DIR2 variable is: " ${FWUPD_INCLUDE_DIR2} )
MESSAGE( STATUS "FWUPD_LIBRARIES variable is: " ${FWUPD_LIBRARIES} )
MESSAGE( STATUS "GLIB_INCLUDE_DIR variable is: " ${GLib_INCLUDE_DIRS} )
MESSAGE( STATUS "GIO_INCLUDE_DIR variable is: " ${GIO_INCLUDE_DIRS} )
MESSAGE( STATUS "SOUP_INCLUDE_DIR variable is: " ${Soup_INCLUDE_DIR} )
MESSAGE( STATUS "GLIB_LIBRARIES variable is: " ${GLib_LIBRARY} )
MESSAGE( STATUS "GLIB_LIBRARIES Working variable is: " ${GLIB_LIBRARIES} )
MESSAGE( STATUS "GIO_LIBRARIES variable is: " ${GIO_LIBRARY} )
MESSAGE( STATUS "SOUP_LIBRARIES variable is: " ${Soup_LIBRARY} )
MESSAGE( STATUS "GLIB variable is: " ${GLib} )
MESSAGE( STATUS "GIO variable is: " ${GIO} )
MESSAGE( STATUS "SOUP variable is: " ${Soup} )
MESSAGE( STATUS "FWUPD variable is: " ${FWUPD_LIB} )
MESSAGE( STATUS "Soup CFLAGS variable is: " ${Soup_PKG_CFLAGS_OTHER} )
MESSAGE( STATUS "GIO CFLAGS variable is: " ${GIO_PKG_CFLAGS_OTHER} )
MESSAGE( STATUS "GLib CFLAGS variable is: " ${GLib_PKG_CFLAGS_OTHER} )
MESSAGE( STATUS "FWUPD_PKG variable is: " ${FWUPD_PKG} )
MESSAGE( STATUS "GLib_PKG variable is: " ${GIO_PKG} )

include_directories(${GIO_INCLUDE_DIRS} ${GLib_INCLUDE_DIRS} ${Soup_INCLUDE_DIR} )

add_library(fwupd-backend MODULE ${fwupd-backend_SRCS})
target_link_libraries(fwupd-backend Qt5::Core Qt5::Widgets KF5::CoreAddons KF5::ConfigCore Discover::Common ${Soup} ${GIO} ${GLib} ${FWUPD_LIBRARIES})

install(TARGETS fwupd-backend DESTINATION ${PLUGIN_INSTALL_DIR}/discover)
install(FILES fwupd-backend-categories.xml DESTINATION ${DATA_INSTALL_DIR}/libdiscover/categories)

add_library(FwupdNotifier MODULE FwupdNotifier.cpp )
target_link_libraries(FwupdNotifier Discover::Notifiers ${FWUPD_LIBRARIES} )
set_target_properties(FwupdNotifier PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}/plasma-discover)

install(TARGETS FwupdNotifier DESTINATION ${PLUGIN_INSTALL_DIR}/discover-notifier)


