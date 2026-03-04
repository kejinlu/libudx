cmake_minimum_required(VERSION 3.14)
project(libudx VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ---------- Options ----------
option(UDX_BUILD_TESTS "Build tests" ON)
option(UDX_BUILD_EXAMPLES "Build examples" ON)
option(BUILD_SHARED_LIBS "Build shared library" OFF)

# ---------- Dependencies ----------
find_package(ZLIB REQUIRED)

# ---------- Library ----------
file(GLOB UDX_SOURCES src/*.c)
file(GLOB UDX_HEADERS src/*.h)

set(UDX_PUBLIC_HEADERS
    src/udx_reader.h
    src/udx_writer.h
    src/udx_types.h
)

add_library(udx ${UDX_SOURCES})
target_include_directories(udx
    PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
            $<INSTALL_INTERFACE:include>
)
target_link_libraries(udx PUBLIC ZLIB::ZLIB)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(udx PUBLIC m)
endif()

if(MSVC)
    target_compile_options(udx PRIVATE /W4)
else()
    target_compile_options(udx PRIVATE -Wall -Wextra -pedantic)
endif()

# ---------- Tests ----------
if(UDX_BUILD_TESTS)
    enable_testing()
    add_executable(udxtest udxtest/main.c)
    target_link_libraries(udxtest PRIVATE udx)
    add_test(NAME udxtest COMMAND udxtest)
endif()

# ---------- Install ----------
include(GNUInstallDirs)

install(TARGETS udx
    EXPORT udxTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES ${UDX_PUBLIC_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/udx)

install(EXPORT udxTargets
    FILE udxTargets.cmake
    NAMESPACE udx::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/udx
)
