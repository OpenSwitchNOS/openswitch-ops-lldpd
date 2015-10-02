[toc]
# Component design of LLDP
The "lldpd" project from https://vincentbernat.github.io/lldpd/ is used for ops-lldpd daemon. This daemon is responsible for advertising and receiving LLDP packets from neighbors. The ability to interface with OVSDB for configurations and to push neighbor updates and statistics was added in the OVSDB Interface layer.

## OVSDB Integration
The lldpd daemon is a single threaded daemon and uses libevent which calls back associated callback functions for events.
Daemons which need to interface with OVSDB using OVSDB-IDL use the poll_loop structure to populate sockets and timeouts events. All "run" and "wait" functions are called inside a while(1) {..} loop and we finally wait on events using poll_block().

```
Example OVS Loop

while(!exiting)
{
    run functions
    wait functions - adds sockets and timers to poll_loop
    poll_block - waits for events
}
```
To integrate lldpd with OVSDB the OVS style poll_loop was integrated into the libevent style looping in lldpd:
- One callback function "ovs_libevent_cb" registered with libevent for all OVSDB events (notifications, timers)
- After calling the "wait" functions, which will update the poll_loop structure with the sockets and timeouts, we will register these sockets and timeouts with libevent
- The "ovs_libevent_cb" will call the "run" functions, "wait" functions and then re-register "ovs_libevent_cb" for new set of socket/timeout events.

```
Pseudo-code for ovs_libevent_cb

ovs_libevent_cb(..)
{
    ovs_clear_libevents
    Run all run() functions
    Run all wait() functions
    Register poll_loop events with libevent
    Register poll_loop timeout with libevent
}
```

##Daemon interactions

```ditaa
+----------------------------------------+      +------------+
|               LLDP Daemon              |      |    OVSDB   |
|  +--------------+   +---------+ Global Configs| +--------+ |
|  |              |   |         +<----------------+ System | |
|  |              |   | LLDPD   |  Global stats | | TABLE  | |
|  |  Open Source |   | OVSDB   | -------+------> |        | |
|  |    lldpd     +<->+Interface|        |      | +--------+ |
|  |              |   |         | Interface cfg | +--------+ |
|  |              |   |         +<--------------+-+        | |
|  |              |   |         | Neighbors/stats |        | |
|  +--------------+   +---------+ --------------> |Interface |
+---^-----^------------------------^-----+      | |  Table | |
    |     |                        |            | |        | |
    |     |    LLDP frames rx/tx   |            | +--------+ |
    v     v                        v            +------------+
+---+-----+------------------------+------+
|                KERNEL                   |
|  +-+   +-+  . . . . . . . .     +-+     |
|  +-+   +-+   interfaces         +-+     |
|                                         |
+-----------------------------------------+

```

## OVSDB-Schema
### System table
```
System:other_config
Keys:
lldp_enable
lldp_tlv_mgmt_addr_enable
lldp_tlv_port_desc_enable
lldp_tlv_port_vlan_enable
lldp_tlv_sys_cap_enable
lldp_tlv_sys_desc_enable
lldp_tlv_sys_name_enable
lldp_mgmt_addr
lldp_tx_interval
lldp_hold

System:statistics
Keys:
lldp_table_inserts
lldp_table_deletes
lldp_table_drops
lldp_table_ageouts
```

### Interface table
```
Interface:other_config
Keys:
lldp_enable_dir ("off","rx","tx","rxtx")

Interface:status
Keys:
lldp_local_port_vlan
lldp_local_port_desc

Interface:statistics
Keys:
lldp_tx
lldp_rx
lldp_rx_discard
lldp_rx_tlv_disc

Interface:lldp_neighbor_info
Keys:
port_description
port_id
port_protocol
port_pvid
chassis_description
chassis_id
chassis_id_subtype
chassis_name
chassis_capability_available
chassis_capability_enabled
mgmt_ip_list
mgmt_iface_list
vlan_name_list
vlan_id_list
```

## Code Design
The OVSDB Interface component for ops-lldpd is enabled using --enable-ovsdb configuration option. Code added into original source lldpd source files for this enablement are under ENABLE_OVSDB macro

The OVSDB interface layer code resides in lldpd_ovsdb_if.c
Purpose of this layer : Main file for integrating lldpd with ovsdb and ovs poll-loop.

Its purpose in life is to provide hooks to lldpd daemon to do following:

1. During start up, read lldpd related configuration data and apply to lldpd.
2. During operations, receive administrative configuration changes and apply to lldpd config.
3. Update statistics and neighbor tables periodically to database and sync up statistics and neighbor info to internal lldp's data structure on lldpd restart.

