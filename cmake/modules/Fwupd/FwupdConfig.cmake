# FWUPD_INCLUDE_DIRS - The FWUPD include directories
# FWUPD_LIBRARIES - The libraries needed to use fwupd

set(FWUPD_LIBRARIES /usr/lib/libfwupd.so)
set(FWUPD_INCLUDE_DIRS /usr/include/fwupd-1)
#set(GLIB_INCLUDE_DIRS /usr/include/glib-2.0)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)

add_library(Fwupd SHARED IMPORTED )
include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})
add_definitions(${GLIB_CFLAGS_OTHER})
set(LIBS ${LIBS} ${GLIB_LIBRARIES})


set_target_properties(Fwupd PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES "Qt5::Core"
  IMPORTED_LOCATION "${FWUPD_LIBRARIES}"
  IMPORTED_SONAME "libfwupd.so"
  )


#target_include_directories(Fwupd PRIVATE ${GLIB_INCLUDE_DIRS})
#target_link_libraries(Fwupd INTERFACE ${GLIB_LDFLAGS})
#${GLIB_INCLUDE_DIR}
#find_package(PkgConfig REQUIRED)
#pkg_search_module(GLIB REQUIRED glib-2.0)
#set(GLIB_INCLUDE_DIRS /usr/include/glib-2.0)
#target_link_libraries(Fwupd ${GLIB_INCLUDE_DIRS})

set_property(TARGET Fwupd PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${FWUPD_INCLUDE_DIRS} )
