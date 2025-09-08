# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\InterfacesAndDevicesOfPC_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\InterfacesAndDevicesOfPC_autogen.dir\\ParseCache.txt"
  "InterfacesAndDevicesOfPC_autogen"
  )
endif()
