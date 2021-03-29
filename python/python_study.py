import secrets
import random
import math
import time
# import py_cpp_connect.CModule

print("hellow")
random.seed(time.localtime)
bit = 48
while True:
    num = random.randrange(2 ** (bit-1), 2 ** bit - 1)
    if (num % 2 != 0):
        break


isPrime = False
st_time = time.perf_counter()
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
ed_time = time.perf_counter()
py_time = ed_time - st_time
print("bit num: {}".format(num))
print(f"py_time: {py_time}")

isPrime = False
st_time = time.perf_counter()
while isPrime == False:
    while True:
        num = random.randrange(2 ** (bit - 1), 2 ** bit - 1)
    if (CModule.isPrime(num)):
        isPrime = True
        break
    else:
        continue

ed_time = time.perf_counter()
wrap_time = ed_time - st_time
print("bit num: {}".format(num))
print(f"wrap_time: {wrap_time}")
