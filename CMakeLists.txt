cmake_minimum_required(VERSION 3.8)

project(broadcaster LANGUAGES CXX)

# Set version number.
set(broadcaster_VERSION_MAJOR 0)
set(broadcaster_VERSION_MINOR 1)

# C++ standard requirements.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add some compile flags to our source files.
set_source_files_properties(${SOURCE_FILES}
	PROPERTIES COMPILE_FLAGS -Wall -Wextra -Wpedantic)

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

	target_link_libraries(${PROJECT_NAME} PUBLIC
		${APPLICATION_SERVICES}
		${AUDIO_TOOLBOX}
		${AV_FOUNDATION}
		${CORE_AUDIO}
		${CORE_FOUNDATION}
		${CORE_MEDIA}
		${CORE_VIDEO}
		${CORE_SERVICES}
	)
endif(APPLE)

include(FetchContent)
FetchContent_Declare(
  mediasoupclient
  GIT_REPOSITORY https://github.com/copiltembel/libmediasoupclient.git
  GIT_TAG        v3-DataChannelsImpl
)
FetchContent_MakeAvailable(mediasoupclient)

FetchContent_Declare(
	cpr
	GIT_REPOSITORY https://github.com/whoshuu/cpr
	GIT_TAG		   master
)
FetchContent_MakeAvailable(cpr)


target_sources(${PROJECT_NAME} PRIVATE
	src/Broadcaster.cpp
	src/main.cpp
	src/MediaStreamTrackFactory.cpp
)

# Private (implementation) header files.
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Source Dependencies.
add_subdirectory(deps/libwebrtc "${CMAKE_CURRENT_BINARY_DIR}/libwebrtc")

# Public (interface) headers from dependencies.
target_include_directories(${PROJECT_NAME} PUBLIC
	${cpr_SOURCE_DIR}/include
	${mediasoupclient_SOURCE_DIR}/include
	"${PROJECT_SOURCE_DIR}/deps/libwebrtc"
)

# Public (interface) dependencies.
target_link_libraries(${PROJECT_NAME} PUBLIC
	cpr
	mediasoupclient
	webrtc_broadcaster
	${LIBWEBRTC_BINARY_PATH}/libwebrtc${CMAKE_STATIC_LIBRARY_SUFFIX}
)

