
cmake_minimum_required(VERSION 2.8)

# determine if jetson-utils is being built as a submodule inside another repo,
# or if it's being build standalone (if the later, we need to do some configuration)
get_directory_property(hasParent PARENT_DIRECTORY)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(hasParent)
	message("-- jetson-utils:  building as submodule, ${hasParent}")
else()
	message("-- jetson-utils:  building as standalone")
	
	# standalone project
	project(jetson-utils)
  
	# -std=gnu++11
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-write-strings")	
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")	
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")	
  
	# setup CUDA
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cuda")
	find_package(CUDA)
	message("-- CUDA version: ${CUDA_VERSION}")

	set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -O3)

	if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
		message("-- CUDA ${CUDA_VERSION} detected (${CMAKE_SYSTEM_PROCESSOR}), enabling SM_53 SM_62")
		set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -gencode arch=compute_53,code=sm_53 -gencode arch=compute_62,code=sm_62)

		if(CUDA_VERSION_MAJOR GREATER 9)
			message("-- CUDA ${CUDA_VERSION} detected (${CMAKE_SYSTEM_PROCESSOR}), enabling SM_72")
			set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -gencode arch=compute_72,code=sm_72)
		endif()

		if(CUDA_VERSION_MAJOR GREATER 10)
			message("-- CUDA ${CUDA_VERSION} detected (${CMAKE_SYSTEM_PROCESSOR}), enabling SM_87")
			set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -gencode arch=compute_87,code=sm_87)
		endif()
	endif()
	
	# setup project output paths
	set(PROJECT_OUTPUT_DIR  ${PROJECT_BINARY_DIR}/${CMAKE_SYSTEM_PROCESSOR})
	set(PROJECT_INCLUDE_DIR ${PROJECT_OUTPUT_DIR}/include)

	file(MAKE_DIRECTORY ${PROJECT_INCLUDE_DIR})
	file(MAKE_DIRECTORY ${PROJECT_OUTPUT_DIR}/bin)

	message("-- system arch:  ${CMAKE_SYSTEM_PROCESSOR}")
	message("-- output path:  ${PROJECT_OUTPUT_DIR}")

	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/bin)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/lib)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/lib)
	
	# detect distro version
	find_program(LSB_RELEASE_EXEC lsb_release)

	execute_process(COMMAND "${LSB_RELEASE_EXEC}" --short --id OUTPUT_VARIABLE LSB_RELEASE_ID OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND "${LSB_RELEASE_EXEC}" --short --release OUTPUT_VARIABLE LSB_RELEASE_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND "${LSB_RELEASE_EXEC}" --short --codename OUTPUT_VARIABLE LSB_RELEASE_CODENAME OUTPUT_STRIP_TRAILING_WHITESPACE)

	message("-- distro ID:       ${LSB_RELEASE_ID}")
	message("-- distro version:  ${LSB_RELEASE_VERSION}")
	message("-- distro codename: ${LSB_RELEASE_CODENAME}")

	# build C/C++ interface
	include_directories(${PROJECT_INCLUDE_DIR})
    #include_directories(/usr/include/gstreamer-1.0 /usr/lib/aarch64-linux-gnu/gstreamer-1.0/include /usr/include/glib-2.0 /usr/include/libxml2 /usr/lib/aarch64-linux-gnu/glib-2.0/include/)
    include_directories(/usr/lib/glib-2.0/include/)

endif()

# option for enabling/disabling NVMM memory in multimedia stack
find_library(NVBUF_UTILS NAMES nvbuf_utils PATHS /usr/lib/aarch64-linux-gnu/tegra)
message("-- nvbuf_utils:  ${NVBUF_UTILS}")

if(NVBUF_UTILS)
	set(ENABLE_NVMM_DEFAULT ON)
else()
	set(ENABLE_NVMM_DEFAULT OFF)
endif()

option(ENABLE_NVMM "Enable use of NVMM zero-copy memory in video and camera streaming" ${ENABLE_NVMM_DEFAULT})
message("-- NVMM zero-copy memory:  ENABLE_NVMM=${ENABLE_NVMM}")

if(ENABLE_NVMM)
	add_definitions(-DENABLE_NVMM)
endif()

