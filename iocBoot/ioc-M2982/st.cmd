#!../../bin/rhel6-x86_64/lnrf_mod

< envPaths

cd ${TOP}

epicsEnvSet("DB_TOP", "$(TOP)/db")
epicsEnvSet("IOCSH_TOP", "$(MODULES)/iocsh")
epicsEnvSet("ENGINEER", "egumtow")
epicsEnvSet("LOCATION", "B084 158")
epicsEnvSet("IOCNAME", "ioc-M2982")

epicsEnvSet("SYSTEM", "LNRF")
epicsEnvSet("BUILDING", "B084")
epicsEnvSet("POSITION", "158")
epicsEnvSet("P", "$(SYSTEM):$(BUILDING):$(POSITION)")

# Register all support components
dbLoadDatabase("dbd/lnrf_mod.dbd",0,0)
lnrf_mod_registerRecordDeviceDriver(pdbbase)

# this IP:port are for a simulator and must be changed
drvAsynIPPortConfigure("mod-beckhoff-plc", "127.0.0.1:1502", 0, 0, 1)
modbusInterposeConfig("mod-beckhoff-plc", 0, 0, 0)

< "iocBoot/$(IOCNAME)/modbus.cmd"

parse_resources_xml("./iocBoot/$(IOCNAME)/Resource.xml")
set_event_output_file("./scandinova-modulator-events-$(P)")

# Load record instances
dbLoadRecords("db/M2982_registers.db","P=$(P)")
dbLoadRecords("db/waveform.db","P=$(P)")
dbLoadRecords("db/waveform_bridge.db","P=$(P)")
dbLoadRecords("db/customer_waveform.db","P=$(P)")
dbLoadRecords("db/log_messages.db", "P=$(P)")
dbLoadRecords("db/log_messages_bridge.db","P=$(P)")
dbLoadRecords("db/modbus_registers.db", "P=$(P)")
dbLoadRecords("db/watchdog_bridge.db","P=$(P)")

cd ${TOP}/iocBoot/${IOC}

iocInit()
