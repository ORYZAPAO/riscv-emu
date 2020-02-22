cmake_minimum_required(VERSION 2.8)
add_executable(rv32uiemu rv32uiemu.cpp)
add_definitions("-Wall -std=c++14 -g") 

# Boostライブラリを検索
find_package(Boost)
 
include(FindPkgConfig)
 
# Boostをincludeパスに設定
include_directories(SYSTEM
 ${Boost_INCLUDE_DIRS}
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
