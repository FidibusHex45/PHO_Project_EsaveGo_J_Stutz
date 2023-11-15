# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2022, Arne Wendt
#


#
# FinduEye-SDK.cmake    - CMake module to find iDS uEye SDK
# uEye-SDK-config.cmake - package configuration file for iDS uEye SDK
#
#
# exports:
#               uEye-SDK - as IMPORTED library target with its INTERFACE_INCLUDE_DIRECTORIES set
#
# sets:
#               uEye-SDK_INCLUDE_DIRS   - the directory containing uEye.h/ueye.h
#               uEye-SDK_LIBRARIES              - uEye SDK libraries
#               uEye-SDK_VERSION                - uEye SDK version
#


IF(NOT TARGET uEye-SDK)
IF(NOT uEye-SDK_FOUND)
        INCLUDE(FindPackageHandleStandardArgs)

        IF(WIN32) # is Windows
                IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
                        SET(LIB_ARCH_SUFFIX "_64")
                ELSE()
                        SET(LIB_ARCH_SUFFIX "")
                ENDIF()

                # assign PROGRAMFILES(X86) name to variable, as variable expansion
                # with typed out "PROGRAMFILES(X86)" fails
                SET(PFX86 "PROGRAMFILES(X86)")

                # visual studios PROGRAMFILES variable may point to a different
                # directory than the standard environment; PROGRAMFILESW6432 seem
                # to be missing completely. try to point to the non-x86 dir by
                # stripping " (x86)" from the end of PROGRAMFILES
                STRING(REGEX REPLACE " \\([xX]86\\).*" "" PFNX86 $ENV{PROGRAMFILES})

                # Be able to define a "common-root"-path to the ueye library and
                # headers. Eases finding it in unusual locations. Try to determine
                # the LIBUEye_DIR based on the location of the header file
                FIND_PATH(uEye-SDK
                        NAMES   "include/uEye.h"
                        PATHS   "$ENV{PROGRAMFILES}"
                                        "$ENV{${PFX86}}"
                                        "$ENV{PROGRAMFILESW6432}"
                                        "${PFNX86}"
                        PATH_SUFFIXES
                                        "IDS/uEye/Develop/"

                )

                FIND_PATH(uEye-SDK_INCLUDE_DIRS
                        NAMES   "uEye.h"
                        PATHS   "${uEye-SDK}/include/"
                )

                FIND_FILE(uEye-SDK_LIBRARIES "uEye_api${LIB_ARCH_SUFFIX}.lib"
                        PATHS   ${uEye-SDK}/Lib/
                )

        ELSE() # is *NIX like

                FIND_PATH(uEye-SDK
                        NAMES   "include/uEye.h" "include/ueye.h"
                        PATHS   "/opt/ids/ueye/"
                                "/usr/"
                                "/usr/local/"
                )

                FIND_PATH(uEye-SDK_INCLUDE_DIRS
                        NAMES   "uEye.h" "ueye.h"
                        PATHS   "${uEye-SDK}"
                                "${uEye-SDK}/include/"
                )

                FIND_FILE(uEye-SDK_LIBRARIES libueye_api.so
                        PATHS           "/usr/"
                                        "/usr/local/"
                                        "${uEye-SDK}"
                                        "/opt/ids/ueye"
                        PATH_SUFFIXES   "lib/"
                                        "lib64/"
                                        "i386-linux-gnu/"
                                        "x86_64-linux-gnu/"
                                        "lib/i386-linux-gnu/"
                                        "lib/x86_64-linux-gnu/"
                )

        ENDIF() # end plattform specific code


        IF(uEye-SDK_INCLUDE_DIRS AND uEye-SDK_LIBRARIES)
                IF(uEye-SDK)
                        # hide individual variables if "common-root" is found
                        MARK_AS_ADVANCED(uEye-SDK_INCLUDE_DIRS)
                        MARK_AS_ADVANCED(uEye-SDK_LIBRARIES)
                ELSE()
                        # hide "common-root" if not found, but individual variables are OK
                        MARK_AS_ADVANCED(uEye-SDK)
                ENDIF()

                # find version
                IF(WIN32) # is Windows
                        # read version from header
                        FIND_FILE(uEye-SDK_VERSION_HEADER version.h PATHS ${uEye-SDK_INCLUDE_DIRS})
                        FILE(STRINGS "${uEye-SDK_VERSION_HEADER}" uEye-SDK_VERSION_DEFINE REGEX "^#define PRODUCT_VERSION *\t*[0-9.]+$")
                        STRING(REGEX MATCH "[0-9.]+" uEye-SDK_VERSION "${uEye-SDK_VERSION_DEFINE}")
                ELSE() # is *NIX like
                        # read version from library filesystem link and string in shared library matching an approx semver string
                        STRING(REPLACE ".so" "" uEye-SDK_LIBRARY_STRIP "${uEye-SDK_LIBRARIES}")
                        FILE(GLOB uEye-SDK_LIBRARY_VERSIONED "${uEye-SDK_LIBRARY_STRIP}*.so.[0-9].*")
                        STRING(REGEX REPLACE "^.*\.so\." "" uEye-SDK_VERSION_MINOR "${uEye-SDK_LIBRARY_VERSIONED}")
                        FILE(STRINGS ${uEye-SDK_LIBRARIES} uEye-SDK_VERSION_LIST_TWIST REGEX "^${uEye-SDK_VERSION_MINOR}\\.[0-9]+\\.[0-9]+$")
                        FILE(STRINGS ${uEye-SDK_LIBRARIES} uEye-SDK_VERSION_LIST_PATCH REGEX "^${uEye-SDK_VERSION_MINOR}\\.[0-9]+$")
                        IF(uEye-SDK_VERSION_LIST_TWIST)
                                LIST(GET uEye-SDK_VERSION_LIST_TWIST 0 uEye-SDK_VERSION_TWIST)
                                SET(uEye-SDK_VERSION "${uEye-SDK_VERSION_TWIST}")
                        ELSEIF(uEye-SDK_VERSION_LIST_PATCH)
                                LIST(GET uEye-SDK_VERSION_LIST_PATCH 0 uEye-SDK_VERSION_PATCH)
                                SET(uEye-SDK_VERSION "${uEye-SDK_VERSION_PATCH}")
                        ELSE()
                                SET(uEye-SDK_VERSION "${uEye-SDK_VERSION_MINOR}")
                        ENDIF()
                ENDIF() # end plattform specific version detection mechanism
                # handled by FIND_PACKAGE_HANDLE_STANDARD_ARGS
                # # FOUND EVERYTHING WE NEED
                # SET (uEye-SDK_FOUND TRUE)

                # SETUP IMPORTED LIBRARY TARGET
                ADD_LIBRARY(uEye-SDK STATIC IMPORTED)
                SET_TARGET_PROPERTIES(uEye-SDK PROPERTIES IMPORTED_LOCATION ${uEye-SDK_LIBRARIES})
                TARGET_INCLUDE_DIRECTORIES(uEye-SDK INTERFACE ${uEye-SDK_INCLUDE_DIRS})

        ELSE()
                # if individual variables are not OK, but uEye-SDK isn't either,
                # hide them. Try to keep configuration simple for the user
                MARK_AS_ADVANCED(uEye-SDK_INCLUDE_DIRS)
                MARK_AS_ADVANCED(uEye-SDK_LIBRARIES)

                # inform user
                set(uEye-SDK_FIND_INFO_MSG "Set uEye-SDK variable to SDK root, or enable advanced mode and manually set uEye-SDK_INCLUDE_DIRS and uEye-SDK_LIBRARIES")
        ENDIF()

        # handled by FIND_PACKAGE_HANDLE_STANDARD_ARGS
        # IF(uEye-SDK_FOUND)
        #       MESSAGE(STATUS "Found uEye-SDK: ${uEye-SDK_INCLUDE_DIRS}, ${uEye-SDK_LIBRARIES}")
        # ELSE()
        #       IF(uEye-SDK_FIND_REQUIRED)
        #               MESSAGE(FATAL_ERROR "Could not find uEye-SDK, try to setup uEye-SDK accordingly. Or, enable advanced mode and point to the include directory(s) and library(s) manually, using uEye-SDK_INCLUDE_DIRS and uEye-SDK_LIBRARIES.")
        #       ELSE()
        #               MESSAGE(STATUS "uEye-SDK not found.")
        #       ENDIF()
        # ENDIF()

        FIND_PACKAGE_HANDLE_STANDARD_ARGS(uEye-SDK
                REQUIRED_VARS uEye-SDK_INCLUDE_DIRS uEye-SDK_LIBRARIES uEye-SDK_VERSION
                VERSION_VAR uEye-SDK_VERSION
                REASON_FAILURE_MESSAGE "${uEye-SDK_FIND_INFO_MSG}"
        )

ENDIF()
ENDIF()