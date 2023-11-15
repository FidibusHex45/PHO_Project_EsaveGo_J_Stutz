## Build configuration ##
cmake_minimum_required(VERSION 3.24)
project(PHO_Project_EsaveGo_J_Stutz)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/inc)

find_package(OpenCV CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(PythonLibs REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${PYTHON_INCLUDE_DIRS})

## Additional CMake Modules
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_MODULE_PATH})
find_package(uEye-SDK 4.95)

add_executable(
    ${PROJECT_NAME} 
    #src/main.cpp
    src/test.cpp
    src/camera.cpp
    src/mem_handler.cpp
    src/serialSTM32.cpp
    src/keystate.cpp
    src/detectTrack.cpp
    #util/trackBar.cpp
    #util/test.cpp
    #src/slider.cpp
    #src/serial.cpp
    )

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} uEye-SDK nlohmann_json::nlohmann_json ${PYTHON_LIBRARIES})