# FindXapian.cmake - Find the Xapian full-text search library
#
# Uses xapian-config if available, otherwise falls back to pkg-config.
#
# This module defines:
#   XAPIAN_FOUND       - True if Xapian was found
#   XAPIAN_INCLUDE_DIRS - Xapian include directories
#   XAPIAN_LIBRARIES   - Xapian libraries to link
#
# And the imported target:
#   Xapian::Xapian

find_program(XAPIAN_CONFIG_EXECUTABLE NAMES xapian-config)

if(XAPIAN_CONFIG_EXECUTABLE)
    execute_process(COMMAND ${XAPIAN_CONFIG_EXECUTABLE} --cxxflags
        OUTPUT_VARIABLE XAPIAN_CXXFLAGS_RAW
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    execute_process(COMMAND ${XAPIAN_CONFIG_EXECUTABLE} --libs
        OUTPUT_VARIABLE XAPIAN_LIBS_RAW
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    execute_process(COMMAND ${XAPIAN_CONFIG_EXECUTABLE} --version
        OUTPUT_VARIABLE XAPIAN_VERSION_RAW
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)

    # Parse include dirs from -I flags
    string(REGEX MATCHALL "-I[^ ]+" XAPIAN_INCLUDE_FLAGS "${XAPIAN_CXXFLAGS_RAW}")
    string(REGEX REPLACE "-I" "" XAPIAN_INCLUDE_DIRS "${XAPIAN_INCLUDE_FLAGS}")

    # Parse library dirs and libs from -L and -l flags
    string(REGEX MATCHALL "-L[^ ]+" XAPIAN_LIBDIR_FLAGS "${XAPIAN_LIBS_RAW}")
    string(REGEX REPLACE "-L" "" XAPIAN_LIBRARY_DIRS "${XAPIAN_LIBDIR_FLAGS}")

    string(REGEX MATCHALL "-lxapian[^ ]*" XAPIAN_LIB_FLAGS "${XAPIAN_LIBS_RAW}")
    string(REGEX REPLACE "-l" "" XAPIAN_LIB_NAMES "${XAPIAN_LIB_FLAGS}")

    # Find full library paths
    set(XAPIAN_LIBRARIES "")
    foreach(_libdir ${XAPIAN_LIBRARY_DIRS})
        foreach(_libname ${XAPIAN_LIB_NAMES})
            find_library(_found_lib NAMES ${_libname} PATHS ${_libdir} NO_DEFAULT_PATH)
            if(_found_lib)
                list(APPEND XAPIAN_LIBRARIES ${_found_lib})
                unset(_found_lib CACHE)
            endif()
        endforeach()
    endforeach()

    # Fallback: just use the flags directly
    if(NOT XAPIAN_LIBRARIES)
        set(XAPIAN_LIBRARIES "-L${XAPIAN_LIBRARY_DIRS}" "-lxapian")
    endif()

    set(XAPIAN_VERSION ${XAPIAN_VERSION_RAW})
else()
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(PC_XAPIAN QUIET xapian)
    endif()

    find_path(XAPIAN_INCLUDE_DIRS
        NAMES xapian.h
        HINTS ${PC_XAPIAN_INCLUDE_DIRS}
        PATH_SUFFIXES xapian)

    find_library(XAPIAN_LIBRARIES
        NAMES xapian
        HINTS ${PC_XAPIAN_LIBRARY_DIRS})

    set(XAPIAN_VERSION ${PC_XAPIAN_VERSION})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Xapian
    REQUIRED_VARS XAPIAN_LIBRARIES XAPIAN_INCLUDE_DIRS
    VERSION_VAR XAPIAN_VERSION)

if(XAPIAN_FOUND AND NOT TARGET Xapian::Xapian)
    add_library(Xapian::Xapian UNKNOWN IMPORTED)
    set_target_properties(Xapian::Xapian PROPERTIES
        IMPORTED_LOCATION "${XAPIAN_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${XAPIAN_INCLUDE_DIRS}")
endif()

mark_as_advanced(XAPIAN_INCLUDE_DIRS XAPIAN_LIBRARIES XAPIAN_CONFIG_EXECUTABLE)
