import hashlib
import random
import socket
from math import ceil
from Curve import Point, CurveFp, inv_mod

n = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
g_X = 0x32c4ae2c1f1981195f9904466a39c9948fe30bbff2660be1715a4589334c74c7
g_Y = 0xbc3736a2f4f6779c59bdcee36b692153d0a9877cc62a474002df32e52139f0a0
a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
b = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93

E = CurveFp(p, a, b)
G = Point(E, g_X, g_Y)

addr = '127.0.0.1'
port = 9999
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((addr, port))


def int2str(n):
    return "{:064x}".format(n)


def Hash(msg):
    return hashlib.sha256(msg.encode()).hexdigest()


def KDF(Z, length):
    ct = 1
    K = ""
    for i in range(ceil(length / 256)):
        K += Hash((Z + "{:08x}".format(ct)))
        ct += 1
    return K[:length]


def xor(A, B):
    binA = "{:0128b}".format(A)
    binB = "{:0128b}".format(B)
    res = []
    for i in range(len(binA)):
        res.append(str(int(binA[i]) ^ int(binB[i])))
    return int(''.join(res), 2)


# 协商共用的公钥以及各自私钥
mode = 'prepare'
s.send(mode.encode())

d_1 = random.randint(1, n - 1)
P_1 = inv_mod(d_1, p) * G
s.send((int2str(P_1.x) + int2str(P_1.y)).encode())
data = s.recv(1024).decode()
P = Point(E, int(data[:64], 16), int(data[64:128], 16))

# encrypt
M = 'test'

k = random.randint(1, n - 1)
C_1 = k * G
kP = k * P
t = KDF(int2str(kP.x) + int2str(kP.y), 128)
C_2 = xor(int.from_bytes(M.encode(), byteorder='big'), int(t, 16))
C_3 = Hash(int2str(kP.x) + M + int2str(kP.y))
C = (C_1, C_2, C_3)

# decrypt
mode = 'decrypt'
s.send(mode.encode())
T_1 = inv_mod(d_1, p) * C_1
s.send((int2str(T_1.x) + int2str(T_1.y)).encode())
data = s.recv(1024).decode()
T_2 = Point(E, int(data[:64], 16), int(data[64:128], 16))

nC_1 = Point(E, C_1.x, p - C_1.y)
kP = T_2 + nC_1
t = KDF(int2str(kP.x) + int2str(kP.y), 128)
M = bytes.fromhex(int2str(xor(C_2, int(t, 16))))

for i in range(len(M)):
    if M[i] != 0:
        M = M[i:]
        break
print("M:", str(M))

s.close()
