#!/usr/bin/python3
from __future__ import print_function
import csv
import sys

def is_number_tryexcept(s):
    """ Returns True is string is a number. """
    try:
        a=float(s)
        return True,a
    except ValueError:
        return False,0.0

if not (len(sys.argv) == 2 or len(sys.argv) == 4):
    print("Usage: ",sys.argv[0],"<filename.csv> [column_name] [value]")
    exit(1)

# Gross py2/3 hack
if (sys.version_info > (3, 0)):
    ifile  = open(sys.argv[1], "r")
else:
    ifile  = open(sys.argv[1], "rb")
    
reader = csv.reader(ifile, delimiter=',', quotechar='\\')
rownum = 0
retval = 0
for row in reader:    # Save header row.
    if rownum == 0:
        header = row
    else:
        colnum = 0
        for col in row:
            tf,number = is_number_tryexcept(col)
            if ( tf and number < 0.0 ) or ( len(sys.argv) == 4 and sys.argv[2] and sys.argv[3] and header[colnum] == sys.argv[2] and col != sys.argv[3] ):
                print("Invalid value for",header[colnum],col)
                exit(1)
            colnum += 1
    rownum += 1
ifile.close()
exit(retval)
