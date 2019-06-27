cmake_minimum_required(VERSION 3.0)
project(timer)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set (CMAKE_CXX_STANDARD 17)

file(GLOB TIMER_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.h")
add_executable(timer "${TIMER_SOURCES}")

find_package(Curses REQUIRED)
target_link_libraries(timer "${CURSES_LIBRARIES}")
message("Curses Libraries are ${CURSES_LIBRARIES}.")

install(TARGETS timer RUNTIME DESTINATION bin)
