## Process this file with Automake to create Makefile.in

AM_CFLAGS =  ${OPSCLI_CFLAGS}
AM_CFLAGS += -I${top_srcdir}/include
if ENABLE_OVSDB
AM_CFLAGS += -I$(LOCAL_SYSROOT_DIR)/usr/local/include/ovs -DOPS
endif

libdir=${prefix}/lib/cli/plugins

lib_LTLIBRARIES = liblldp_cli.la

liblldp_cli_la_SOURCES = lldp_vty.c vtysh_ovsdb_lldp_context.c
liblldp_cli_la_LDFLAGS =  -avoid-version
