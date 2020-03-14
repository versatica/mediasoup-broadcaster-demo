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

target_sources(${PROJECT_NAME} PRIVATE
	src/Broadcaster.cpp
	src/main.cpp
	src/MediaStreamTrackFactory.cpp
	src/fake_audio_capture_module.cc
	src/fake_frame_source.cc
	src/frame_generator_capturer.cc
	src/frame_generator.cc
	src/test_video_capturer.cc
	src/frame_utils.cc
)

# Private (implementation) header files.
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Source Dependencies.
add_subdirectory(deps/libmediasoupclient "${CMAKE_CURRENT_BINARY_DIR}/libmediasoupclient")
add_subdirectory(deps/cpr "${CMAKE_CURRENT_BINARY_DIR}/cpr")

# Public (interface) headers from dependencies.
target_include_directories(${PROJECT_NAME} PUBLIC
	"${PROJECT_SOURCE_DIR}/deps/libmediasoupclient/include"
	${CPR_INCLUDE_DIRS}
)

# Public (interface) dependencies.
target_link_libraries(${PROJECT_NAME} PUBLIC
	mediasoupclient
	${CPR_LIBRARIES}
)

# # Create a library with required webrtc extra code and link against it.
# file(GLOB_RECURSE WEBRTC_EXTRAS
# 	${LIBWEBRTC_BINARY_PATH}/pc/pc_test_utils/fake_audio_capture_module.o
# 	${LIBWEBRTC_BINARY_PATH}/rtc_base/task_queue_for_test/task_queue_for_test.o
# 	${LIBWEBRTC_BINARY_PATH}/media/rtc_media_tests_utils/fake_frame_source.o
# )

# set(WEBRTC_EXTRA_OBJS ${WEBRTC_EXTRAS})

# add_library (webrtc_extras STATIC ${WEBRTC_EXTRA_OBJS})
# set_target_properties(webrtc_extras PROPERTIES LINKER_LANGUAGE CXX)
# target_link_libraries (${PROJECT_NAME} PUBLIC webrtc_extras)
