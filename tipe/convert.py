import os, sys
from PIL import Image

# big endian
c1 = lambda x: x.to_bytes(1, byteorder='big')
c2 = lambda x: x.to_bytes(2, byteorder='big')
c3 = lambda x: x.to_bytes(3, byteorder='big')
c4 = lambda x: x.to_bytes(4, byteorder='big')


fd = open('new-bdd', 'r')
lines = fd.readlines()
data = []
for i in range(int(len(lines)/2)):
    data.append([lines[2*i], lines[2*i+1]])
fd.close()

# Gen image 256x256 wide
img_size=64

header = b''
# header_size
header += c1(15)
# input_length
header += c4(img_size*img_size)
# output_length
header += c2(3)
# inf_in
header += c2(0)
# sup_in
header += c2(255)
# inf_out
header += c2(0)
# sup_out
header += c2(255)

out = open('db-bin', 'bw')
out.write(header)
i=0
for e in data:
    im = Image.open(e[0][:-1])
    # Scale values and image
    im = im.resize((img_size, img_size))
    im = im.convert("L")
    out.write(im.tobytes())
    if e[1] == "None\n":
        # 255 if city is present, 0 otherwise
        out.write(c3(0))
    else:
        parts = e[1][1:-2].split(',')
        x, y = int(float(parts[0])*256/1000.), int(float(parts[1])*256/1000.)
        out.write(c3(int(((int(y)*256)+int(x))*256)+img_size-1))
    i+=1
    print(i)

out.close()
