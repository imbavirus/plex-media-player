find_library(FOUNDATION Foundation)
find_library(APPKIT AppKit)
find_library(IOKIT IOKit)
find_library(COCOA Cocoa)
find_Library(CARBON Carbon)
find_library(SECURITY Security)

set(OS_LIBS ${FOUNDATION} ${APPKIT} ${IOKIT} ${COCOA} ${SECURITY} ${CARBON} spmediakeytap)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.9")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.9")
set(WARNINGS "-Wall")

set(HAVE_UPDATER 1)
find_program(UPDATER_PATH updater HINTS ${CMAKE_FIND_ROOT_PATH}/update_installer/ NO_DEFAULT_PATH)
if(${UPDATER_PATH} MATCHES "UPDATER_PATH-NOTFOUND")
  set(HAVE_UPDATER 0)
  message(STATUS "will build without the updater")
endif(${UPDATER_PATH} MATCHES "UPDATER_PATH-NOTFOUND")

set(INSTALL_BIN_DIR .)
set(INSTALL_RESOURCE_DIR Contents/Resources)