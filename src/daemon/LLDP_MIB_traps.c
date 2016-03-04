#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "vswitch-idl.h"
#include "ovsdb-idl.h"


oid objid_enterprise[] = {1, 3, 6, 1, 4, 1, 3, 1, 1};
oid objid_sysdescr[] = {1, 3, 6, 1, 2, 1, 1, 1, 0};
oid objid_sysuptime[] = {1, 3, 6, 1, 2, 1, 1, 3, 0};
oid objid_snmptrap[] = {1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0};

static int status;

void init_snmp_notifications(struct ovsdb_idl *idl) {
//  ovsdb_idl_add_table(idl, &ovsrec_snmp_trap);
//  ovsdb_idl_add_column(idl, &ovsrec_snmp_trap_col_trap_community_name);
//  ovsdb_idl_add_column(idl, &ovsrec_snmp_trap_col_trap_receiver_address);
//  ovsdb_idl_add_column(idl, &ovsrec_snmp_trap_col_trap_receiver_port);
//  ovsdb_idl_add_column(idl, &ovsrec_snmp_trap_col_trap_type);
}

//static oid objid_lldpRemTablesChange[] = {1, 0, 8802, 1, 1, 2, 0, 0, 1};
static oid objid_lldpStatsRemTablesInserts[] = {1, 0, 8802, 1, 1, 2, 1, 2, 2};
static oid objid_lldpStatsRemTablesDeletes[] = {1, 0, 8802, 1, 1, 2, 1, 2, 3};
static oid objid_lldpStatsRemTablesDrops[] = {1, 0, 8802, 1, 1, 2, 1, 2, 4};
static oid objid_lldpStatsRemTablesAgeouts[] = {1, 0, 8802, 1, 1, 2, 1, 2, 5};
void send_lldpRemTablesChange(struct ovsdb_idl *idl, const char *lldpStatsRemTablesInserts_value,
                              const char *lldpStatsRemTablesDeletes_value,
                              const char *lldpStatsRemTablesDrops_value,
                              const char *lldpStatsRemTablesAgeouts_value) {
//  const struct ovsrec_snmp_trap *trap_row = ovsrec_snmp_trap_first(idl);
//  if (trap_row == NULL) {
//    VLOG_ERR(
//        "ovsrec_snmp_trap_first failed to return trap row in send_trap_pdu");
    //return NULL;
//  }

//  OVSREC_SNMP_TRAP_FOR_EACH(trap_row, idl) {
    netsnmp_session session, *ss;
    netsnmp_pdu *pdu, *response;
//    oid name[MAX_OID_LEN];
    int inform = 0;
    SOCK_STARTUP;
    snmp_sess_init(&session);
    char comm[] = "public";
    session.community = comm;
    session.community_len = strlen(session.community);
/*
    const char *trap_type = trap_row->trap_type;
    if (strcmp(trap_type, OVSREC_SNMP_TRAP_TRAP_TYPE_V1_TRAP) == 0) {
      session.version = SNMP_VERSION_1;
      pdu = snmp_pdu_create(SNMP_MSG_TRAP);
    } else if (strcmp(trap_type, OVSREC_SNMP_TRAP_TRAP_TYPE_V2C_TRAP) == 0) {
      session.version = SNMP_VERSION_2c;
      pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
    } else if (strcmp(trap_type, OVSREC_SNMP_TRAP_TRAP_TYPE_V2C_INFORM) == 0) {
      session.version = SNMP_VERSION_2c;
      inform = 1;
      pdu = snmp_pdu_create(SNMP_MSG_INFORM);
    } else if (strcmp(trap_type, OVSREC_SNMP_TRAP_TRAP_TYPE_V3_TRAP) == 0) {
      session.version = SNMP_VERSION_3;
      pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
    } else if (strcmp(trap_type, OVSREC_SNMP_TRAP_TRAP_TYPE_V3_INFORM) == 0) {
      session.version = SNMP_VERSION_3;
      inform = 1;
      pdu = snmp_pdu_create(SNMP_MSG_INFORM);
    }
*/
    session.version = SNMP_VERSION_2c;
    pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
    if (pdu == NULL) {
      fprintf(stderr, "Failed to create notification PDUn");
      SOCK_CLEANUP;
      return;
    }

/*    char temp_port[10];
    snprintf(temp_port, 10, "%d", trap_row->trap_receiver_port);
    char *temp_peername = (char *)malloc(
        strlen(trap_row->trap_receiver_address) + strlen(temp_port) + 1);
    strcpy(temp_peername, trap_row->trap_receiver_address);
    strcat(temp_peername, temp_port);
    session.peername = temp_peername;*/
    session.peername = "localhost";
    ss = snmp_add(&session,
                  netsnmp_transport_open_client("snmptrap", session.peername),
                  NULL, NULL);
    if (ss == NULL) {
      snmp_sess_perror("snmptrap", &session);
//      VLOG_ERR("Failed in snmp_add for %s", session.peername);
//      SOCK_CLEANUP;
//      if (!temp_peername)
//        free(temp_peername);
      return;
    }

    long sysuptime;
    char csysuptime[20];

    sysuptime = get_uptime();
    sprintf(csysuptime, "%ld", sysuptime);
    snmp_add_var(pdu, objid_sysuptime, sizeof(objid_sysuptime) / sizeof(oid),
                 't', csysuptime);
    if (snmp_add_var(pdu, objid_snmptrap, sizeof(objid_snmptrap) / sizeof(oid),
                     'o', "1.0.8802.1.1.2.0.0.1") != 0) {
      SOCK_CLEANUP;
//      if (!temp_peername)
//        free(temp_peername);
      return;
    }

    if (snmp_add_var(pdu, objid_lldpStatsRemTablesInserts,
                     sizeof(objid_lldpStatsRemTablesInserts) / sizeof(oid), 'i',
                     lldpStatsRemTablesInserts_value) != 0) {
      SOCK_CLEANUP;
//      if (!temp_peername)
//        free(temp_peername);
      return;
    }

    if (snmp_add_var(pdu, objid_lldpStatsRemTablesDeletes,
                     sizeof(objid_lldpStatsRemTablesDeletes) / sizeof(oid), 'i',
                     lldpStatsRemTablesDeletes_value) != 0) {
      SOCK_CLEANUP;
//      if (!temp_peername)
//        free(temp_peername);
      return;
    }

    if (snmp_add_var(pdu, objid_lldpStatsRemTablesDrops,
                     sizeof(objid_lldpStatsRemTablesDrops) / sizeof(oid), 'i',
                     lldpStatsRemTablesDrops_value) != 0) {
      SOCK_CLEANUP;
//      if (!temp_peername)
//        free(temp_peername);
      return;
    }

    if (snmp_add_var(pdu, objid_lldpStatsRemTablesAgeouts,
                     sizeof(objid_lldpStatsRemTablesAgeouts) / sizeof(oid), 'i',
                     lldpStatsRemTablesAgeouts_value) != 0) {
      SOCK_CLEANUP;
//      if (!temp_peername)
//        free(temp_peername);
      return;
    }

    if (inform) {
      status = snmp_synch_response(ss, pdu, &response);
    } else {
      status = snmp_send(ss, pdu) == 0;
    }
    if (status) {
      snmp_sess_perror(inform ? "snmpinform" : "snmptrap", ss);
      if (!inform) {
        snmp_free_pdu(pdu);
      }
    } else if (inform) {
      snmp_free_pdu(response);
    }

    snmp_close(ss);
//  }

  snmp_shutdown("snmpapp");
//  if (!temp_peername)
//    free(temp_peername);
  SOCK_CLEANUP;
}
