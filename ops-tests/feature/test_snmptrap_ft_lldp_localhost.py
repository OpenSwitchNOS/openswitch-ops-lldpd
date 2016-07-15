# -*- coding: utf-8 -*-
# (C) Copyright 2015 Hewlett Packard Enterprise Development LP
# All Rights Reserved.
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
#
##########################################################################

"""
OpenSwitch Test for lldp SNMP traps on localhost
for tier 2
Author: Avinash(avinash.varma@hpe.com)
Description: Tests for snmp traps in LLDP
"""

# from pytest import mark
from time import sleep

TOPOLOGY = """
#
# +-------+     +-------+
# | ops1  <----->  ops2 |
# +-------+     +-------+
#

# Nodes
[type=openswitch name="OpenSwitch 1"] ops1
[type=openswitch name="OpenSwitch 2"] ops2

# Links
ops1:1 -- ops2:1
"""


def config_lldp(ops):
    with ops.libs.vtysh.Configure() as ctx:
        ctx.lldp_enable()


def disable_lldp(ops):
    with ops.libs.vtysh.Configure() as ctx:
        ctx.no_lldp_enable()


def config_trapreceiver(ops):
    opsmgmtipsub = '10.10.10.4/24'
    opsmgmtip = '10.10.10.4'

    with ops.libs.vtysh.ConfigInterfaceMgmt() as ctx:
        ctx.ip_static(opsmgmtipsub)

    with ops.libs.vtysh.Configure() as ctx:
        ctx.snmp_server_host_trap_version(opsmgmtip, 'v1')

    with ops.libs.vtysh.Configure() as ctx:
        ctx.snmp_server_host_trap_version(opsmgmtip, 'v2c')

    with ops.libs.vtysh.Configure() as ctx:
        ctx.snmp_server_host_inform_version(opsmgmtip, 'v2c')

    ops.libs.vtysh.show_running_config()

    ops('systemctl stop snmptrapd', shell="bash")
    ops('echo "authCommunity log,execute,net public" > \
        /etc/snmp/snmptrapd.conf', shell="bash")
    ops('snmptrapd -Lftemp.txt', shell="bash")


def verify_lldpremtableschange(ops1, ops2):
    ops1p1 = ops1.ports['1']
    ops2p1 = ops2.ports['1']

    # This is main trap oid which is present in the trap output
    remtableschangeoid = 'iso.0.8802.1.1.2.0.0.1'
    # This is the insert counter which is incremented when lldp is enabled.
    # Verify if the count has increased in the trap output
    remtablesinsertoid = 'iso.0.8802.1.1.2.1.2.2'
    remtablesdeletesoid = 'iso.0.8802.1.1.2.1.2.3'
    remtablesageoutsoid = 'iso.0.8802.1.1.2.1.2.5'

    config_lldp(ops1)
    config_lldp(ops2)

    config_trapreceiver(ops1)

    with ops1.libs.vtysh.ConfigInterface(ops1p1) as ctx:
        ctx.no_shutdown()

    with ops2.libs.vtysh.ConfigInterface(ops2p1) as ctx:
        ctx.no_shutdown()

    ops1.libs.vtysh.show_running_config()
    ops2.libs.vtysh.show_running_config()

    sleep(40)
    ops1.libs.vtysh.show_lldp_neighbor_info(ops1p1)
    ops2.libs.vtysh.show_lldp_neighbor_info(ops2p1)

    with ops2.libs.vtysh.ConfigInterface(ops1p1) as ctx:
        ctx.shutdown()

    ops2.libs.vtysh.show_running_config()

    sleep(130)
    ops1.libs.vtysh.show_lldp_neighbor_info(ops1p1)
    ops2.libs.vtysh.show_lldp_neighbor_info(ops2p1)

    with ops2.libs.vtysh.ConfigInterface(ops1p1) as ctx:
        ctx.no_shutdown()

    sleep(40)
    ops1.libs.vtysh.show_lldp_neighbor_info(ops1p1)
    ops1.libs.vtysh.show_lldp_neighbor_info(ops2p1)
    disable_lldp(ops2)

    sleep(130)

    ret = ops1('cat temp.txt', shell="bash")
    trapcount = 0
    incount = deletecount = ageoutcount = 0
    for line in ret.split('\n'):
        if remtableschangeoid in line:
            trapcount = trapcount + 1
        if remtablesinsertoid + ' = Gauge32: 1' in line:
            incount = incount + 1
        if remtablesdeletesoid + ' = Gauge32: 1' in line:
            deletecount = deletecount + 1
        if remtablesageoutsoid + ' = Gauge32: 1' in line:
            ageoutcount = ageoutcount + 1

    assert trapcount == 12, "Total number of traps received \
            is not equal to expected count"
    assert incount >= 3, "Total number of insert received \
            is not equal to expected count"
    assert deletecount >= 3, "Total number of deletes received \
            is not equal to expected count"
    assert ageoutcount >= 3, "Total number of ageouts received \
            is not equal to expected count"


def test_snmptrap_ft_lldp(topology, step):
    ops1 = topology.get('ops1')
    ops2 = topology.get('ops2')

    assert ops1 is not None
    assert ops2 is not None

    verify_lldpremtableschange(ops1, ops2)
