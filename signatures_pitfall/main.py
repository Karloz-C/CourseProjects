import random
import hashlib
from math import gcd
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


def Hash(msg):
    return hashlib.md5(msg.encode()).hexdigest()


def sm2_sign(m, d, k=0):
    Z_A = Hash(
        int2str(ENTL) + int2str(ID) + int2str(a) + int2str(b) + int2str(g_X) + int2str(g_Y) + int2str(P.x) + int2str(
            P.y))
    M = Z_A + m
    e = int(Hash(M), 16)
    if k == 0:
        k = random.randint(1, n - 1)
        while gcd(k, n) != 1:
            k = random.randint(1, n - 1)
    P1 = k * G
    r = (e + P1.x) % n
    s = (inv_mod(1 + d, n) * (k - r * d)) % n
    return r, s, k


def sm2_verify(r, s, m, P):
    Z_A = Hash(
        int2str(ENTL) + int2str(ID) + int2str(a) + int2str(b) + int2str(g_X) + int2str(g_Y) + int2str(P.x) + int2str(
            P.y))
    M = Z_A + m
    e = int(Hash(M), 16)
    t = (r + s) % n
    P1 = s * G + t * P
    R = (e + P1.x) % n
    return R == r


def ECDSA_sign(m, d, k=0):
    if k == 0:
        k = random.randint(1, n - 1)
        while gcd(k, n) != 1:
            k = random.randint(1, n - 1)
    R = k * G
    r = R.x % n
    e = int(Hash(m), 16)
    s = (inv_mod(k, n) * (e + d * r)) % n
    return r, s, k


def ECDSA_verify(r, s, m, P):
    e = int(Hash(m), 16)
    w = inv_mod(s, n)
    R_ = e * w * G + r * w * P
    return R_.x == r


def schnorr_sign(m, d, k=0):
    if k == 0:
        k = random.randint(1, n - 1)
        while gcd(k, n) != 1:
            k = random.randint(1, n - 1)
    R = k * G
    e = int(Hash(int2str(R.x) + int2str(R.y) + m), 16)
    s = (k + e * d) % n
    return R, s, k


def schnorr_verify(R, s, m, P):
    e = int(Hash(int2str(R.x) + int2str(R.y) + m), 16)
    return (s * G).__eq__(R + e * P)


# generate d and P
d = random.randint(1, n - 1)
while gcd(d, n) != 1:
    d = random.randint(1, n - 1)
P = d * G

# user ID
ID = 202000460058
ENTL = len(bin(ID)[2:])

# 测试签名验证正确性
msg = 'test'
r1, s1, leak_k = sm2_sign(msg, d)
r2, s2, k2 = ECDSA_sign(msg, d)
r3, s3, k3 = schnorr_sign(msg, d)
print('SM2:', sm2_verify(r1, s1, msg, P))
print('ECDSA:', ECDSA_verify(r2, s2, msg, P))
print('schnorr:', schnorr_verify(r3, s3, msg, P))
print()

# ---------------------------------- leaking k -----------------------------------
print('---------------------------------- leaking k -----------------------------------')
print('正确的私钥 d=', d, sep='')
print('根据泄露的k计算所得私钥 d\'=', (inv_mod(s1 + r1, n) * (leak_k - s1)) % n, sep='')
print()

# ---------------------------------- reusing k -----------------------------------
print('---------------------------------- reusing k -----------------------------------')
r1, s1, leak_k = sm2_sign(msg, d)
r2, s2, k2 = sm2_sign(msg + '123', d, leak_k)
d_ = ((s2 - s1) * inv_mod(s1 - s2 + r1 - r2, n)) % n
print('正确的私钥 d=', d, sep='')
print('根据泄露的k计算所得私钥 d\'=', d_, sep='')
print()

# ------------------------- reusing k by different users -------------------------
d_A = random.randint(1, n - 1)
while gcd(d, n) != 1:
    d_A = random.randint(1, n - 1)
P_A = d_A * G

d_B = random.randint(1, n - 1)
while gcd(d, n) != 1:
    d_B = random.randint(1, n - 1)
P_B = d_B * G

msg1 = 'Alice'
msg2 = 'Bob'

r1, s1, k = sm2_sign(msg1, d_A)
r2, s2, k = sm2_sign(msg2, d_B, k)

# A compute
dB = ((k - s2) * inv_mod(s2 + r2, n) % n)
# B compute
dA = ((k - s1) * inv_mod(s1 + r1, n) % n)
print('------------------------- reusing k by different users -------------------------')
print('正确的私钥d_A和d_B:\n', d_A, ' ', d_B, sep='')
print('根据重用k推导出对方的公钥d_A\'和d_B\'\n', dA, ' ', dB, sep='')
print()

# --------------------------- same d and k with ECDSA ----------------------------
r1, s1, k1 = ECDSA_sign(msg, d)
e1 = int(Hash(msg), 16)
r2, s2, k2 = sm2_sign(msg, d, k1)

d_ = ((s1 * s2 - e1) * inv_mod(r1 - s1 * s2 - s1 * r2, n)) % n
print('--------------------------- same d and k with ECDSA ----------------------------')
print('正确的私钥 d=', d, sep='')
print('计算所得私钥d\'=', d_, sep='')
print()

# --------------------------------- Ambiguity ---------------------------------
r1, s1, leak_k = sm2_sign(msg, d)
r2, s2, k2 = ECDSA_sign(msg, d)
r3, s3, k3 = schnorr_sign(msg, d)
print('--------------------------------- Ambiguity ---------------------------------')
print('SM2:', sm2_verify(r1, n - s1, msg, P))
print('ECDSA:', ECDSA_verify(r2, n - s2, msg, P))
print('schnorr:', schnorr_verify(r3, n - s3, msg, P))
