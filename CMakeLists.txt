# Project setup
cmake_minimum_required(VERSION 3.16)
project(wirevisual VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Profiles
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message("Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
endif()

# Creates the compile_commands.json file for your LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Create symlink for compile_commands.json in root dir for lsp
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
)

# Enable the Qt Meta Object Compiler (MOC)
set(CMAKE_AUTOMOC TRUE)

# Output path
set(OutputDirectory "${CMAKE_SOURCE_DIR}/bin/${CMAKE_SYSTEM_PROCESSOR}/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OutputDirectory}/obj")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OutputDirectory}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OutputDirectory}")

# Get the Qt library
find_package(Qt5 REQUIRED COMPONENTS Widgets)

# Add cafe
include_directories(/opt/gfa/cafe/cpp/cafe-1.19.2-gcc-7.3.0/include)
link_directories(/opt/gfa/cafe/cpp/cafe-1.19.2-gcc-7.3.0/lib/RHEL8-x86_64)

# Add epics
include_directories(/usr/local/epics/base-7.0.6/include)
include_directories(/usr/local/epics/base-7.0.6/include/os/Linux)
include_directories(/usr/local/epics/base-7.0.6/include/compiler/gcc)
link_directories(/usr/local/epics/base-7.0.6/lib/RHEL8-x86_64)

# Add the subdirectories
add_subdirectory(forms)
add_subdirectory(src/app)
add_subdirectory(src/logic)

# Build the QT app
add_executable(wirevisual
    ${UI_FILES}
    ${APP_SRC_FILES}
    main.cpp
)

# Add the include files to avoid relative paths
target_include_directories(wirevisual PRIVATE src/app)
target_include_directories(wirevisual PRIVATE forms)
target_include_directories(libwirevisual PUBLIC include)

# Link Epics Chanel Acess to custom lib
target_link_libraries(libwirevisual ca)

# Link cafe with custom lib
target_link_libraries(libwirevisual cafe)

# Link Qt5 and custom lib to the main executable 
target_link_libraries(wirevisual PRIVATE Qt5::Widgets)
target_link_libraries(wirevisual PUBLIC libwirevisual)
