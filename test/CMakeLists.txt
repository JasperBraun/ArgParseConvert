# Now simply link against gtest or gtest_main as needed.
add_executable(parameter_map_test
        "${PROJECT_SOURCE_DIR}/test/parameter_map_test.cc"
        "${PROJECT_SOURCE_DIR}/src/parameter_map.cc"
        "${PROJECT_SOURCE_DIR}/src/parameter.cc")
target_include_directories(parameter_map_test PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/test")
add_test(NAME parameter_map_test COMMAND parameter_map_test)