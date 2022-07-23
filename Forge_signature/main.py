import hashlib
import random
from Curve import Point, CurveFp, inv_mod

# Secp256k1
p = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
a = 0x0000000000000000000000000000000000000000000000000000000000000000
b = 0x0000000000000000000000000000000000000000000000000000000000000007
G_x = 0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
G_y = 0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8
n = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141

E = CurveFp(p, a, b)
G = Point(E, G_x, G_y)

d = random.randint(1, n - 1)
P = d * G

msg = 'forge_signature'


def Hash(msg):
    return hashlib.md5(msg.encode()).hexdigest()


def sign(d, m):
    k = random.randint(1, n - 1)
    R = k * G
    r = R.x % n
    e = int(Hash(m), 16)
    s = (inv_mod(k, n) * (e + d * r)) % n
    return r, s


# 不检查消息m
def verify(r, s, e, P):
    w = inv_mod(s, n)
    Pt = e * w * G + r * w * P
    if r != Pt.x:
        return False
    return True


def forge(r, s):
    u = random.randint(1, n - 1)
    v = random.randint(1, n - 1)
    R_ = u * G + v * P
    r_ = R_.x % n
    e_ = (r_ * u * inv_mod(v, n)) % n
    s_ = (r_ * inv_mod(v, n)) % n
    return r_, s_, e_


r, s = sign(d, msg)
e = int(Hash(msg), 16)
print('real sign:\n', (r, s), sep='')
print('verify:', verify(r, s, e, P), sep='')

r_, s_, e_ = forge(r, s)
print('forge sign:\n', (r_, s_), sep='')
print('e\'=', e_, sep='')
print('verify:', verify(r_, s_, e_, P), sep='')
