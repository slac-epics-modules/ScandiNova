#!../../bin/linux-x86_64/lnrf_mod

#- You may have to change lnrf_mod2 to something else
#- everywhere it appears in this file

< envPaths

cd ${TOP}

epicsEnvSet("DB_TOP", "$(TOP)/db")
epicsEnvSet("IOCSH_TOP", "$(MODULES)/iocsh")
epicsEnvSet("ENGINEER", "tford")
epicsEnvSet("LOCATION", "Linac Modulator 2")
epicsEnvSet("WIKI", "LinacModulator")
epicsEnvSet("IOCNAME", "lnrf_mod2")
epicsEnvSet("P", "LNRF:")
epicsEnvSet("R", "Mod2:")

## Register all support components
dbLoadDatabase("dbd/lnrf_mod.dbd",0,0)
lnrf_mod_registerRecordDeviceDriver(pdbbase) 

drvAsynIPPortConfigure("mod-beckhoff-plc", "131.243.89.170:502", 0, 0, 1)
#drvAsynIPPortConfigure("mod-beckhoff-plc", "10.1.1.222:502", 0, 0, 1)
modbusInterposeConfig("mod-beckhoff-plc", 0, 0, 0)

< "iocBoot/modbus.cmd"

py "import eventAndWaveform"
py "eventAndWaveform.build('$(P)$(R)', '$(TOP)', '/home/als/physbase/phoebus/accelerator/gtb/linac/mod/events')"

## Load record instances
dbLoadRecords("db/registers.db","P=$(P),R=$(R)")
dbLoadRecords("db/waveform.db","P=$(P),R=$(R)")
dbLoadRecords("db/py.db","P=$(P),R=$(R)")
iocshLoad("$(IOCSH_TOP)/als_default.iocsh")

# access control list
asSetFilename("$(TOP)/access_security/lnrf_mod.acf")

cd ${TOP}/iocBoot/${IOC}

iocInit()

dbpf $(P)$(R)RFReturnLoss.HIHI 85 

dbl >/vxboot/PVnames/${IOCNAME}
epicsEnvShow > /vxboot/PVenv/${IOCNAME}.softioc
