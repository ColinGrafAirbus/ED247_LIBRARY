# Base tests library
add_library(libtests STATIC
  memhooks.cpp
  tests_tools.cpp
)
target_include_directories(libtests
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(libtests
  PUBLIC
    GTest::gtest
    $<$<PLATFORM_ID:Linux>:dl>
)

# Unitary tests lib statically linked to ED247
add_library(libtests_unitary INTERFACE)
target_link_libraries(libtests_unitary
  INTERFACE
    libtests
    ed247_objects
)

# Functionnal tests lib dynamically linked to ED247
add_library(libtests_functional INTERFACE)

target_link_libraries(libtests_functional
  INTERFACE
    Ed247::ed247
    libtests
    sync_entity
)
