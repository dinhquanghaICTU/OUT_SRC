# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/OUT_SRC_TRUNGKIEN_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/OUT_SRC_TRUNGKIEN_autogen.dir/ParseCache.txt"
  "OUT_SRC_TRUNGKIEN_autogen"
  )
endif()
