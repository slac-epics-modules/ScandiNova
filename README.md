# Linac Modulator ModbusTCP IOC

IOC to communicate with linac Scandinova modulators over Modbus.

## Requirements

- EPICS Modules
  - asyn
  - modbus
  - pyDevSup

## Installation and Setup

- [./docs](./docs)
  - [./docs/ScandiCAT ModbusTCP.xml](ScandiCAT ModbusTCP.xml) is a register map from Scandinova
  - [./docs/csv-gen.py](csv-gen.py) generates a CSV file from the register map. PV names, descriptions, etc. have to be added but it is a start
    - The generated csv eventually is used after modifications in the epics database generation
  - Also contains PDF documents about the Scandinova control system
- [./lnrf_modApp/Db/db.py](./lnrf_modApp/Db/db.py)
  - Automatically run by EPICS building process, it creates registers.db and modbus.cmd

## pyDevSup

Large amounts of the event registers mean nothing without being combined with others. To accomplish this, the pyDevSup module is used (http://mdavidsaver.github.io/pyDevSup/)
