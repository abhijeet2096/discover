if (FWUPD_INCLUDE_DIR1 AND FWUPD_LIBRARIES)

  find_library (FWUPD_LIBRARIES
    NAMES fwupd libfwupd
  )

  find_path (FWUPD_INCLUDE_DIR1
    NAMES fwupd.h
    PATH_SUFFIXES fwupd-1
  )

  include (FindPackageHandleStandardArgs)
  find_package_handle_standard_args (FWUPD DEFAULT_MSG FWUPD_LIBRARIES FWUPD_INCLUDE_DIR1)

endif (FWUPD_INCLUDE_DIR1 AND FWUPD_LIBRARIES)

mark_as_advanced(FWUPD_INCLUDE_DIR1 FWUPD_LIBRARIES)

find_package(PkgConfig)



if (FWUPD_FOUND)
   find_package(GLib)
   add_library(FWUPD SHARED IMPORTED)
   set_target_properties(FWUPD PROPERTIES
       INTERFACE_INCLUDE_DIRECTORIES ${FWUPD_INCLUDE_DIR1}
       IMPORTED_LOCATION ${FWUPD_LIB}
       INTERFACE_COMPILE_OPTIONS ${Soup_PKG_CFLAGS_OTHER})
endif()
