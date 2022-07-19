import hashlib
import hmac
import re
import random
from Curve import Point, CurveFp, inv_mod
from math import gcd

q = 0x8542D69E4C044F18E8B92435BF6FF7DE457283915C45517D722EDB8B08F1DFC3
a = 0x787968B4FA32C3FD2417842E73BBFEFF2F3C848B6831D7E0EC65228B3937E498
b = 0x63E4C6D3B23B0C849CF84241484BFE48F61D59A5B16BA06E6E12D1DA27C5249A
x_G = 0x421DEBD61B62EAB6746434EBC3CC315E32220B3BADD50BDC4C4E6C147FEDD43D
y_G = 0x0680512BCBB42C07D47349D2153B70C4E5D7FDFCBFA36EA1A85841B9E46E09A2
n = 0x8542D69E4C044F18E8B92435BF6FF7DD297720630485628D5AE74EE7C32E79B7

msg = b'hello'

string_types = (str)
string_or_bytes_types = (str, bytes)
int_types = (int, float)

# 方便处理字符串与数字转换
code_strings = {
    2: '01',
    10: '0123456789',
    16: '0123456789abcdef',
    32: 'abcdefghijklmnopqrstuvwxyz234567',
    58: '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz',
    256: ''.join([chr(x) for x in range(256)])
}


# 返回数据hash值
def bin_sha256(string):
    binary_data = string if isinstance(string, bytes) else bytes(string, 'utf-8')
    return hashlib.sha256(binary_data).digest()


def get_code_string(base):
    if base in code_strings:
        return code_strings[base]
    else:
        raise ValueError("Invalid base!")


# 按照base进制编码 数字->字符
def encode(val, base, minlen=0):
    base, minlen = int(base), int(minlen)
    code_string = get_code_string(base)
    result_bytes = bytes()
    while val > 0:
        curcode = code_string[val % base]
        result_bytes = bytes([ord(curcode)]) + result_bytes
        val //= base

    pad_size = minlen - len(result_bytes)

    padding_element = b'\x00' if base == 256 else b'1' \
        if base == 58 else b'0'
    if pad_size > 0:
        result_bytes = padding_element * pad_size + result_bytes

    result_string = ''.join([chr(y) for y in result_bytes])
    result = result_bytes if base == 256 else result_string

    return result


# 同理 解码
def decode(string, base):
    if base == 256 and isinstance(string, str):
        string = bytes(bytearray.fromhex(string))
    base = int(base)
    code_string = get_code_string(base)
    result = 0
    if base == 256:
        def extract(d, cs):
            return d
    else:
        def extract(d, cs):
            return cs.find(d if isinstance(d, str) else chr(d))

    if base == 16:
        string = string.lower()
    while len(string) > 0:
        result *= base
        result += extract(string[0], code_string)
        string = string[1:]
    return result


# 将hash字符串转化成数字
def hash_to_int(x):
    if len(x) in [40, 64]:
        return decode(x, 16)
    return decode(x, 256)


def from_int_to_byte(a):
    return bytes([a])


def from_string_to_bytes(a):
    return a if isinstance(a, bytes) else bytes(a, 'utf-8')


# 使用字符symbol将消息填充至length
def lpad(msg, symbol, length):
    if len(msg) >= length:
        return msg
    return symbol * (length - len(msg)) + msg


# 更换字符串进制 先解码成数字再编码
def changebase(string, frm, to, minlen=0):
    if frm == to:
        return lpad(string, get_code_string(frm)[0], minlen)
    return encode(decode(string, frm), to, minlen)


def bin_dbl_sha256(s):
    bytes_to_hash = from_string_to_bytes(s)
    return hashlib.sha256(hashlib.sha256(bytes_to_hash).digest()).digest()


def b58check_to_bin(inp):
    leadingzbytes = len(re.match('^1*', inp).group(0))
    data = b'\x00' * leadingzbytes + changebase(inp, 58, 256)
    assert bin_dbl_sha256(data[:-4])[:4] == data[-4:]
    return data[1:-4]


def bin_to_b58check(inp, magicbyte=0):
    if magicbyte == 0:
        inp = from_int_to_byte(0) + inp
    while magicbyte > 0:
        inp = from_int_to_byte(magicbyte % 256) + inp
        magicbyte //= 256

    leadingzbytes = 0
    for x in inp:
        if x != 0:
            break
        leadingzbytes += 1

    checksum = bin_dbl_sha256(inp)[:4]
    return '1' * leadingzbytes + changebase(inp + checksum, 256, 58)


# 获取私钥格式
def get_privkey_format(priv):
    if isinstance(priv, int_types):
        return 'decimal'
    elif len(priv) == 32:
        return 'bin'
    elif len(priv) == 33:
        return 'bin_compressed'
    elif len(priv) == 64:
        return 'hex'
    elif len(priv) == 66:
        return 'hex_compressed'
    else:
        bin_p = b58check_to_bin(priv)
        if len(bin_p) == 32:
            return 'wif'
        elif len(bin_p) == 33:
            return 'wif_compressed'
        else:
            raise Exception("WIF does not represent privkey")


