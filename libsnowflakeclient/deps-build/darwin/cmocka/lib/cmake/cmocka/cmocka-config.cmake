get_filename_component(CMOCKA_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

if (EXISTS "${CMOCKA_CMAKE_DIR}/CMakeCache.txt")
    # In build tree
    include(${CMOCKA_CMAKE_DIR}/cmocka-build-tree-settings.cmake)
else()
    set(CMOCKA_INCLUDE_DIR /tmp/cmocka/include)
endif()

set(CMOCKA_LIBRARY /tmp/cmocka/lib/libcmocka.dylib)
set(CMOCKA_LIBRARIES /tmp/cmocka/lib/libcmocka.dylib)
