/* LLDP CLI commands header file
 *
 * Copyright (C) 2015-2016 Hewlett Packard Enterprise Development LP
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * File: lldp_vty.h
 *
 * Purpose:  To add declarations required for lldp_vty.c
 */

#ifndef _LLDP_VTY_H
#define _LLDP_VTY_H

#include "ops-utils.h"

#define CONFIG_LLDP_STR "Configure LLDP parameters\n"
#define SHOW_LLDP_STR "Show various LLDP settings\n"
#define OVSDB_LLDP_INTF_ROW_FETCH_ERROR "No interface found"
/* As of now same helpstring for both CONFIG and INTERFACE context
 * subjected to change when more commands are added
 */
#define INTF_LLDP_STR CONFIG_LLDP_STR
#define LLDP_TIMER_MAX_STRING_LENGTH 10
#define LLDP_STR_CHASSIS_TLV_LENGTH 10
#define LLDP_MAX_BUF_SIZE 256
#define INTF_NAME_SIZE 20

void cli_pre_init(void);
void cli_post_init(void);

#endif /* _LLDP_VTY_H */
