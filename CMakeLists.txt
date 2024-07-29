cmake_minimum_required(VERSION 3.16)
project(Project)
set(CMAKE_CXX_STANDARD 17)                                                      
set(CMAKE_CXX_STANDARD_REQUIRED ON)                                             

set(CMAKE_AUTOMOC ON)                                                           
set(CMAKE_AUTORCC ON)                                                           
set(CMAKE_AUTOUIC ON)  

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "/home/orangepi/pytorch")

find_package(Torch REQUIRED)
include_directories(SYSTEM ${TORCH_INCLUDE_DIRS})

message(STATUS "Pytorch status :")
message(STATUS "    libraries: ${TORCH_LIBRARIES}")
message(STATUS "    Torch Flags: ${TORCH_CXX_FLAGS}")
message(STATUS "    Torch Include: ${TORCH_INCLUDE_DIRS}")

find_package( Qt5 REQUIRED COMPONENTS Core Multimedia SerialPort)

# Specifying we are using pthread for UNIX systems.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS} -Wall")

if(NOT Torch_FOUND)
    message(FATAL_ERROR "Pytorch Not Found!")
endif(NOT Torch_FOUND)

set( QT_DONT_USE_QTGUI TRUE )

add_executable( ${PROJECT_NAME} 
                src/main.cpp 
                lib/ws2812.cpp
                lib/dr_wav.c
                src/led_effects.cpp
                src/touch.cpp
                src/sound_input.cpp
                src/predictionc.cpp
                src/M701.cpp
                src/effects/Effect.cpp
                src/effects/basic/SparklesEffect.cpp
                src/effects/basic/FireEffect.cpp
                helpers/SignalHandler.cpp
                helpers/Logger.cpp
                libi2c/src/i2c.c
                lib/smbus.cpp
                )

include_directories(
                src
                src/effects
                src/effects/basic
                helpers
                lib
                libi2c/include/i2c
                libi2c/include
)
# target_link_libraries( ${PROJECT_NAME}  ${QT_LIBRARIES} )
#add_library(torch SHARED IMPORTED)
target_link_libraries( ${PROJECT_NAME}  Qt5::Core Qt5::Multimedia Qt5::SerialPort pthread torch) #"${TORCH_LIBRARIES}"