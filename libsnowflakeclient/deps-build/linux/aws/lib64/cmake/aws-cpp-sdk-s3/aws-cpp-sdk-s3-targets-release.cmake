#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "aws-cpp-sdk-s3" for configuration "Release"
set_property(TARGET aws-cpp-sdk-s3 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(aws-cpp-sdk-s3 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib64/libaws-cpp-sdk-s3.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS aws-cpp-sdk-s3 )
list(APPEND _IMPORT_CHECK_FILES_FOR_aws-cpp-sdk-s3 "${_IMPORT_PREFIX}/lib64/libaws-cpp-sdk-s3.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
