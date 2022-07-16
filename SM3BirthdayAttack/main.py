from gmssl import sm3
import random

length = 64

h = {}

for i in range(2 ** (length // 2)):
    msg = hex(random.randint(0, 2 ** 512 - 1))[2:]
    hashmsg = sm3.Hash_sm3(msg, 1)[:length // 4]
    if h.get(hashmsg) and h.get(hashmsg) != msg:
        print(h[hashmsg], msg, sep='\n')
    else:
        h[hashmsg] = msg

# 32bit
# 31ba16f96e37ff9824add9f711f10dea0c493eb3cb4b9327f9df7028d50466c57b2def6d49273b5ec2a32fe2b946bf6ef9521c3210651261a619142c3428409f
# 19e5cdc40074afe6cbe3ded556f2c53ca69198bc28e46279beda34e01b5a0bb37ab8d23f423b489ce9acaa4da09d9bbc048e9b1321ec52bc1cf6844dd64722e2
