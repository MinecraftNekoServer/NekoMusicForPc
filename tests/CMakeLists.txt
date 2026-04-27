enable_testing()

find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(test_playerengine
    test_playerengine.cpp
    ${CMAKE_SOURCE_DIR}/src/core/playerengine.cpp
)

target_include_directories(test_playerengine PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_playerengine PRIVATE
    Qt6::Test
    Qt6::Multimedia
)

add_test(NAME test_playerengine COMMAND test_playerengine)
