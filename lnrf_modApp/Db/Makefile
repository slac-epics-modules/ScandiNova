TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

# DB_INSTALLS for db outside this IOC's source
DB_INSTALLS += $(devIocStats)/db/iocAdmin.db
DB_INSTALLS += $(AUTOSAVE)/db/save_restoreStatus.db

# DB for db inside this IOC's source
DB += state.template
DB += access.template
DB += registers_simulator.db
DB += modbus_registers.db
DB += log_messages.db
DB += log_messages_bridge.db
DB += waveform.db
DB += waveform_bridge.db
DB += waveform_customer.db
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
