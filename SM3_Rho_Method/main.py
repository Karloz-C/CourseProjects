from gmssl import sm3
import random

length = 32
upperbound = 2 ** 512

msg1 = sm3.Hash_sm3(hex(random.randint(0, 2 ** 512 - 1))[2:], 1)
msg2 = msg1


def H(msg):
    return sm3.Hash_sm3(msg, 1)


cnt = 0

while True:
    cnt += 1

    h1 = H(msg1)
    h2 = H(H(msg2))

    # print(h1[:length // 4], h2[:length // 4])

    if h1[:length // 4] == h2[:length // 4] and msg1 != H(msg2):
        print(msg1, h1)
        print(H(msg2), h2)
        print(cnt)
        break

    msg1 = h1
    msg2 = h2
