import hashlib
import socket
import random
from math import gcd
from argon2 import PasswordHasher
from typing import Union
from argon2.low_level import hash_secret

p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF

a = random.randint(1, p - 1)
while gcd(a, p-1) != 1:
    a = random.randint(1, p - 1)


def inv_mod(b, p):
    if b < 0 or p <= b:
        b = b % p
    c, d = b, p
    uc, vc, ud, vd, temp = 1, 0, 0, 1, 0
    while c != 0:
        temp = c
        q, c, d = d // c, d % c, temp
        uc, vc, ud, vd = ud - q * uc, vd - q * vc, uc, vc

    assert d == 1
    if ud > 0:
        return ud
    else:
        return ud + p


def bytes2int(s):
    Sum = 0
    for i in s:
        Sum = Sum * 2 ** 8 + ord(i)
    return Sum


def int2str(num):
    return '{:064x}'.format(num)


def ensurebytes(s: Union[bytes, str], encoding: str) -> bytes:
    """
    Ensure *s* is a bytes string.  Encode using *encoding* if it isn't.
    """
    if isinstance(s, bytes):
        return s
    return s.encode(encoding)


def Argon2(u, p, hlen):
    ph = PasswordHasher(hash_len=hlen)
    h = hash_secret(secret=ensurebytes(p, ph.encoding),
                    salt=hashlib.sha256(u.encode()).hexdigest().encode(),
                    time_cost=ph.time_cost,
                    memory_cost=ph.memory_cost,
                    parallelism=ph.parallelism,
                    hash_len=ph.hash_len,
                    type=ph.type)
    return h.decode()


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('127.0.0.1', 9090))

userName, passWord = 'userA', '123'

h = Argon2(userName, passWord, 32)[-43:]
k = bytes2int(h[:2])
v = pow(bytes2int(h), a, p)

data = int2str(k) + int2str(v)
s.send(data.encode())

data = s.recv(1024).decode()
hab = int(data[:64], 16)
S = []
cnt = len(data) // 64 - 1
for i in range(cnt):
    S.append(int(data[(i + 1) * 64:(i + 2) * 64], 16))

hb = pow(hab, inv_mod(a, p-1), p)

if hb in S:
    print('accept')
else:
    print('wrong')

s.close()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('127.0.0.1', 9090))

# 设置错误的口令，重复上述过程
userName, passWord = 'userA', '1234'

h = Argon2(userName, passWord, 32)[-43:]
k = bytes2int(h[:2])
v = pow(bytes2int(h), a, p)

data = int2str(k) + int2str(v)
s.send(data.encode())

data = s.recv(1024).decode()
hab = int(data[:64], 16)
S = []
cnt = len(data) // 64 - 1
for i in range(cnt):
    S.append(int(data[(i + 1) * 64:(i + 2) * 64], 16))

hb = pow(hab, inv_mod(a, p-1), p)

if hb in S:
    print('accept')
else:
    print('wrong')

s.close()
