cmake_minimum_required(VERSION 3.13)
project(server_proj)

set(CMAKE_CXX_STANDARD 17)


include_directories(/home/akuznetsov/Client-server-sokoban/ /home/akuznetsov/libtcod/include/) #/home/akuznetsov/sokoban_proj/cmake-build-debug/
link_directories(/home/akuznetsov/libtcod/ ) #/home/akuznetsov/sokoban_proj/cmake-build-debug/

#cmake-build-debug/sec_client.cpp
#cmake-build-debug/cl_main2.cpp

add_executable(server main.cpp /home/akuznetsov/Client-server-sokoban/TwoDimArray.h)
add_executable(client /home/akuznetsov/Client-server-sokoban/cl_main.cpp TwoDimArray.h)
add_executable(client2 /home/akuznetsov/Client-server-sokoban/cl_main2.cpp TwoDimArray.h)

target_link_libraries(server -ltcod -ltcodxx )
target_link_libraries(client -ltcod -ltcodxx )
target_link_libraries(client2 -ltcod -ltcodxx )