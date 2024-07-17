
file(GLOB videoViewerSources *.cpp)
file(GLOB videoViewerIncludes *.h )

find_library(DRM_LIB NAMES drm)
find_library(EGL_LIB NAMES EGL)
find_library(GBM_LIB NAMES gbm)
find_library(GLES2_LIB NAMES GLESv2)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-write-strings")	

add_executable(video-viewer ${videoViewerSources})
target_link_libraries(video-viewer jetson-utils)
#target_link_libraries(video-viewer pthread ${EGL_LIB} ${GLES2_LIB} ${GBM_LIB} ${DRM_LIB} )

install(TARGETS video-viewer DESTINATION bin)
