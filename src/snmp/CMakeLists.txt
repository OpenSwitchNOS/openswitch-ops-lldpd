# (c) Copyright 2015 Hewlett Packard Enterprise Development LP
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.

cmake_minimum_required (VERSION 2.8)

set (SRC_SNMP ${PROJECT_SOURCE_DIR}/src/snmp)
project (lldp_snmp)
#set (SRC_DIR src)
set (LIBLLDP_SNMP lldp_snmp)
set (INCL_DIR include)

# This option is specified in Yocto -- openswitch.bbclass.
OPTION( CPU_LITTLE_ENDIAN "Specifies CPU architecture is Little-Endian" OFF )

# Define compile flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Werror -ggdb -g3 -O0")

# Rules to locate needed libraries
include(FindPkgConfig)
pkg_check_modules(OVSCOMMON REQUIRED libovscommon)
pkg_check_modules(OVSDB REQUIRED libovsdb)

include_directories (${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/${INCL_DIR}
                     ${OVSCOMMON_INCLUDE_DIRS}
					)

# SNMP libraries source files
set (SOURCES_LIB ${SRC_SNMP}/lldpMessageTxInterval.c)

# Rules to build SNMP LIBRARY
add_library (${LIBLLDP_SNMP} SHARED ${SOURCES_LIB})

#target_link_libraries (${BROADVIEW}
#                       ${OVSCOMMON_LIBRARIES} ${OVSDB_LIBRARIES}
#                       -lpthread -lrt -lnetsnmpmibs -lpci -ldl -lnetsnmpagent -lnetsnmp -lcrypto -lm)

# Installation
install(TARGETS ${LIBLLDP_SNMP}
        LIBRARY DESTINATION lib/snmp/plugins
       )
