import csv
import os
import argparse
import datetime
from scandinovaParser import *
import sys
sys.path.insert(1, os.path.dirname(os.path.abspath(__file__)) + '/../py')
import scandinovaUtils

###
# This file creates st.cmd and modbus.db files
# Input is read from scandinova.csv file which is partially created by csv-gen.py
# Lots of business logic is need for some of the XML parsing because of the way
# Scandinova set it up. That logic is separated out into the ModbusParser class in modbusParserHelp.py
###

#path = os.path.dirname(os.path.abspath(__file__))
scandinova = scandinovaUtils.ScandinovaXmlParser('../../py/Resource.xml', '../../py/MatrixInfo.xml')

for k, v in scandinova.message_dict.items():
    scandinova.message_dict[k] = v.replace('AccessLevel: ', '')


def write_from_csv(csv_file, db_file, st_file):
    with open(st_file, 'w') as st_f:
        with open(db_file, 'w') as db_f:
            with open(csv_file, newline='') as csv_f:
                reader = csv.DictReader(csv_f)
                config_cmd = 'drvModbusAsynConfigure("{t}{addr}", "mod-beckhoff-plc", 0, {modbusCmd}, {addr}, {length}, {dtype}, {timeout}, "mod-beckhoff-plc")\n'
                default_timeout = 1000
                curr_addr = -1
                parser = Parser(db_f, st_f, scandinova, config_cmd, default_timeout)
                for row in reader:
                    if row['PVName'] == '':
                        continue
                    start_address = row['StartAddress']
                    end_address = row['EndAddress']
                    readonly = True if row['Readonly'].lower() == 'true' else False
                    data_type = row['Type']
                    length = int(end_address) - int(start_address) + 1
                    if readonly:
                        t = 'read'
                        timeout = default_timeout 
                        code = 4
                    else:
                        t = 'write'
                        timeout = -1
                        if length == 1:
                            code = 6
                        else:
                            code = 16
                    if data_type == 'Uint16' or data_type == 'Word':
                        modbus_type = 0
                    elif data_type == 'Int16':
                        modbus_type = 4
                    elif data_type == 'Single':
                        modbus_type = 7
                    elif data_type == 'Uint32':
                        modbus_type = 5
                    elif data_type == 'Uint64':
                        ## need to fix
                        modbus_type = 0
                    elif data_type == 'Struct':
                        modbus_type = 0
                    cmd = asyn_port_config = config_cmd.format(t=t, addr=start_address, modbusCmd=code, length=length, dtype=modbus_type, timeout=timeout)
                    
                    special_addrs = ['1000', '1100', '1700', '1715', '2000', '2300', '3001']
                    if curr_addr != start_address and start_address not in special_addrs:
                        st_f.write(cmd)

                    # if length is greater than 2 this is not a easily handled register
                    # going to need to do specific stuff based on info in scandinova modbus pdf doc 
                    if length > 2:
                        pvname = row['PVName']
                        # skipping increment registers since they are worthless for us
                        if start_address == '1000':
                            continue
                        elif start_address == '1050':
                            parser.write_event_type(row, length, pvname)
                        elif start_address == '1100':
                            parser.write_event_time(row, start_address, pvname)
                        elif start_address == '1300':
                            parser.write_event_longin_32bit(row, length, pvname)
                        elif start_address == '1400':
                            parser.write_event_longin_16bit(row, length, pvname)
                        elif start_address == '1450':
                            parser.write_event_longin_16bit(row, length, pvname)
                        elif start_address == '1500':
                            parser.write_event_data_type(row, length, pvname, scandinova)
                        elif start_address == '1550':
                            parser.write_event_longin_32bit(row, length, pvname)
                        elif start_address == '1700':
                            parser.write_event(row, start_address, pvname)
                        elif start_address == '1715':
                            parser.write_event(row, start_address, pvname)
                        elif start_address == '2000':
                            parser.write_state_read_array(row, start_address, pvname)
                        elif start_address == '2300':
                            parser.write_status_bit_array(row, start_address, pvname)
                        elif start_address == '3001':
                            parser.write_waveform(row, start_address, pvname)
                        else:
                            print("Start Add: {}  End Add: {}".format(start_address, end_address))
                    else:
                        if row['StringRep'] == 'State':
                            if readonly:
                                rec = mbbi(row, scandinova.state_dict)
                            else:
                                rec = mbbo(row, scandinova.state_dict)
                        elif row['StringRep'] == 'Message':
                            if readonly:
                                rec = mbbi(row, scandinova.message_dict)
                            else:
                                rec = mbbo(row, scandinova.message_dict)
                        elif row['Bit'] != '':
                            if readonly:
                                rec = bi(row)
                            else:
                                rec = bo(row)
                        elif data_type == 'Int16' or data_type == 'Uint16':
                            if readonly:
                                rec = longin(row)
                            else:
                                rec = longout(row)
                        elif data_type == 'Single':
                            if readonly:
                                rec = ai(row)
                            else:
                                rec = ao(row)                        

                        db_f.write(str(rec))
                        curr_addr = row['StartAddress']



if __name__ == "__main__":
    """ Parse command line arguments and call write_from_csv """
    parser = argparse.ArgumentParser()
    parser = argparse.ArgumentParser(description='Generate EPICS database and st.cmd based on CSV')
    parser.add_argument('csv_file_name', action='store', default=None,
                        help='Path to csv file that specifies mapping between PVs and registers')
    parser.add_argument('database_file_name', action='store', default=None,
                        help='Path of the EPICS database file to create')
    parser.add_argument('st_cmd_file_name', action='store', default=None,
                        help='Path of the st.cmd file to create')
    args = parser.parse_args()

    if os.path.isfile(args.csv_file_name):
        write_from_csv(args.csv_file_name, args.database_file_name, args.st_cmd_file_name)
    else:
        print('No file exists at the specified path: {}'.format(args.csv_file_name))
