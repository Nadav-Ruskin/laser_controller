cmake_minimum_required(VERSION 3.14)
enable_testing()
set(CMAKE_CXX_STANDARD 17)
project(LaserController VERSION "0.1")
add_executable(LaserController)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(USE_WIN32_THREADS ${CMAKE_USE_WIN32_THREADS_INIT})
set(USE_PTHREADS ${CMAKE_USE_PTHREADS_INIT})
target_link_libraries(LaserController PRIVATE ${CMAKE_THREAD_LIBS_INIT})

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()


target_sources(LaserController
PRIVATE    
	 "${CMAKE_CURRENT_LIST_DIR}/src/LaserController.cpp"
	 "${CMAKE_CURRENT_LIST_DIR}/src/LaserController.h"
	 "${CMAKE_CURRENT_LIST_DIR}/src/LaserControllerEngine.cpp"
	 "${CMAKE_CURRENT_LIST_DIR}/src/qconsolelistener.cpp"
	 "${CMAKE_CURRENT_LIST_DIR}/src/qconsolelistener.h"	 

)

