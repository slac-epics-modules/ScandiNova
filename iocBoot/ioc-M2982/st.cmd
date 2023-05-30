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

epicsEnvSet("LOG_FILE", "/nfs/slac/g/lcls/epics/ioc/data/ioc-M2982/log.html")

# Register all support components
dbLoadDatabase("dbd/lnrf_mod.dbd",0,0)
lnrf_mod_registerRecordDeviceDriver(pdbbase)

# this IP:port are for a simulator and must be changed
drvAsynIPPortConfigure("mod-beckhoff-plc", "127.0.0.1:1502", 0, 0, 1)
modbusInterposeConfig("mod-beckhoff-plc", 0, 0, 0)

< "iocBoot/$(IOCNAME)/modbus.cmd"

parse_resources_xml("./iocBoot/$(IOCNAME)/Resource.xml")
set_event_output_file("./$(LOG_FILE)")

# Load record instances
dbLoadRecords("db/registers_M2982.db","P=$(P)")
dbLoadRecords("db/registers_common.db", "P=$(P)")
dbLoadRecords("db/waveform_bridge.db","P=$(P)")
dbLoadRecords("db/waveform_enable_bridge.db","P=$(P)")
dbLoadRecords("db/waveform_customer.db","P=$(P)")
dbLoadRecords("db/log_messages.db", "P=$(P)")
dbLoadRecords("db/log_messages_bridge.db","P=$(P)")
dbLoadRecords("db/watchdog_bridge.db","P=$(P)")
dbLoadRecords("db/subsystem_status.db","P=$(P)")
dbLoadRecords("db/subsystem_status_bridge.db","P=$(P)")

cd ${TOP}/iocBoot/${IOC}

iocInit()
