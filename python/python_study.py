import secrets
import random
import math
import threading
import time
import numpy as np

print("hellow")
random.seed(time.localtime)
bit = 48
while True:
    num = random.randrange(2 ** (bit-1), 2 ** bit - 1)
    if (num % 2 != 0):
        break


isPrime = False
while isPrime == False:
    while True:
        num = random.randrange(2 ** (bit - 1), 2 ** bit - 1)
        if (num % 2 != 0):
            break
    limit = math.sqrt(float(num))
    temp = 3
    print("num: {}".format(num))
    while (isPrime == False):
        if (num % temp == 0):
            break
        if (temp > limit):
            isPrime = True
            break
        temp += 2

    
print("bit num: {}".format(num))