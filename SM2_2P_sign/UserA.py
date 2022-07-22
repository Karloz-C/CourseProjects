import socket
import hashlib
import random
import binascii
from math import gcd, log2, ceil
from Curve import Point, CurveFp, inv_mod

n = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
g_X = 0x32c4ae2c1f1981195f9904466a39c9948fe30bbff2660be1715a4589334c74c7
g_Y = 0xbc3736a2f4f6779c59bdcee36b692153d0a9877cc62a474002df32e52139f0a0
a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
b = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93

E = CurveFp(p, a, b)
G = Point(E, g_X, g_Y)


def int2str(n):
    return "{:064x}".format(n)


sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

addr = '127.0.0.1'
port = 9999

sock.connect((addr, port))

d_1 = random.randint(1, n - 1)
P_1 = inv_mod(d_1, p) * G
sock.send((int2str(P_1.x) + int2str(P_1.y)).encode())

Z = 12345678
msg = 'hello'
e = hashlib.md5((int2str(Z) + msg).encode()).hexdigest()
k_1 = random.randint(1, n - 1)
Q_1 = k_1 * G
sock.send((int2str(Q_1.x) + int2str(Q_1.y) + e).encode())

data = sock.recv(1024).decode()
r, s_2, s_3 = int(data[:64], 16), int(data[64:128], 16), int(data[128:], 16)
s = (d_1 * k_1 * s_2 + d_1 * s_3 - r) % n

sig = (r, s)
print('sign:', sig, sep='\n')
sock.close()
