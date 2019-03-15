project(mongoose C)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(mongoose
    mongoose.c)
