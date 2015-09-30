
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
and is almost immediately re-startted by the sys daemon.  After a few
seconds have elapsed, the counters are checked in the ovsdb to make
sure that they have not been reset to 0 and in fact have at least the
values they had before the crash.

There are no CLI commands to run.  The test is ran simply by running
the python test script (test_lldpd_ct_counters_recovery.py) in the
correct framework.

### Test Result Criteria

#### Test Pass/Fail Criteria

The test script will print out whether the test passed or failed.
There is no user configuration to perform or configuration output
to monitor.

## Call Libevent Test

### Objective

This test verifies that main loop of the LLDP daemon is working
correctly by sending a burst of 100 events and then checking that
each event actually did get processed.

### Requirements

- Virtual Mininet Test Setup
- **CT File**: ops-lldpd/tests/test_lldpd_ct_call_libevent.py

### Setup

The setup requires only one switch.

### Description

There are no CLI commands to run.  The test is ran simply by running
the python test script (test_lldpd_ct_call_libevent.py) in the correct
framework.

### Test Result Criteria

#### Test Pass/Fail Criteria

The test script will print out whether the test passed or failed.
There is no user configuration to perform or configuration output
to monitor.