def decode_privkey(priv, formt=None):
    if not formt: formt = get_privkey_format(priv)
    if formt == 'decimal':
        return priv
    elif formt == 'bin':
        return decode(priv, 256)
    elif formt == 'bin_compressed':
        return decode(priv[:32], 256)
    elif formt == 'hex':
        return decode(priv, 16)
    elif formt == 'hex_compressed':
        return decode(priv[:64], 16)
    elif formt == 'wif':
        return decode(b58check_to_bin(priv), 256)
    elif formt == 'wif_compressed':
        return decode(b58check_to_bin(priv)[:32], 256)
    else:
        raise Exception("WIF does not represent privkey")


def encode_privkey(priv, formt, vbyte=128):
    if not isinstance(priv, int_types):
        return encode_privkey(decode_privkey(priv), formt, vbyte)
    if formt == 'decimal':
        return priv
    elif formt == 'bin':
        return encode(priv, 256, 32)
    elif formt == 'bin_compressed':
        return encode(priv, 256, 32) + b'\x01'
    elif formt == 'hex':
        return encode(priv, 16, 64)
    elif formt == 'hex_compressed':
        return encode(priv, 16, 64) + '01'
    elif formt == 'wif':
        return bin_to_b58check(encode(priv, 256, 32), int(vbyte))
    elif formt == 'wif_compressed':
        return bin_to_b58check(encode(priv, 256, 32) + b'\x01', int(vbyte))
    else:
        raise Exception("Invalid format!")


def deterministic_generate_k(msghash, sk):
    v = b'\x01' * 32
    k = b'\x00' * 32

    sk = encode_privkey(sk, 'bin')

    msghash = encode(hash_to_int(msghash), 256, 32)

    # K = HMAC_K（V || 0x00 || int2octets（x）|| bits2octets（h1））
    k = hmac.new(k, v + b'\x00' + sk + msghash, hashlib.sha256).digest()
    # V = HMAC_K（V）
    v = hmac.new(k, v, hashlib.sha256).digest()
    # K = HMAC_K（V || 0x01 || int2octets（x）|| bits2octets（h1））
    k = hmac.new(k, v + b'\x01' + sk + msghash, hashlib.sha256).digest()
    # V = HMAC_K（V）
    v = hmac.new(k, v, hashlib.sha256).digest()

    while True:
        v = hmac.new(k, v, hashlib.sha256).digest()
        T = v
        res = decode(T, 256)
        if 1 <= decode(T, 256) <= q - 1:
            return res
        k = hmac.new(k, v + b'\x00', hashlib.sha256).digest()
        v = hmac.new(k, v, hashlib.sha256).digest()


d_A = random.randint(2, n - 1)
k = deterministic_generate_k(bin_sha256(str(msg)), encode(d_A, 256, 32))

ID = b'1234567812345678'
ID_str = '1234567812345678'
E = CurveFp(q, a, b)
G = Point(E, x_G, y_G)

while gcd(d_A, n) != 1:
    d_A = random.randint(2, n - 1)
P_A = d_A * G
ENTL = len(encode(decode(ID, 256), 2))


# print(encode(ENTL,16))

def sign(msg, ID_str, d_A):
    Z_A = encode(ENTL, 16) + ID_str + encode(a, 16) + encode(b, 16) + encode(x_G, 16) + encode(y_G, 16) + encode(P_A.x,
                                                                                                                 16) + encode(
        P_A.y, 16)
    Z_A = hashlib.sha256(Z_A.encode()).hexdigest()
    M = Z_A + encode(decode(msg, 256), 16)
    e = decode(hashlib.sha256(M.encode()).hexdigest(), 16)
    kG = k * G
    r = (e + kG.x) % n
    s = (inv_mod(1 + d_A, n) * (k - r * d_A)) % n
    return r, s


def verify(msg, r, s, ID_str):
    Z_A = encode(ENTL, 16) + ID_str + encode(a, 16) + encode(b, 16) + encode(x_G, 16) + encode(y_G, 16) + encode(P_A.x,
                                                                                                                 16) + encode(
        P_A.y, 16)
    Z_A = hashlib.sha256(Z_A.encode()).hexdigest()
    M = Z_A + encode(decode(msg, 256), 16)
    e = decode(hashlib.sha256(M.encode()).hexdigest(), 16)
    t = (r + s) % n
    pt = s * G + t * P_A
    R = (e + pt.x) % n
    return R == r


r, s = sign(msg, ID_str, d_A)
print("private_key:", hex(d_A), sep='')
print("k=", hex(k), sep='')
print('message:', msg)
print(hex(r), hex(s), sep='\n')
print('verify:', verify(msg, r, s, ID_str))
