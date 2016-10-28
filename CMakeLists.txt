# Minimum CMake required
cmake_minimum_required(VERSION 2.8.12)

# Project
set(PROJECT ProtoMQTT)
project(${PROJECT})
 
 

SET(PROTOBUF_SRC_ROOT_FOLDER "X:/protobuf")
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

 
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS RobotMsg.proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})




find_path(PAHO_MQTT_INCLUDE_DIR MQTTClient.h)
find_library(PAHO_MQTT_SYNC_LIBRARY NAMES paho-mqtt3c.lib) 
find_library(PAHO_MQTT_ASYNC_LIBRARY NAMES paho-mqtt3a.lib) 
SET(PAHO_MQTT_LIBRARIES ${PAHO_MQTT_SYNC_LIBRARY} ${PAHO_MQTT_ASYNC_LIBRARY})
include_directories(${PAHO_MQTT_INCLUDE_DIR})

add_executable(ProtoMQTT main.cpp ${PROTO_SRCS})
target_link_libraries(ProtoMQTT ${PROTOBUF_LIBRARIES} ${PAHO_MQTT_LIBRARIES})