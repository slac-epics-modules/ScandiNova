#!../../bin/rhel6-x86_64/lnrf_mod

#- You may have to change lnrf_mod1 to something else
#- everywhere it appears in this file

< envPaths

cd ${TOP}

epicsEnvSet("DB_TOP", "$(TOP)/db")
epicsEnvSet("IOCSH_TOP", "$(MODULES)/iocsh")
epicsEnvSet("ENGINEER", "egumtow")
epicsEnvSet("LOCATION", "B084 158")
epicsEnvSet("WIKI", "LinacModulator")
epicsEnvSet("IOCNAME", "sioc-sim-B084-158")

epicsEnvSet("SYSTEM", "LNRF")
epicsEnvSet("BUILDING", "B084")
epicsEnvSet("POSITION", "158")
epicsEnvSet("P", "$(SYSTEM):$(BUILDING):$(POSITION)")

## Register all support components
dbLoadDatabase("dbd/lnrf_mod.dbd",0,0)
lnrf_mod_registerRecordDeviceDriver(pdbbase)

drvAsynIPPortConfigure("mod-beckhoff-plc", "127.0.0.1:1502", 0, 0, 1)
modbusInterposeConfig("mod-beckhoff-plc", 0, 0, 0)

< "iocBoot/$(IOCNAME)/modbus.cmd"

#py "import eventAndWaveform"
#py "eventAndWaveform.build('$(P)$(R)', '$(TOP)', '/u/gu/egumtow/scandinova-modulator-events')"
parse_resources_xml("./iocBoot/$(IOCNAME)/Resource.xml")
set_event_output_file("./scandinova-modulator-events-$(P)")


## Load record instances
dbLoadRecords("db/registers.db","P=$(P)")
dbLoadRecords("db/waveform.db","P=$(P)")
dbLoadRecords("db/waveform_bridge.db","P=$(P)")
dbLoadRecords("db/customer_waveform.db","P=$(P)")
dbLoadRecords("db/log_messages.db", "P=$(P)")
dbLoadRecords("db/log_messages_bridge.db","P=$(P)")
dbLoadRecords("db/modbus_registers.db", "P=$(P)")
dbLoadRecords("db/watchdog_bridge.db","P=$(P)")
#iocshLoad("$(IOCSH_TOP)/als_default.iocsh")

# access control list
#asSetFilename("$(TOP)/access_security/lnrf_mod.acf")

cd ${TOP}/iocBoot/${IOC}

iocInit()

#dbpf $(P):SolenoidPS1Current.HIHI 42
#dbpf $(P):SolenoidPS1Current.HIGH 41.8
#dbpf $(P):SolenoidPS1Current.LOW 35
#dbpf $(P):SolenoidPS1Current.LOLO 34
#dbpf $(P):SolenoidPS2Current.HIHI 21
#dbpf $(P):SolenoidPS2Current.HIGH 20
#dbpf $(P):SolenoidPS2Current.LOW 18
#dbpf $(P):SolenoidPS2Current.LOLO 17
#dbpf $(P):SolenoidPS2Voltage.LOW 27.5
#dbpf $(P):SolenoidPS2Voltage.LOLO 27


#dbpf $(P):SolenoidPS1CurrentSet.HIHI 42
#dbpf $(P):SolenoidPS1CurrentSet.HIGH 41.8
#dbpf $(P):SolenoidPS1CurrentSet.LOW 35
#dbpf $(P):SolenoidPS1CurrentSet.LOLO 34
#dbpf $(P):SolenoidPS2CurrentSet.HIHI 21
#dbpf $(P):SolenoidPS2CurrentSet.HIGH 20
#dbpf $(P):SolenoidPS2CurrentSet.LOW 18
#dbpf $(P):SolenoidPS2CurrentSet.LOLO 17

#dbpf $(P):SolenoidPS2CurrentSet.DRVH 20

#dbpf $(P):IonPumpCurrent.HIHI 500
#dbpf $(P):IonPumpCurrent.HIGH 300

#dbpf $(P):CapVoltageDivider.HIHI 240
#dbpf $(P):CapVoltageDivider.HIGH 230

#dbpf $(P):RFReturnLoss.HIHI 85
#dbpf $(P):RFForwardPower.HIHI 104
#dbpf $(P):RFForwardPower.HIGH 103

#dbl >/vxboot/PVnames/${IOCNAME}
#epicsEnvShow > /vxboot/PVenv/${IOCNAME}.softioc
