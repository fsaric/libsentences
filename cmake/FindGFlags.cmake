# - Find gflags
# Find the gflags includes and library
#
#  GFLAGS_INCLUDE_DIR - where to gflags/gflags.h, etc.
#  GFLAGS_LIBRARY        - gflags library
#  GFLAGS_FOUND       - True if gflags found.


IF(GFLAGS_INCLUDE_DIR)
    SET(GFLAGS_FIND_QUIETLY TRUE)
ENDIF(GFLAGS_INCLUDE_DIR)

FIND_PATH(GFLAGS_INCLUDE_DIR gflags/gflags.h)

FIND_LIBRARY(GFLAGS_LIBRARY gflags)

# handle the QUIETLY and REQUIRED arguments and set ICONV_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GFLAGS DEFAULT_MSG GFLAGS_INCLUDE_DIR)

IF(GFLAGS_FOUND AND GFLAGS_LIBRARY)
    SET(GFLAGS_LIBRARY "${GFLAGS_LIBRARY}")
ELSE(GFLAGS_FOUND AND GFLAGS_LIBRARY)
    SET(GFLAGS_LIBRARY)
ENDIF(GFLAGS_FOUND AND GFLAGS_LIBRARY)

MARK_AS_ADVANCED(GFLAGS_LIBRARY GFLAGS_INCLUDE_DIR GFLAGS_LIBRARY)