set(CPR_SEARCH_PATH_LIST
  ${CPR_HOME}
  $ENV{CPR_HOME}
  /usr/local
  /opt
  /usr
)

find_path(CPR_INCLUDE_DIR cpr/cpr.h
	HINTS ${CPR_SEARCH_PATH_LIST}
  PATH_SUFFIXES include
  DOC "Find the cpr includes"
)

find_library(CPR_LIBRARY NAMES cpr
  HINTS ${CPR_SEARCH_PATH_LIST}
  PATH_SUFFIXES lib
  DOC "Find the cpr library"
)

set(CPR_LIBRARIES ${CPR_LIBRARY})
set(CPR_INCLUDE_DIRS ${CPR_INCLUDE_DIR})
