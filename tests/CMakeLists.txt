enable_testing()

add_executable(player_tests test_main.cpp)
target_include_directories(player_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
# Link against the player library so Playlist (and AudioPlayer) are available.
target_link_libraries(player_tests PRIVATE vibe_player_lib)

add_test(NAME PlayerBasicTest COMMAND player_tests)
