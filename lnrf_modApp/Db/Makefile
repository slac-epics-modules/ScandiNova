TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
# Create and install (or just install) into <top>/db
# databases, templates, substitutions like this:

# DB_INSTALLS for db outside this IOC's source
DB_INSTALLS += $(devIocStats)/db/iocAdmin.db
DB_INSTALLS += $(AUTOSAVE)/db/save_restoreStatus.db

# DB for db inside this IOC's source
DB += simulator_registers.db
DB += M2982_registers.db
DB += modbus_registers.db
DB += log_messages.db
DB += log_messages_bridge.db
DB += waveform.db
DB += waveform_bridge.db
DB += customer_waveform.db
DB += watchdog_bridge.db
DB += waveform_enable_bridge.db
DB += subsystem_status.db
DB += subsystem_status_bridge.db

#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_template = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

#../O.$(EPICS_HOST_ARCH)/registers.db: ../registers.csv ../db.py
#	python3 ../db.py ../registers.csv registers.db $(TOP)/iocBoot/modbus.cmd.xxx
