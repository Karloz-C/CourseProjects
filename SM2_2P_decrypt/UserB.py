import hashlib
import random
import socket
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

port = 9999

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('127.0.0.1', port))
s.listen(5)


def int2str(n):
    return "{:064x}".format(n)


def Hash(msg):
    return hashlib.sha256(msg.encode()).hexdigest()


def KDF(Z, length):
    ct = 1
    K = ""
    for i in range(ceil(length / 256)):
        K += Hash((Z + str(ct).format("08x")))
        ct += 1
    return K[:length]


d_2 = random.randint(1, n - 1)


def prepare(sock, addr):
    data = sock.recv(1024).decode()
    P_1 = Point(E, int(data[:64], 16), int(data[64:128], 16))
    nG = Point(E, G.x, p - G.y)
    P = inv_mod(d_2, p) * P_1 + nG
    sock.send((int2str(P.x) + int2str(P.y)).encode())
    print("prepare finish")


def decrypt(sock, addr):
    data = sock.recv(1024).decode()
    T_1 = Point(E, int(data[:64], 16), int(data[64:128], 16))
    T_2 = inv_mod(d_2, p) * T_1
    sock.send((int2str(T_2.x) + int2str(T_2.y)).encode())
    print("decrypt finish")


while True:
    sock, addr = s.accept()
    while True:
        mode = sock.recv(1024).decode()
        if mode == 'prepare':
            prepare(sock, addr)
        if mode == 'decrypt':
            decrypt(sock, addr)
