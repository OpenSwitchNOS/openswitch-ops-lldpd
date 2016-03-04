#include "vswitch-idl.h"
#include "ovsdb-idl.h"

void init_snmp_notifications(struct ovsdb_idl *idl);

void send_lldpRemTablesChange(struct ovsdb_idl *idl, const char *lldpStatsRemTablesInserts_value, const char *lldpStatsRemTablesDeletes_value, const char *lldpStatsRemTablesDrops_value, const char *lldpStatsRemTablesAgeouts_value);
