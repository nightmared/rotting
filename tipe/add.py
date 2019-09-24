import os, sys

out = open('db-bin-all', 'bw')
f2 = open('rond64_bin', 'rb')
out.write(f2.read())
f2.close()
f1 = open('db-bin', 'rb')
out.write(f1.read()[15:])
f1.close()
out.close()
