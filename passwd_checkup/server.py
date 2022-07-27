import hashlib
import socket
import random
from argon2 import PasswordHasher
from typing import Union
from argon2.low_level import hash_secret

p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF

b = random.randint(1, p - 1)

port = 9090
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('127.0.0.1', port))


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


def checkup(sock, addr):
    data = sock.recv(1024).decode()
    k, v = int(data[:64], 16), int(data[64:128], 16)

    hab = int2str(pow(v, b, p))
    S = ''
    for i in Set[k]:
        S += int2str(i)
    sock.send((hab + S).encode())
    sock.close()


Data_records = {'userA': '123', 'userB': '456', 'userC': '789'}
Set = [0] * 2 ** 16
for i in range(2 ** 16):
    Set[i] = []

for user in Data_records:
    h = Argon2(user, Data_records[user], 32)[-43:]
    k = bytes2int(h[:2])
    v = pow(bytes2int(h), b, p)
    Set[k].append(v)

print('listening port 9090...')
s.listen(5)

while True:
    sock, addr = s.accept()
    print('connected from', addr)
    checkup(sock, addr)
