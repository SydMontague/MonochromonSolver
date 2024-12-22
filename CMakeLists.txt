cmake_minimum_required(VERSION 3.27)
project(DW1Simulator VERSION 1.2.0)

include(cmake/CPM.cmake)

# boost
if(WIN32)
    set(Boost_USE_STATIC_LIBS ON) 
endif()

CPMAddPackage(
  NAME Boost
  VERSION 1.86.0
  GITHUB_REPOSITORY "boostorg/boost"
  GIT_TAG "boost-1.86.0"
  OPTIONS "BOOST_ENABLE_CMAKE ON" "BOOST_INCLUDE_LIBRARIES program_options"
)

# --- Target ---
set(SOURCE_FILES ${SOURCE_FILES} "src/DW1Simulator.cpp" "src/MonochromeShop.cpp" "src/FullSolver.cpp" "src/MonochromeShop.hpp" "src/FullSolver.hpp")

add_executable(DW1Simulator ${SOURCE_FILES})
target_link_libraries(DW1Simulator PRIVATE Boost::program_options)

set_target_properties(DW1Simulator PROPERTIES CXX_STANDARD 20)
set_target_properties(DW1Simulator PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

target_compile_definitions(DW1Simulator PRIVATE PROJECT_NAME="${PROJECT_NAME}")
target_compile_definitions(DW1Simulator PRIVATE PROJECT_VERSION="v${PROJECT_VERSION}")
target_compile_definitions(DW1Simulator PRIVATE PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(DW1Simulator PRIVATE PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(DW1Simulator PRIVATE PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH})

install(TARGETS DW1Simulator)