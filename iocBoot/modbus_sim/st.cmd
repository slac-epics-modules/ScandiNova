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
epicsEnvSet("IOCNAME", "scandinova_modulator_sim")
epicsEnvSet("P", "LNRF:")
epicsEnvSet("R", "Mod1:")

## Register all support components
dbLoadDatabase("dbd/lnrf_mod.dbd",0,0)
lnrf_mod_registerRecordDeviceDriver(pdbbase) 

drvAsynIPPortConfigure("mod-beckhoff-plc", "127.0.0.1:1502", 0, 0, 1)
modbusInterposeConfig("mod-beckhoff-plc", 0, 0, 0)

< "iocBoot/modbus.cmd"

py "import eventAndWaveform"
py "eventAndWaveform.build('$(P)$(R)', '$(TOP)', '/u/gu/egumtow/scandinova-modulator-events')"

## Load record instances
dbLoadRecords("db/registers.db","P=$(P),R=$(R)")
dbLoadRecords("db/waveform.db","P=$(P),R=$(R)")
dbLoadRecords("db/py.db","P=$(P),R=$(R)")
iocshLoad("$(IOCSH_TOP)/als_default.iocsh")

# access control list
asSetFilename("$(TOP)/access_security/lnrf_mod.acf")

cd ${TOP}/iocBoot/${IOC}

iocInit()

dbpf $(P)$(R)SolenoidPS1Current.HIHI 42
dbpf $(P)$(R)SolenoidPS1Current.HIGH 41.8 
dbpf $(P)$(R)SolenoidPS1Current.LOW 35 
dbpf $(P)$(R)SolenoidPS1Current.LOLO 34 
dbpf $(P)$(R)SolenoidPS2Current.HIHI 21 
dbpf $(P)$(R)SolenoidPS2Current.HIGH 20 
dbpf $(P)$(R)SolenoidPS2Current.LOW 18 
dbpf $(P)$(R)SolenoidPS2Current.LOLO 17 
dbpf $(P)$(R)SolenoidPS2Voltage.LOW 27.5 
dbpf $(P)$(R)SolenoidPS2Voltage.LOLO 27 


dbpf $(P)$(R)SolenoidPS1CurrentSet.HIHI 42
dbpf $(P)$(R)SolenoidPS1CurrentSet.HIGH 41.8 
dbpf $(P)$(R)SolenoidPS1CurrentSet.LOW 35 
dbpf $(P)$(R)SolenoidPS1CurrentSet.LOLO 34 
dbpf $(P)$(R)SolenoidPS2CurrentSet.HIHI 21 
dbpf $(P)$(R)SolenoidPS2CurrentSet.HIGH 20 
dbpf $(P)$(R)SolenoidPS2CurrentSet.LOW 18 
dbpf $(P)$(R)SolenoidPS2CurrentSet.LOLO 17 

dbpf $(P)$(R)SolenoidPS2CurrentSet.DRVH 20 

dbpf $(P)$(R)IonPumpCurrent.HIHI 500 
dbpf $(P)$(R)IonPumpCurrent.HIGH 300 

dbpf $(P)$(R)CapVoltageDivider.HIHI 240 
dbpf $(P)$(R)CapVoltageDivider.HIGH 230 

dbpf $(P)$(R)RFReturnLoss.HIHI 85 
dbpf $(P)$(R)RFForwardPower.HIHI 104
dbpf $(P)$(R)RFForwardPower.HIGH 103

#dbl >/vxboot/PVnames/${IOCNAME}
#epicsEnvShow > /vxboot/PVenv/${IOCNAME}.softioc
