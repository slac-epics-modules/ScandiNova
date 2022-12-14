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
DB += waveform.db
DB += registers.db
DB += bridge.db
DB += modbus_registers.db
DB += log_messages.db

#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_template = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

#../O.$(EPICS_HOST_ARCH)/registers.db: ../registers.csv ../db.py
#	python3 ../db.py ../registers.csv registers.db $(TOP)/iocBoot/modbus.cmd.xxx
