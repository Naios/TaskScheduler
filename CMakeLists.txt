cmake_minimum_required(VERSION 3.16)

project(TaskScheduler LANGUAGES CXX VERSION 1.0)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(FetchContent)

option(BUILD_TASKSCHEDULER_TESTAPP "Build unit-test app" ON)

add_library(TaskScheduler STATIC)
add_library(TaskScheduler::TaskScheduler ALIAS TaskScheduler)

if(MSVC)
    if(${MSVC_VERSION} LESS 1800)
        message(FATAL_ERROR "You are using an unsupported version of Visual Studio "
            "which doesn't support all required C++11 features. "
            "(Visual Studio 2013 (Version >= 1800) is required!)")
    endif()

    if(PLATFORM EQUAL 64)
        add_definitions("-D_WIN64")
    else()
        # ...
    endif()

    target_compile_options(TaskScheduler
        PRIVATE /W4)

else()

    target_compile_options(TaskScheduler
        PRIVATE -Wall -Wextra -pedantic $<$<CONFIG:Debug>:-fno-omit-frame-pointer>)
endif()

target_sources(TaskScheduler
    PRIVATE src/TaskScheduler.cpp)

target_include_directories (TaskScheduler PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/TaskScheduler>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/TaskScheduler>)

target_compile_features(TaskScheduler
    PUBLIC cxx_std_11)

set_target_properties(TaskScheduler PROPERTIES
    PUBLIC_HEADER include/TaskScheduler/TaskScheduler.hpp)

if (BUILD_TASKSCHEDULER_TESTAPP)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v2.13.1)

    FetchContent_MakeAvailable(Catch2)

    add_executable(Test testing/Test.cpp)
    target_link_libraries(Test
        PRIVATE TaskScheduler::TaskScheduler Catch2::Catch2)
endif()

# INSTALLATION
install(TARGETS TaskScheduler
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    )

# Make the library consumers' life as easy as possible
install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

# Export all targets in the target group "also" in the build directory, to be used by a consuming project (even if this project has not been installed) by including the *Targets.cmake file
export (
    EXPORT ${PROJECT_NAME}Targets
    NAMESPACE ${PROJECT_NAME}::
    FILE ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Targets.cmake #Consumers can just include this file
    )

# Add configuration/version files that allows consumers to use find_package
configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/ProjectConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
    )

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )
