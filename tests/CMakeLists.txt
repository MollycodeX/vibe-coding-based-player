enable_testing()
add_executable(player_tests test_main.cpp)
target_include_directories(player_tests PUBLIC ${PROJECT_SOURCE_DIR}/include)
add_test(NAME PlayerBasicTest COMMAND player_tests)