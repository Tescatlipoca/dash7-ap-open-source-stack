# 
# OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
# lowpower wireless sensor communication
#
# Copyright 2015 University of Antwerp
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

include(${PROJECT_SOURCE_DIR}/cmake/module_macros.cmake)

SET(MODULE_EXTRA_MODULES_DIR "" CACHE STRING "Path to directory containing extra, out of tree modules (optional)")

#Add the 'BUILD_MODULES' option
OPTION(BUILD_MODULES "Build Modules" ON)
LIST_SUBDIRS(MODULE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
IF(NOT(MODULE_EXTRA_MODULES_DIR STREQUAL ""))
    LIST_SUBDIRS(EXTRA_MODULES_DIRS ${MODULE_EXTRA_MODULES_DIR})
ENDIF()
LIST(APPEND MODULE_DIRS ${EXTRA_MODULES_DIRS})

#Add conditional options for all subdirs (which are individual modules)
FOREACH(__dir ${MODULE_DIRS})
    GET_FILENAME_COMPONENT(__module_name ${__dir} NAME) # strip full path keeping only module name
    STRING(TOUPPER ${__module_name} __upper_name)
    OPTION(MODULE_${__upper_name} "Build Module ${__module_name}" false)
    UNSET(__upper_name)
ENDFOREACH()

#Hide the parameters declared by the modules during the previous run
#Modules that are enabled will re-enable their parameters during declaration (eg by calling MODULE_PARAM)
HIDE_PARAMETERS(MODULE_PARAM_LIST)

#Finally Load the individual modules
FOREACH(__dir ${MODULE_DIRS})
    GET_FILENAME_COMPONENT(__module_name ${__dir} NAME) # strip full path keeping only module name
    GEN_PREFIX(MODULE_PREFIX "MODULE" ${__module_name})
    IF(BUILD_MODULES AND ${MODULE_PREFIX})
        ADD_SUBDIRECTORY(${__dir} ${CMAKE_CURRENT_BINARY_DIR}/modules/${__module_name})
        MESSAGE(STATUS "Added module " ${__module_name})
    ENDIF()
    UNSET(MODULE_PREFIX)
ENDFOREACH()
