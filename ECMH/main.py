import random
import hashlib
from Curve import Point, CurveFp

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


class C:
    def __init__(self, a, b):
        self.a = a
        self.b = b


def Cmul(a, n, p, i1, i2):
    c = a * a - n
    t = C((i1.a * i2.a + i1.b * i2.b % p * c) % p, (i1.b * i2.a + i1.a * i2.b) % p)
    return t


def Cpow(a, n, p, x, y):
    z = C(1, 0)
    while y:
        if y & 1:
            z = Cmul(a, n, p, z, x)
        x = Cmul(a, n, p, x, x)
        y >>= 1
    return z


# 欧拉准则判断二次剩余
def legendre(a, p):
    return pow(a, (p - 1) // 2, p)


def Cipolla(n, p):
    while True:
        a = random.randint(0, p - 1)
        if legendre(a * a - n, p) == p - 1:
            break
    u = C(a, 1)
    u = Cpow(a, n, p, u, (p + 1) // 2)
    # 平方根存在两个解，且一定为y和p-y的形式(y<p/2)。为保证hash值唯一性，取较大的一个。
    if u.a < p // 2:
        u.a = p - u.a
    return u.a % p


def Hash(msg):
    # 将单个点使用md5映射到椭圆曲线点的横坐标
    x = int(hashlib.sha256(msg.encode()).hexdigest(), 16) % p
    temp = x ** 3 + a * x + b
    while legendre(temp, p) != 1:
        x += 1
        temp = x ** 3 + a * x + b
    y = Cipolla(temp, p)
    return Point(E, x, y)


def setHash(s):
    P = Hash(s[0])
    for msg in s[1:]:
        P += Hash(msg)
    return P


def printPoint(P):
    print(hex(P.x), hex(P.y))


s1 = ['test1']
s2 = ['test2']
s3 = ['test3']
s12 = s1 + s2
s = ['test1', 'test2', 'test3']
printPoint(setHash(s))
printPoint(setHash(s1 + s2 + s3))
printPoint(setHash(s12 + s3))
