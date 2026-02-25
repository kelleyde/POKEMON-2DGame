# Install script for directory: C:/Users/laney/source/repos/POKEMON-2DGame/test/vendored/SDL_image/external/aom

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/game")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/aom" TYPE FILE FILES
    "C:/Users/laney/source/repos/POKEMON-2DGame/test/vendored/SDL_image/external/aom/aom/aom.h"
    "C:/Users/laney/source/repos/POKEMON-2DGame/test/vendored/SDL_image/external/aom/aom/aom_codec.h"
    "C:/Users/laney/source/repos/POKEMON-2DGame/test/vendored/SDL_image/external/aom/aom/aom_frame_buffer.h"
    "C:/Users/laney/source/repos/POKEMON-2DGame/test/vendored/SDL_image/external/aom/aom/aom_image.h"
    "C:/Users/laney/source/repos/POKEMON-2DGame/test/vendored/SDL_image/external/aom/aom/aom_integer.h"
    "C:/Users/laney/source/repos/POKEMON-2DGame/test/vendored/SDL_image/external/aom/aom/aom_decoder.h"
    "C:/Users/laney/source/repos/POKEMON-2DGame/test/vendored/SDL_image/external/aom/aom/aomdx.h"
    "C:/Users/laney/source/repos/POKEMON-2DGame/test/vendored/SDL_image/external/aom/aom/aomcx.h"
    "C:/Users/laney/source/repos/POKEMON-2DGame/test/vendored/SDL_image/external/aom/aom/aom_encoder.h"
    "C:/Users/laney/source/repos/POKEMON-2DGame/test/vendored/SDL_image/external/aom/aom/aom_external_partition.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "C:/Users/laney/source/repos/POKEMON-2DGame/test/build/vendored/SDL_image/external/aom-build/aom.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/laney/source/repos/POKEMON-2DGame/test/build/vendored/SDL_image/external/aom-build/aom.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Users/laney/source/repos/POKEMON-2DGame/test/build/Debug/aom.dll")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/laney/source/repos/POKEMON-2DGame/test/build/vendored/SDL_image/external/aom-build/aom_static.lib")
endif()

