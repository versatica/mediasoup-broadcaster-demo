cmake_minimum_required(VERSION 3.8)

project(broadcaster LANGUAGES CXX)

# Set version number.
set(broadcaster_VERSION_MAJOR 0)
set(broadcaster_VERSION_MINOR 1)

# C++ standard requirements.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

message("\n=========== mediasoup-broadcaster-demo Build Configuration ===========\n")
message(STATUS "LIBWEBRTC_INCLUDE_PATH : " ${LIBWEBRTC_INCLUDE_PATH})
message(STATUS "LIBWEBRTC_BINARY_PATH  : " ${LIBWEBRTC_BINARY_PATH})
if(NOT APPLE)
    message(STATUS "OPENSSL_INCLUDE_DIR    : " ${OPENSSL_INCLUDE_DIR})
    message(STATUS "CMAKE_USE_OPENSSL      : " ${CMAKE_USE_OPENSSL})
endif()
message("")

# Add some compile flags to our source files.
set_source_files_properties(${SOURCE_FILES}
	PROPERTIES COMPILE_FLAGS -Wall -Wextra -Wpedantic)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

# Create target.
add_executable(${PROJECT_NAME} ${SOURCES})

# Extra libs required in order to use Apple media devices.
if(APPLE)
	find_library(APPLICATION_SERVICES ApplicationServices)
	find_library(AUDIO_TOOLBOX AudioToolbox)
	find_library(AV_FOUNDATION AVFoundation)
	find_library(CORE_AUDIO CoreAudio)
	find_library(CORE_FOUNDATION Foundation)
	find_library(CORE_MEDIA CoreMedia)
	find_library(CORE_VIDEO CoreVideo)
	find_library(CORE_SERVICES CoreServices)
	find_library(SECURITY Security)
	find_library(LDAP LDAP)

	target_link_libraries(${PROJECT_NAME} PUBLIC
		${APPLICATION_SERVICES}
		${AUDIO_TOOLBOX}
		${AV_FOUNDATION}
		${CORE_AUDIO}
		${CORE_FOUNDATION}
		${SECURITY}
		${LDAP}
		${CORE_MEDIA}
		${CORE_VIDEO}
		${CORE_SERVICES}
	)

endif(APPLE)

target_sources(${PROJECT_NAME} PRIVATE
	src/Broadcaster.cpp
	src/main.cpp
	src/MediaStreamTrackFactory.cpp
)

# Private (implementation) header files.
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Source Dependencies.
if(APPLE)
    find_package(CPR REQUIRED)
    find_package(CURL REQUIRED)
else()
    add_subdirectory(deps/cpr "${CMAKE_CURRENT_BINARY_DIR}/cpr")
endif()

add_subdirectory(deps/libmediasoupclient "${CMAKE_CURRENT_BINARY_DIR}/libmediasoupclient")
add_subdirectory(deps/libwebrtc "${CMAKE_CURRENT_BINARY_DIR}/libwebrtc")

# Public (interface) headers from dependencies.
target_include_directories(${PROJECT_NAME} PUBLIC
	${CPR_INCLUDE_DIRS}
	"${PROJECT_SOURCE_DIR}/deps/libmediasoupclient/include"
	"${PROJECT_SOURCE_DIR}/deps/libwebrtc"
)

# Public (interface) dependencies.
target_link_libraries(${PROJECT_NAME} PUBLIC
	mediasoupclient
	webrtc_broadcaster
	${CPR_LIBRARIES}
	${CURL_LIBRARIES}
)

