# Copyright (C) 2016 Hewlett Packard Enterprise Development LP.
# Process this file with Automake to create Makefile.in
#
# File: Makefile.am

AM_CFLAGS =  ${OPSCLI_CFLAGS}
AM_CFLAGS += -I${top_srcdir}/include
AM_CFLAGS += -I$(LOCAL_SYSROOT_DIR)/usr/local/include/ovs -DOPS

libdir=${prefix}/lib/cli/plugins

lib_LTLIBRARIES = liblldp_cli.la

liblldp_cli_la_SOURCES = lldp_vty.c vtysh_ovsdb_lldp_context.c
liblldp_cli_la_LDFLAGS =  -avoid-version