# additional paths for includes and libraries
include_directories(${PROJECT_INCLUDE_DIR}/jetson-utils)
include_directories(/usr/include/gstreamer-1.0 /usr/include/glib-2.0 /usr/include/libxml2 /usr/include/json-glib-1.0 /usr/include/libsoup-2.4 /usr/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu/gstreamer-1.0/include /usr/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu/glib-2.0/include/)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
	include_directories(/usr/src/jetson_multimedia_api/include)
	link_directories(/usr/lib/aarch64-linux-gnu/tegra)
endif()

# build library
#file(GLOB jetsonUtilitySources *.cpp camera/*.cpp codec/*.cpp display/*.cpp image/*.cpp input/*.cpp network/*.cpp threads/*.cpp video/*.cpp)
file(GLOB jetsonUtilitySources *.cpp camera/*.cpp codec/*.cpp image/*.cpp input/*.cpp network/*.cpp threads/*.cpp video/*.cpp)
file(GLOB jetsonUtilityIncludes *.h *.hpp camera/*.h codec/*.h display/*.h image/*.h image/*.inl input/*.h network/*.h threads/*.h threads/*.inl video/*.h)

option(WITH_CUDA "With CUDA support" ON)
option(WITH_OPENCL "With OpenCL support" OFF)
option(WITH_IMGUI "Enable dear imgui support" TRUE)
option(WITH_DRM "Enable DRM rendering support" TRUE)
option(WITH_DISPLAY "Use OpenGL based display output" TRUE)
option(USE_OPENGL_ES2 "Use OpenGL ES2 instead of OpenGL" FALSE)

#set(WITH_DISPLAY OFF)

find_library(GL_LIB NAMES GL)
find_library(GLU_LIB NAMES GLU)
find_library(GLES2_LIB NAMES GLESv2)
find_library(GLEW_LIB NAMES GLEW)

if(${USE_OPENGL_ES2})
    set(OPENGL_LIBRARY ${GLES2_LIB})
    add_definitions(-DUSE_OPENGL_ES2)
else()
    set(OPENGL_LIBRARY ${GL_LIB})
endif()

if(${WITH_DISPLAY})
    message(STATUS "Display is enabled")
    add_definitions(-DWITH_DISPLAY=1)
    #find_package(glfw3 REQUIRED)
    file(GLOB jetsonUtilitySourcesDisplay display/*.cpp)
    list(APPEND jetsonUtilitySources  ${jetsonUtilitySourcesDisplay})
    if(${WITH_IMGUI})
        list(APPEND jetsonUtilitySources  dearimgui/dearimgui.cpp)
        list(APPEND jetsonUtilityIncludes dearimgui/dearimgui.h)
    endif()
endif()

if(${WITH_CUDA})
    message(STATUS "CUDA is enabled")
    add_definitions(-DWITH_CUDA=1)
    file(GLOB jetsonUtilitySourcesCuda cuda/*.cpp cuda/*.cu)
    file(GLOB jetsonUtilityIncludesCuda cuda/*.h cuda/*.cuh)
    list(APPEND jetsonUtilitySources  ${jetsonUtilitySourcesCuda})
    list(APPEND jetsonUtilityIncludes ${jetsonUtilityIncludesCuda})
endif(${WITH_CUDA})

if(${WITH_OPENCL})
    find_package(OpenCL)
    add_subdirectory("ocl")
    include_directories(${OpenCV_INCLUDE_DIRS})
    message(STATUS OpenCL_LIBS=${OpenCL_LIBRARY})

    add_definitions(-DWITH_OPENCL=1)
    add_definitions(-DCL_TARGET_OPENCL_VERSION=300)
    file(GLOB jetsonUtilitySourcesOCL opencl/*.cpp ocl/*.cpp)
    file(GLOB jetsonUtilityIncludesOCL opencl/*.h opencl/*.cl ocl/*.h)
    list(APPEND jetsonUtilitySources  ${jetsonUtilitySourcesOCL})
    list(APPEND jetsonUtilityIncludes ${jetsonUtilityIncludesOCL})
endif(${WITH_OPENCL})


if(WITH_IMGUI)

	add_definitions(-DWITH_IMGUI)
    #might need this to be visible from client code

    if(${USE_OPENGL_ES2})
        add_definitions(-DIMGUI_IMPL_OPENGL_ES2)
    endif()

	file(GLOB IMGUI_SRC imgui/*.cpp imgui/backends/imgui_impl_opengl3.cpp imgui/backends/imgui_impl_glfw.cpp)
	file(GLOB IMGUI_INC imgui/*.h imgui/backends/imgui_impl_opengl3.h imgui/backends/imgui_impl_glfw.h)

    # implot
    set(IMGUI_SRC  ${IMGUI_SRC} implot/implot.cpp implot/implot_items.cpp implot/implot_demo.cpp)
    set(IMGUI_INC ${IMGUI_INC} implot/implot.h implot/implot_internal.h)


	list(APPEND jetsonUtilitySources  ${IMGUI_SRC})
	list(APPEND jetsonUtilityIncludes  ${IMGUI_INC})
endif()

if(WITH_DRM)
    find_library(DRM_LIB NAMES drm)
    find_library(EGL_LIB NAMES EGL)
    find_library(GBM_LIB NAMES gbm)
    find_path(DRM_INCLUDE_DIR NAME drm.h PATHS /usr/include/libdrm /usr/include/drm)
    find_path(GBM_INCLUDE_DIR NAME gbm.h PATHS /usr/include)

    add_definitions(-DWITH_DRM)
    include_directories(${DRM_INCLUDE_DIR})
    include_directories(${GBM_INCLUDE_DIR})

    file(GLOB DRM_SRC drm/*.cpp drm/drmlib/*.c)
    file(GLOB DRM_INC drm/*.h)

    list(APPEND jetsonUtilitySources  ${DRM_SRC})
    list(APPEND jetsonUtilityIncludes  ${DRM_INC})
endif()

if(WITH_CUDA)
    cuda_add_library(jetson-utils SHARED ${jetsonUtilitySources})
    target_compile_definitions(jetson-utils PUBLIC "-DWITH_CUDA=1")
else()
    add_library(jetson-utils SHARED ${jetsonUtilitySources})
endif()

#target_link_libraries(jetson-utils GL GLU GLEW gstreamer-1.0 gstapp-1.0 gstpbutils-1.0 gstwebrtc-1.0 gstsdp-1.0 gstrtspserver-1.0 json-glib-1.0 soup-2.4 ${CUDA_nppicc_LIBRARY})	
target_link_libraries(jetson-utils ${OPENGL_LIBRARY} GLX ${GLU_LIB} ${GLEW_LIB} gstreamer-1.0 gstapp-1.0 gstpbutils-1.0 gstwebrtc-1.0 gstsdp-1.0 gstrtspserver-1.0 json-glib-1.0 soup-2.4 ${CUDA_nppicc_LIBRARY})	

if(NVBUF_UTILS)
	target_link_libraries(jetson-utils nvbuf_utils)
endif()

if(WITH_IMGUI)
	target_link_libraries(jetson-utils glfw)
endif()

if(WITH_OPENCL)
    target_link_libraries(jetson-utils ${OpenCL_LIBRARY} ocl)
    target_compile_definitions(jetson-utils PUBLIC "-DWITH_OPENCL=1")
endif()


find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)
target_link_libraries(jetson-utils PRIVATE PkgConfig::deps)

if(WITH_DRM)
    target_link_libraries(jetson-utils pthread ${EGL_LIB} ${GLES2_LIB} ${GBM_LIB} ${DRM_LIB} )
endif()

# transfer all headers to the include directory 
file(MAKE_DIRECTORY ${PROJECT_INCLUDE_DIR}/jetson-utils)

foreach(include ${jetsonUtilityIncludes})
	message("-- Copying ${include}")
	configure_file(${include} ${PROJECT_INCLUDE_DIR}/jetson-utils COPYONLY)
endforeach()
	
# install headers
foreach(include ${jetsonUtilityIncludes})
	install(FILES "${include}" DESTINATION include/jetson-utils)
endforeach()

# install the shared library
install(TARGETS jetson-utils DESTINATION lib EXPORT jetson-utilsConfig)

# install the cmake project, for importing
install(EXPORT jetson-utilsConfig DESTINATION share/jetson-utils/cmake)

# build python bindings + samples
#add_subdirectory(python)
add_subdirectory(video/video-viewer)

#add_subdirectory(camera/camera-viewer)
#add_subdirectory(display/gl-display-test)
#add_subdirectory(network/webrtc-server)
#add_subdirectory(network/rtsp-server)

