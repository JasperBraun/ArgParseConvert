add_executable(parameter_test
        "${PROJECT_SOURCE_DIR}/test/parameter_test.cc")
target_include_directories(parameter_test PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/lib/catch/include"
        "${PROJECT_SOURCE_DIR}/test")
add_test(NAME parameter_test COMMAND parameter_test)

add_executable(parameter_map_test
        "${PROJECT_SOURCE_DIR}/test/parameter_map_test.cc")
target_include_directories(parameter_map_test PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/lib/catch/include"
        "${PROJECT_SOURCE_DIR}/test")
add_test(NAME parameter_map_test COMMAND parameter_map_test)