### OVSDB initialization
The ops-lldpd registers with OVSDB tables and columns for lldp configuration notifications.

###Global configs
The lldp source stores global configurations in  "struct lldpd", which we update on getting config updates from OVSDB.
All global configurations in OVSDB in System:user_config column such as lldp_enable, lldp_mgmt_addr, lldp_hold, lldp_tx_interval, lldp_tlv_mgmt_addr_enable are handled in "lldpd_apply_global_changes".

### Per interface configurations

We keep hash of all interfaces in the system in the "all_interfaces" hash. This is a hash which stores the following data structure for each interface.
```
struct interface_data {
        char *name;                 /* Always non null */
        int native_vid;             /* "tag" column - native VLAN ID. */
        int synced_from_db;
        const struct ovsrec_interface *ifrow;       /* Handle to ovsrec row */
        struct port_data *portdata; /* Handle to port data */
        struct lldpd_hardware *hw;  /* Handle to lldp hardware interface */
};
```
This holds a pointer to the OVSDB row entry of interface and corresponding lldpd_hardware which is maintained by lldpd.

We use this to configure per interface level configuration lldp_enable_dir. The per interface configuration is handled in "lldpd_apply_interface_changes".

###Neighbor table syncronization

LLDP triggers a neighbor info change by setting a change request in lldp's interface and sending a libevent message to schedule a database update.

LLDP scheduling logic makes sure no outstanding transaction is in progress as well as previous neighbor transaction succeeded. It then, scans all LLDP ports and looks for neighbor changes triggered by LLDPD. The following changes are supported: ADD, MOD, UPDATE and DELETE. Update is a keep alive refresh and only requires an update time change ADD/MOD writes/rewrites the entire neighbor table to OVSDB DEL deletes neighbor info from OVSDB.

LLDP scans all LLDP port, again, looking for and aged out neighbor info. Stale neighbor info is detected by checking update time against current time. In this case the neighbor cell gets deleted. This will cover any corner cases, like port disconnect or restart, in which lldpd fails to report aged out nbr entries in a timely manner.

Any neighbor transaction failure triggers a full sync up of LLDP internal neighbor info to the database by copying LLDP table info from LLDP to OVSDB for each active LLDP interface.

If LLDP feature is disabled, neighbor info will be deleted from all interfaces.


####LLDP Neighbor decode Functions
LLDPD internal structures are strings, numbers, network addresses, opcodes and bitmasks. Strings are stores as strings, numbers are converted to strings and network addresses are parsed and converted to strings.

Opcodes and bitmasks use a decode table to convert an opcode into a string and a bitmask into a list of strings, a string for each bit in the mask.

Examples:

Management address is an opcode with possible values:
NONE -   0
IP4  -   1
IP6  -   2

lldp_decode_table[IP_ADDRESS_BASE]   = "None"
lldp_decode_table[IP_ADDRESS_BASE+1] = "IPv4"
lldp_decode_table[IP_ADDRESS_BASE+2] = "IPv6"

Port capability is a logical or of the following modes:
OTHER     -  0x01
REPEATER  -  0x02
BRIDGE    -  0x04
WLAN      -  0x08
ROUTER    -  0x10
TELEPHONE -  0x20
DOCSIS    -  0x40
STATION   -  0x80

lldp_decode_table[CAPABILITY_BASE+1]   = "Other"
lldp_decode_table[CAPABILITY_BASE+2]   = "Repeater"
lldp_decode_table[CAPABILITY_BASE+3]   = "Bridge"
lldp_decode_table[CAPABILITY_BASE+4]   = "WLAN"
lldp_decode_table[CAPABILITY_BASE+5]   = "Router"
lldp_decode_table[CAPABILITY_BASE+6]   = "Telephone"
lldp_decode_table[CAPABILITY_BASE+7]   = "DOCSIS"
lldp_decode_table[CAPABILITY_BASE+8]   = "Station"

####LLDP Neighbor key-value pair updates
Database updates parses every lldpd's interface field and decodes it according to field type. Each field is stored a key-value pair in neighbor info. Linked list are broken into multiple lists of structure members, each list is stored as a comma-seperated linked list of strings.

###Statistics and Counter transactions
Statistic reporting is scheduled every "lldpd_stats_check_interval" seconds and if somehow it gets called before that time has elapsed, it reschedules itself to be called at precisely that interval. The main worker function for updating statistics to OVSDB is done in "lldpd_stats_analyze"

#References
http://vincentbernat.github.io/lldpd/
