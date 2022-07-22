import socket
import threading
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

port = 9999


def int2str(n):
    return "{:064x}".format(n)


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('127.0.0.1', port))
s.listen(5)


def sign(sock, addr):
    d_2 = random.randint(1, n - 1)
    data = sock.recv(1024).decode()
    P_1x, P_1y = int(data[:64], 16), int(data[64:128], 16)
    P_1 = Point(E, P_1x, P_1y)
    nG = Point(E, G.x, p - G.y)
    P = inv_mod(d_2, p) * P_1 + nG

    data = sock.recv(1024).decode()
    Q_1 = Point(E, int(data[:64], 16), int(data[64:128], 16))
    e = int(data[128:], 16)
    k_2 = random.randint(1, n - 1)
    Q_2 = k_2 * G
    k_3 = random.randint(1, n - 1)
    temp = k_3 * Q_1 + Q_2
    r = (temp.x + e) % n
    s_2 = (d_2 * k_3) % n
    s_3 = (d_2 * (r + k_2)) % n
    sock.send((int2str(r) + int2str(s_2) + int2str(s_3)).encode())
    sock.close()


while True:
    sock, addr = s.accept()
    t = threading.Thread(target=sign, args=(sock, addr))
    t.start()
