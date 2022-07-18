# Input variables:
#   ED247_LIB_DIR                 Default folder that contain ED247 library
#   TEST_NAME
#   WORKING_DIRECTORY             Where test executables are launched
#   TEST_NB_ACTORS                1 or 2
#   TEST_MULTICAST_INTERFACE_IP
message("Running test: ${TEST_NAME}")

# Allows to validate another library build (only for functional tests)
if (DEFINED ENV{ED247_LIB_DIR})
  set(ED247_LIB_DIR $ENV{ED247_LIB_DIR})
  message("Using ED247 library in '${ED247_LIB_DIR}'")
endif()

if (NOT DEFINED ED247_LIB_DIR OR NOT EXISTS ${ED247_LIB_DIR})
  message(FATAL_ERROR "Please set ED247_LIB_DIR to a valid path!")
endif()

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  set(ENV{PATH} "${ED247_LIB_DIR};$ENV{PATH}")
else()
  set(ENV{LD_LIBRARY_PATH} "${ED247_LIB_DIR};$ENV{LD_LIBRARY_PATH}")
endif()


if("${ACTOR_TWO_EXE}" STREQUAL "")
  execute_process(
    COMMAND ${CMAKE_COMMAND}
    -DWORKING_DIRECTORY=${WORKING_DIRECTORY}
    -DCMD_EXE=${ACTOR_ONE_EXE}
    -DCMD_ARG1=${WORKING_DIRECTORY}/config
    -DCMD_ARG2=${TEST_MULTICAST_INTERFACE_IP}
    -P ${CMAKE_CURRENT_LIST_DIR}/execute_process_redirect.cmake
    WORKING_DIRECTORY ${WORKING_DIRECTORY}
    COMMAND_ECHO STDOUT
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
    )
  if(NOT TEST_RESULT EQUAL 0)
    if("${TEST_ERROR}" STREQUAL "")
      message(FATAL_ERROR "Unknown error")
    else()
      message(FATAL_ERROR "${TEST_ERROR}")
    endif()
  endif()

else()
  execute_process(
    COMMAND ${CMAKE_COMMAND}
    -DWORKING_DIRECTORY=${WORKING_DIRECTORY}
    -DCMD_EXE=${ACTOR_ONE_EXE}
    -DCMD_ARG1=${WORKING_DIRECTORY}/config
    -DCMD_ARG2=${TEST_MULTICAST_INTERFACE_IP}
    -P ${CMAKE_CURRENT_LIST_DIR}/execute_process_redirect.cmake
    COMMAND ${CMAKE_COMMAND}
    -DWORKING_DIRECTORY=${WORKING_DIRECTORY}
    -DCMD_EXE=${ACTOR_TWO_EXE}
    -DCMD_ARG1=${WORKING_DIRECTORY}/config
    -DCMD_ARG2=${TEST_MULTICAST_INTERFACE_IP}
    -P ${CMAKE_CURRENT_LIST_DIR}/execute_process_redirect.cmake
    WORKING_DIRECTORY ${WORKING_DIRECTORY}
    COMMAND_ECHO STDOUT
    ERROR_VARIABLE TEST_ERROR
    RESULT_VARIABLE TEST_RESULT
    )

  if(NOT TEST_RESULT EQUAL 0)
    if("${TEST_ERROR}" STREQUAL "")
      message(FATAL_ERROR "Unknown error")
    else()
      message(FATAL_ERROR "${TEST_ERROR}")
    endif()
  endif()

endif()
