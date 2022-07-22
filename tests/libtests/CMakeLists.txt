# Base tests library
add_library(libtests_objects STATIC memhooks.cpp)
target_include_directories(libtests_objects
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(libtests_objects
  PUBLIC
  GTest::gtest
)

# Unitary tests lib statically linked to ED247
add_library(libtests_unitary STATIC tests_tools.cpp)
target_link_libraries(libtests_unitary
  PUBLIC
    libtests_objects
    $<$<PLATFORM_ID:Linux>:dl>
    ed247_objects
)

# Functionnal tests lib dynamically linked to ED247
add_library(libtests_functional STATIC tests_tools.cpp)

target_link_libraries(libtests_functional
  PUBLIC
    Ed247::ed247
    libtests_objects
    sync_entity
    $<$<PLATFORM_ID:Linux>:dl>
)
