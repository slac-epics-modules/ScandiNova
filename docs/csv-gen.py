from xml.etree import ElementTree as ET
from enum import Enum
import datetime
import csv
# Expected XML Format
# Configuration
#   Port
#   IpAddr
#   Mapping
#       Input Registers
#           MappingInfo
#               This is where the Modbus register info lives
#       Output Registers
#           MappingInfo
#               This is where the Modbus register info lives

# Creates scandinova.csv file based on ScandiCAT ModbusTCP.xml file
# Additional fields have to be added to CSV file such as PV names
# But this is a good start to get registers into CSV format

def csv_writer():
    tree = ET.parse('ScandiCAT ModbusTCP.xml')
    root = tree.getroot()
    
    with open('scandinova-{}.csv'.format(datetime.datetime.now().strftime("%m-%d-%Y-%H-%M-%S")), 'w', newline='') as csvfile:
        fieldnames = ['StartAddress', 'EndAddress', 'Type', 'Unit', 'Readonly', 'General', 'PVName', 'Description', 'Bit', 'StringRep',  'Function', 'ASG', 'ZNAM', 'ONAM', 'ZSV', 'OSV', 'UNSV', 'ZRSV', 'ONSV', 'TWSV', 'THSV', 'HighHigh', 'High', 'Low', 'LowLow', 'DRVH', 'DRVL', 'Archive']
        csv_writer = csv.DictWriter(csvfile, delimiter=',', fieldnames=fieldnames)

        csv_writer.writeheader()

        for child in root:
            if child.tag == "Mapping":
                row = {}
                for register in child:
                    if register.tag == 'InputRegisters':
                        row['Readonly'] = True
                    elif register.tag == 'OutputRegisters':
                        row['Readonly'] = False
                    for data in register:
                        for elm in data:
                            if elm.tag == 'StartAddress':
                                row['StartAddress'] = int(elm.text)
                            elif elm.tag == 'EndAddress':
                                row['EndAddress'] = int(elm.text)
                            elif elm.tag == 'Comments':
                                for comment_elm in elm:
                                    if comment_elm.tag == 'Type':
                                        row['Type'] = comment_elm.text
                                    elif comment_elm.tag == 'General':
                                        row['General'] = comment_elm.text
                                    elif comment_elm.tag == 'Unit':
                                        row['Unit'] = comment_elm.text
                                    elif comment_elm.tag == 'Function':
                                        row['Function'] = comment_elm.text
                        csv_writer.writerow(row)

if __name__ == "__main__":
    csv_writer()
