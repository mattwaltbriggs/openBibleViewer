set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(openBibleViewer MACOSX_BUNDLE
    ${openBibleViewer_SRCS}
    ${MOCS}
    ${RSCS}
    ${UIS}
    ${OBVCore_SRCS}
)

target_link_libraries(
  openBibleViewer
    OBVCore
    RtfReader
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Xml
    Qt6::Network
    Qt6::Sql
    Qt6::WebEngineWidgets
    Qt6::PrintSupport
    Qt6::Qml
    Qt6::Core5Compat
    Xapian::Xapian
    ${ZLIB_LIBRARY}
    ${SW_LIBS}
)

set_target_properties(openBibleViewer PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Info.plist.in"
    MACOSX_BUNDLE_BUNDLE_NAME "openBibleViewer"
    MACOSX_BUNDLE_BUNDLE_VERSION "${OBV_VERSION_NUMBER}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${OBV_VERSION_NUMBER}"
)

install(
  TARGETS openBibleViewer
  BUNDLE DESTINATION .
  RUNTIME DESTINATION bin)

set(CPACK_GENERATOR "DragNDrop")
set(CPACK_DMG_FORMAT "UDBZ")
set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
set(CPACK_SYSTEM_NAME "macOS")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "openBibleViewer Project")
set(CPACK_PACKAGE_VENDOR "Paul Walger")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "9")
set(CPACK_PACKAGE_VERSION_PATCH "0")

include(CPack)
