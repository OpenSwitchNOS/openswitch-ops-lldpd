
# LLDP Component Test Cases

## Contents

- [Counters/Statistics Recovery Test](#create-vlan-interface)
- [Call Libevent Test](#add-ipv4-address)

##  Counters/Statistics Recovery Test

### Objective

This test verifies that after an lldp daemon crashes and recovers,
its counters and statistics will not reset back to 0 but
will be maintained with the values before such a crash.

### Requirements

- Virtual Mininet Test Setup
- **CT File**: ops-lldpd/tests/test_lldpd_ct_counters_recovery.py

### Setup

The setup requires one switch and at least one host which supports
LLDP.

### Description

The two devices are allowed to run for a time of about 30 seconds
to 1 minute, so that the switch lldp daemon accumulates some
counters and statistics.  Then the LLDP daemon is deliberately killed
and is almost immediately re-started by the sys daemon.  After a few
seconds have elapsed, the counters are checked in the ovsdb to make
sure that they have not been reset to 0 and in fact have at least the
values they had before the crash.

There are no CLI commands to run.  The test is ran simply by running
the python test script (test_lldpd_ct_counters_recovery.py) in the
correct framework.

### Test Result Criteria

#### Test Pass/Fail Criteria

The test script will print out whether the test passed or failed.
The test will pass if the counters/stats in the ovsdb are at least
or greater than the last time their values were just before the
daemon was killed.  It will fail otherwise.  The daemon is expected
to have synchronised its counters to the ovsdb as soon as it has
re-started.

There is no user configuration to perform or configuration output
to monitor.

## Call Libevent Test

### Objective

This test verifies that event scheduling of the LLDP daemon is working
correctly by sending a burst of 100 events and then checking that
each event actually did get processed.

### Requirements

- Virtual Mininet Test Setup
- **CT File**: ops-lldpd/tests/test_lldpd_ct_call_libevent.py

### Setup

The setup requires only one switch.

### Description

The test sends a burst of 100 libevents and waits 2 seconds to allow
these events to get processed by LLDP daemon. It then checks that at
least 100 events got processed.
There are no CLI commands to run.  The test is ran simply by running
the python test script (test_lldpd_ct_call_libevent.py) in the correct
framework.

### Test Result Criteria

#### Test Pass/Fail Criteria

The test passes if 100 or more libevents have arrived from start
of the test to the end of the test.
The test script will print out whether the test passed or failed.
There is no user configuration to perform or configuration output
to monitor.
