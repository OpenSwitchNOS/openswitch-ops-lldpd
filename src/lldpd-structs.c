///* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2008 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "lldpd-structs.h"
#include "log.h"

#ifdef ENABLE_OVSDB
#include "daemon/lldpd_ovsdb_if.h"
#include "vswitch-idl.h"
#endif


void
lldpd_chassis_mgmt_cleanup(struct lldpd_chassis *chassis)
{
	struct lldpd_mgmt *mgmt, *mgmt_next;

	log_debug("alloc", "cleanup management addresses for chassis %s",
	    chassis->c_name ? chassis->c_name : "(unknwon)");

	for (mgmt = TAILQ_FIRST(&chassis->c_mgmt);
	     mgmt != NULL;
	     mgmt = mgmt_next) {
		mgmt_next = TAILQ_NEXT(mgmt, m_entries);
		free(mgmt);
	}
	TAILQ_INIT(&chassis->c_mgmt);
}

void
lldpd_chassis_cleanup(struct lldpd_chassis *chassis, int all)
{
	lldpd_chassis_mgmt_cleanup(chassis);
	log_debug("alloc", "cleanup chassis %s",
	    chassis->c_name ? chassis->c_name : "(unknwon)");
#ifdef ENABLE_LLDPMED
	free(chassis->c_med_hw);
	free(chassis->c_med_sw);
	free(chassis->c_med_fw);
	free(chassis->c_med_sn);
	free(chassis->c_med_manuf);
	free(chassis->c_med_model);
	free(chassis->c_med_asset);
#endif
	free(chassis->c_id);
	free(chassis->c_name);
	free(chassis->c_descr);
	if (all)
		free(chassis);
}

#ifdef ENABLE_DOT1
void
lldpd_vlan_cleanup(struct lldpd_port *port)
{
	struct lldpd_vlan *vlan, *vlan_next;
	for (vlan = TAILQ_FIRST(&port->p_vlans);
	    vlan != NULL;
	    vlan = vlan_next) {
		free(vlan->v_name);
		vlan_next = TAILQ_NEXT(vlan, v_entries);
		free(vlan);
	}
	TAILQ_INIT(&port->p_vlans);
}

void
lldpd_ppvid_cleanup(struct lldpd_port *port)
{
	struct lldpd_ppvid *ppvid, *ppvid_next;
	for (ppvid = TAILQ_FIRST(&port->p_ppvids);
	    ppvid != NULL;
	    ppvid = ppvid_next) {
		ppvid_next = TAILQ_NEXT(ppvid, p_entries);
		free(ppvid);
	}
	TAILQ_INIT(&port->p_ppvids);
}

void
lldpd_pi_cleanup(struct lldpd_port *port)
{
	struct lldpd_pi *pi, *pi_next;
	for (pi = TAILQ_FIRST(&port->p_pids);
	    pi != NULL;
	    pi = pi_next) {
		free(pi->p_pi);
		pi_next = TAILQ_NEXT(pi, p_entries);
		free(pi);
	}
	TAILQ_INIT(&port->p_pids);
}
#endif

/* Cleanup a remote port. The before last argument, `expire` is a function that
 * should be called when a remote port is removed. If the last argument is 1,
 * all remote ports are removed.
 */
void
lldpd_remote_cleanup(struct lldpd_hardware *hardware,
    void(*expire)(struct lldpd_hardware *, struct lldpd_port *),
    int all)
{
	struct lldpd_port *port, *port_next;
	int del;
	time_t now = time(NULL);

	log_debug("alloc", "cleanup remote port on %s",
	    hardware->h_ifname);
	for (port = TAILQ_FIRST(&hardware->h_rports);
	     port != NULL;
	     port = port_next) {
		port_next = TAILQ_NEXT(port, p_entries);
		del = all;
		if (del) {

#ifdef ENABLE_OVSDB
			/*
			* Increment the delete count and clear port entry
			* for all the port cleanup.
			*/
			hardware->h_insert_cnt = 0;
			hardware->h_delete_cnt++;
#endif
			if (expire) expire(hardware, port);
			/* This TAILQ_REMOVE is dangerous. It should not be
			 * called while in liblldpctl because we don't have a
			 * real list. It is only needed to be called when we
			 * don't delete the entire list. */
#ifdef ENABLE_OVSDB
			hardware->h_rport_change_opcode = LLDPD_AF_NBR_DEL;
#endif

			if (!all) TAILQ_REMOVE(&hardware->h_rports, port, p_entries);
			lldpd_port_cleanup(port, 1);
			free(port);
		}
	}
	if (all) TAILQ_INIT(&hardware->h_rports);
}

/* If `all' is true, clear all information, including information that
   are not refreshed periodically. Port should be freed manually. */
void
lldpd_port_cleanup(struct lldpd_port *port, int all)
{
#ifdef ENABLE_LLDPMED
	int i;
	if (all)
		for (i=0; i < LLDP_MED_LOCFORMAT_LAST; i++)
			free(port->p_med_location[i].data);
#endif
#ifdef ENABLE_DOT1
	lldpd_vlan_cleanup(port);
	lldpd_ppvid_cleanup(port);
	lldpd_pi_cleanup(port);
#endif
	/* will set these to NULL so we don't free wrong memory */

	free(port->p_id);
	port->p_id = NULL;
	free(port->p_descr);
	port->p_descr = NULL;
	if (all) {
		free(port->p_lastframe);
		if (port->p_chassis) { /* chassis may not have been attributed, yet */
			port->p_chassis->c_refcount--;
			port->p_chassis = NULL;
		}
	}
}

void
lldpd_config_cleanup(struct lldpd_config *config)
{
	log_debug("alloc", "general configuration cleanup");
	free(config->c_mgmt_pattern);
	free(config->c_cid_pattern);
	free(config->c_iface_pattern);
	free(config->c_hostname);
	free(config->c_platform);
	free(config->c_description);
}
