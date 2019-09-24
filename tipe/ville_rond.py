import pygame as pg
import numpy as np
from random import random, randint, gauss
import matplotlib.pyplot as plt

# Taille de l'image
X, Y = 64, 64
a = 256//X
pg.init()
screen = pg.display.set_mode((X, Y))
#pg.display.set_caption("Ville rond")
screen.fill((30,8,12))

YELLOW = (250,250,250)
BLUE = (1,1,1)

def byte_to_int(fd):
    res = 0
    for i in fd :
        res = 256*res + int(i)
    return res

def int_to_bytes(n, n_bytes) :
    if n < 256 and n_bytes < 2 :
        return bytes([n])
    return int_to_bytes(n//256, n_bytes-1)+bytes([n%256])

def generer_villes() :
    global screen
    screen.fill(BLUE)
    villes = []
    n = randint(1, 20)
    for i in range(n) :
        pos = (randint(1, X)-1, randint(1, Y)-1)
        r = abs(int(gauss(2, 3)))
        pg.draw.circle(screen, YELLOW, pos, r)
        villes.append((r, pos))
    pos = max(villes)[1]
    Pixa = np.array(pg.PixelArray(screen))%256
    return Pixa, (pos[0]*a, pos[1]*a)

def flou(image, b=3) :
    return np.array([[image[max(y-b,0):min(y+b,Y),max(x-b,0):min(x+b,X)].mean() for x in range(X)] for y in range(Y)])

def im_to_bytes(image) :
    s = b""
    for y in image :
        s += bytes([int(x) for x in y])
    return s

im, pos = generer_villes()
plt.imshow(flou(im), cmap="bone")
plt.show()

datafile = open("rond64_bin", "wb")
datafile.write(int_to_bytes(15, 1))
datafile.write(int_to_bytes(X**2, 4))
datafile.write(int_to_bytes(3, 2))
datafile.write(int_to_bytes(0, 8))
for i in range(5000) :
    print(i)
    im, (x, y) = generer_villes()
    datafile.write(im_to_bytes(flou(im)))
    datafile.write(bytes([x]))
    datafile.write(bytes([y]))
    datafile.write(int_to_bytes(X-1, 1))
datafile.close()

pg.quit()
