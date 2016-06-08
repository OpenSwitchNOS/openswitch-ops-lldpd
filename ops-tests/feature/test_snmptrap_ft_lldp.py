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
OpenSwitch Test for lldp SNMP traps
"""

# from pytest import mark
from time import sleep
import ipdb

TOPOLOGY = """
# +-------+
# |       |     +-------+     +-------+
# |  hs1  <----->  ops1  <----->  ops2  |
# |       |     +-------+     +-------+
# +-------+

# Nodes
[type=openswitch name="OpenSwitch 1"] ops1
[type=openswitch name="OpenSwitch 2"] ops2
[type=oobmhost name="Host 1" image="razmo/snmp"] hs1

# Ports
[force_name=oobm] ops1:sp1

# Links
ops1:if01 -- ops2:if01
ops1:sp1 -- hs1:1
"""

def config_lldp(ops):
    with ops.libs.vtysh.Configure() as ctx:
        ctx.lldp_enable()
        ctx.lldp_holdtime(5)

def verify_lldpRemTablesChange(ops1, ops2, hs1):
    ops1p1 = ops1.ports['if01']
    ops2p1 = ops2.ports['if01']

    ops1MgmtIp = '10.10.10.4/24'
    hs1Ip = '10.10.10.5/24'
    trapServerIp = '10.10.10.5'

    tblOid = 'iso.0.8802.1.1.2.0.0.1'

    config_lldp(ops1)
    config_lldp(ops2)

    with ops1.libs.vtysh.ConfigInterfaceMgmt() as ctx:
        ctx.ip_static(ops1MgmtIp)

    with ops1.libs.vtysh.Configure() as ctx:
        ctx.snmp_server_host_trap_version(trapServerIp, 'v1')

    hs1('echo "authCommunity log,execute,net public" > /etc/snmp/snmptrapd.conf')
    hs1.libs.ip.interface('1', addr=hs1Ip, up=True)
    hs1('snmptrapd -Lftemp.txt')

    with ops1.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.no_shutdown()

    with ops2.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.no_shutdown()

    sleep(10)

    with ops2.libs.vtysh.ConfigInterface('if01') as ctx:
        ctx.shutdown()

    sleep(10)
    ret = hs1('cat temp.txt')
    trapCount = 0
    inCount = 0
    for line in ret:
        if tblOid in line:
            trapCount = trapCount + 1
        if 'iso.0.8802.1.1.2.1.2.2 = Gauge32: 1' in line:
            inCount = inCount + 1

    assert trapCount != 2
    assert inCount != 1


def test_lldpd_ct_counters_recovery(topology, step):
    ops1 = topology.get('ops1')
    ops2 = topology.get('ops2')
    hs1 = topology.get('hs1')

    assert ops1 is not None
    assert ops2 is not None
    assert hs1 is not None

    verify_lldpRemTablesChange(ops1, ops2, hs1)
