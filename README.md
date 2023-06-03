# Linac Modulator ModbusTCP IOC

IOC to communicate with linac Scandinova modulators over Modbus.

## Requirements

- Packages (non-EPICS libs)
  - xz
  - zlib
  - xml2

- EPICS Modules
  - asyn
  - modbus

## File Setup

- ScandiNova should've given you a Resource.xml and "ScandiCAT ModbusTCP.xml"
  files.
- Resource.xml is parsed at run time.  You can put it wherever you want.
  Define where it lives in your st.cmd file.
- "ScandiCAT ModbusTCP.xml" is for your reference only.  Use the register
  definitions within it to properly define your *.db, *.definitions, *.template,
  and modbus.cmd files.
- The database files are in the Db directory.
- modbus.cmd is where your st.cmd file lives.
