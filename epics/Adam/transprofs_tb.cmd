# EPICS IOC Module
# Name: streamPAC 
# Author: Waldemar Koprek [waldemar.koprek@psi.ch]
#
# The module allows operation of PAC boards with various piggyback modules
# The module uses streamDevice driver for communication over telnet
# Dependencies: The module is compatiple with firmware located here:
#    https://git.psi.ch/GFA/Libraries/BoardSupport/PAC/pac_base
#
# Startup command: streamPAC.cmd DEVICE= PAC= SLOT0= SLOT1= SLOT2= SLOT3=
#   where:
#     DEVICE - Device name which is part before : of all created records
#     PAC    - Hostname of the PAC board
#     SLOTn  - Name of the piggyback module in the specific slot. 
#              Currently supported modules: M06
#              All parameters must be provided. If module is not installed
#                use keyword EMPTY
#
# Deployment example:
#  require streamPAC
#  streamPAC.cmd DEVICE=MY_TEST_DEVICE PAC=PAC0043 SLOT0=EMPTY SLOT1=M06 SLOT2=EMPTY SLOT3=EMPTY
# 
require SynApps

dbLoadTemplate("./transprofs_tb.subs", "DEVICE=MTEST-TRANSPROFS")
