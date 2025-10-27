cmake_minimum_required(VERSION 3.15)
project(VanguardEmulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR "This project requires Windows")
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/logger.cpp
    src/config.cpp
    src/service_manager.cpp
    src/pipe_server.cpp
    src/message_parser.cpp
)

set(HEADERS
    include/logger.h
    include/config.h
    include/service_manager.h
    include/pipe_server.h
    include/message_parser.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    psapi
    kernel32
    user32
    advapi32
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        _CONSOLE
        UNICODE
        _UNICODE
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES config.ini DESTINATION bin)
install(FILES README.md DESTINATION .)
