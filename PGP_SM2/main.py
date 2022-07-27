import hashlib
import random
import binascii
from math import gcd, log2, ceil
from Curve import Point, CurveFp
from gmssl import sm2
from gmssl.sm4 import CryptSM4, SM4_ENCRYPT, SM4_DECRYPT

n = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
g_X = 0x32c4ae2c1f1981195f9904466a39c9948fe30bbff2660be1715a4589334c74c7
g_Y = 0xbc3736a2f4f6779c59bdcee36b692153d0a9877cc62a474002df32e52139f0a0
a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
b = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93

E = CurveFp(p, a, b)
G = Point(E, g_X, g_Y)


def Hash(msg):
    return hashlib.sha256(msg.encode()).hexdigest()


def point2hex(p):
    return hex(p.x)[2:] + hex(p.y)[2:]


def hex2str(n):
    return hex(n)[2:]


# length bit
def KDF(Z, length):
    ct = 1
    K = ""
    for i in range(ceil(length / 256)):
        K += Hash((Z + str(ct).format("08x")))
        ct += 1
    return K[:length]


# generate ID and key

ID_A = hex(random.randint(0, 2 ** 32 - 1))[2:]
ID_B = hex(random.randint(0, 2 ** 32 - 1))[2:]
ENTL_A = hex(len(bin(int(ID_A, 16))[2:]))[2:]
ENTL_B = hex(len(bin(int(ID_B, 16))[2:]))[2:]

d_A = random.randint(2, n - 1)
while gcd(d_A, n) != 1:
    d_A = random.randint(2, n - 1)

d_B = random.randint(2, n - 1)
while gcd(d_B, n) != 1 or d_A == d_B:
    d_B = random.randint(2, n - 1)

P_A = d_A * G
P_B = d_B * G

Z_A = Hash(
    ENTL_A + ID_A + hex2str(a) + hex2str(b) + hex2str(g_X) + hex2str(g_Y) + hex2str(P_A.x) + hex2str(P_A.y))
Z_B = Hash(
    ENTL_B + ID_B + hex2str(a) + hex2str(b) + hex2str(g_X) + hex2str(g_Y) + hex2str(P_B.x) + hex2str(P_B.y))

w = ceil(log2(n) / 2) - 1

# A
r_A = random.randint(1, n - 1)
R_A = r_A * G
x_1 = 2 ** w + (R_A.x & (2 ** w - 1))

# B
r_B = random.randint(1, n - 1)
R_B = r_B * G
x_2 = 2 ** w + (R_B.x & (2 ** w - 1))
t_B = (d_B + x_2 * r_B) % n
if not E.contains_point(R_A.x, R_A.y):
    print("Wrong1")
x_1 = 2 ** w + (R_A.x & (2 ** w - 1))
V = t_B * (P_A + x_1 * R_A)
K_B = KDF(hex2str(V.x) + hex2str(V.y) + Z_A + Z_B, 128)
S_B = Hash('\x02' + hex2str(V.y) + Hash(
    hex2str(V.x) + Z_A + Z_B + hex2str(R_A.x) + hex2str(R_A.y) + hex2str(R_B.x) + hex2str(R_B.y)))

# A
t_A = (d_A + x_1 * r_A) % n
if not E.contains_point(R_B.x, R_B.y):
    print("Wrong2")
x_2 = 2 ** w + (R_B.x & (2 ** w - 1))
U = t_A * (P_B + x_2 * R_B)
S_1 = Hash('\x02' + hex2str(U.y) + Hash(
    hex2str(U.x) + Z_A + Z_B + hex2str(R_A.x) + hex2str(R_A.y) + hex2str(R_B.x) + hex2str(R_B.y)))
if S_1 != S_B:
    print('Wrong3')
K_A = KDF(hex2str(U.x) + hex2str(U.y) + Z_A + Z_B, 128)
S_A = Hash('\x03' + hex2str(U.y) + Hash(
    hex2str(U.x) + Z_A + Z_B + hex2str(R_A.x) + hex2str(R_A.y) + hex2str(R_B.x) + hex2str(R_B.y)))

# B
S_2 = Hash('\x03' + hex2str(V.y) + Hash(
    hex2str(V.x) + Z_A + Z_B + hex2str(R_A.x) + hex2str(R_A.y) + hex2str(R_B.x) + hex2str(R_B.y)))
if S_2 != S_A:
    print("Wrong4")
print('session key:')
print('K_A:', K_A, '\nK_B:', K_B, sep='')

# session
message = b'hello'
# A encrypt session key
cipher_A = sm2.CryptSM2(None, point2hex(P_B))
ciphertext = cipher_A.encrypt(K_A.encode())
# print(ciphertext)


# B decrypt
cipher_B = sm2.CryptSM2(hex2str(d_B), point2hex(P_B))
key_str = cipher_B.decrypt(ciphertext)
# print(key_str)

key = binascii.a2b_hex(key_str)

# A encrypt message with key
encrypt = CryptSM4()
encrypt.set_key(key, SM4_ENCRYPT)
c = encrypt.crypt_ecb(message)

# B decrypt message with key
decrypt = CryptSM4()
decrypt.set_key(key, SM4_DECRYPT)
m = decrypt.crypt_ecb(c)
print(m)
