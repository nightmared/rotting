import matplotlib
from random import random
from matplotlib import cbook
from matplotlib import cm
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
from time import time
import numpy as np

wx = np.linspace(-5, 5, 150);
wy = np.linspace(-5, 5, 150);
WX, WY = np.meshgrid(wx, wy)


W, w4, w5 = [(random()-0.5)*8 for _ in range(6)], (random()-0.5)*2, (random()-0.5)*2
b1, b2, b3 = (random()-0.5)*2, (random()-0.5)*2, (random()-0.5)*2

matplotlib.rc('xtick', labelsize=15)
matplotlib.rc('ytick', labelsize=15)
fig, ax = plt.subplots(figsize=(16, 9))
#plt.subplots_adjust(bottom=0.2)
def compute(eta):
    global W, b1, b2, b3
    X, Y = [], []
    X.append(W[0])
    Y.append(W[3])
    for i in range(75):
        inputs = [int(random()*2), int(random()*2), int(random()*2)]
        target = inputs[0]&inputs[1]&inputs[2]
        y1 = W[0] * inputs[0] + W[1] * inputs[1] + W[2] * inputs[2] + b1
        y2 = W[3] * inputs[0] + W[3+1] * inputs[1] + W[3+2] * inputs[2] + b2
        yf = w4 * y1 + w5 * y2 + b3

        #db3 = -eta * (yf-target) 
        #dw4 = -eta * (yf-target)*(y1) 
        #dw5 = -eta * (yf-target)*(y2) 
        #db1 = db3 * w4
        #db2 = db3 * w5
        #dW = [0]*6
        #for i in range(6):
        #    if i < 3:
        #        dW[i] = -eta * (yf-target) * w4 * inputs[i]
        #    else:
        #        dW[i] = -eta * (yf-target) * w4 * inputs[i-3]
        #
        #for i in range(6):
        #    W[i] += dW[i]
        dW0 = -eta * (yf-target) * w4 * inputs[0]
        dW3 = -eta * (yf-target) * w5 * inputs[0]

        W[0] += dW0 
        W[3] += dW3

        #b1 += db1
        #b2 += db2
        #b3 += db3
        X.append(W[0])
        Y.append(W[3])

    WZ = 1/2 * ((b3+w4*b1+w5*b2)**2 + (b3+w4*(WX+b1)+w5*(WY+b2))**2 + (b3+w4*(W[1]+b1)+w5*(W[4]+b2))**2 + (b3+w4*(W[2]+b1)+w5*(W[5]+b2))**2 + (b3+w4*(WX+W[1]+b1)+w5*(WY+W[4]+b2))**2 + (b3+w4*(WX+W[2]+b1)+w5*(WY+W[5]+b2))**2 + (b3+w4*(W[1]+W[2]+b1)+w5*(W[4]+W[5]+b2))**2 + (b3+w4*(WX+W[1]+W[2]+b1)+w5*(WY+W[4]+W[5]+b2)-1)**2)
    return (X, Y, WZ)


def draw(color, X, Y, WZ):
    plt.plot(X, Y, 'o', c='black', markersize=5)
    plt.contourf(WX, WY, WZ, cmap=cm.viridis, levels=[i for i in range(0, 100, 5)])
    plt.xlabel(r'$w_{11}^1$', fontsize=18)
    plt.ylabel(r'$w_{31}^1$', fontsize=18, rotation=0)
    plt.title("Rétropropagation sur une porte logique AND", fontsize=20)
    plt.legend()

X, Y, WZ = compute(0.25)
draw('black', X, Y, WZ)
#axis = plt.axes([0.13, 0.08, 0.65, 0.03])
#slider = Slider(axis, r'$w_3^1$', -2.5, 2.5, valinit=0)
#plt.sca(fig.axes[0])
#
#def update(val):
#    plt.gca().cla()
#    X, Y, WZ = compute(val, 0.05)
#    draw('black', X, Y, WZ)
#    #plt.savefig("graph-"+str(time())+".png", dpi=256)
#
#slider.on_changed(update)

plt.colorbar()
plt.savefig("graph.png", dpi=256)
plt.show()
