# Linac Modulator ModbusTCP IOC

IOC to communicate with linac Scandinova modulators over Modbus.

## Requirements

- EPICS Modules
  - asyn
  - modbus
  - pyDevSup

## File Setup

- [docs](./docs)
  - [ScandiCAT ModbusTCP.xml](./docs/ScandiCAT%20ModbusTCP.xml) is a register map from Scandinova
  - [csv-gen.py](./docs/csv-gen.py) generates a CSV file from the register map. PV names, descriptions, etc. have to be added but it is a start
    - The generated csv eventually is used after modifications in the epics database generation
    - cp docs/scandinova-blah-blah.csv lnrf_mod/Db/registers.csv
  - Also contains PDF documents about the Scandinova control system
- [./lnrf_modApp/Db/db.py](./lnrf_modApp/Db/db.py)
  - Automatically run by EPICS build process, it creates db/registers.db and iocBoot/modbus.cmd
  - Uses lnrf_mod/Db/registers.csv

## Installation and Setup

- Get the following files off of the Beckhoff PLC
  - C:\ScandiCAT GUI\Contents\Resource.xml
  - C:\ScandiCAT GUI\Contents\MatrixInfo.xml
  - C:\Mbus\ScandiCAT ModbusTCP.xml
- Place ScandiCAT ModbusTCP.xml in $(TOP)/docs
- Run $(TOP)/docs/csv-gen.py to create a csv file
  - Fill out PV names, alarm info, etc in csv
  - Copy csv to $(TOP)/lnrf_modApp/Db/registers.csv
- Compile the IOC, it should create a $(TOP)/db/registers.db file


## pyDevSup

Large amounts of the event registers mean nothing without being combined with others. To accomplish this, the pyDevSup module is used (http://mdavidsaver.github.io/pyDevSup/)
