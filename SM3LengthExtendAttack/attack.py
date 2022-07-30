from gmssl import sm3, func
import random
import gmssl_sm3
import struct

# 原始消息及其hash值
message = str(random.randint(10 ** 200, 10 ** 201))
message_hash = sm3.sm3_hash(func.bytes_to_list(bytes(message, encoding='utf-8')))
# 附加消息
append_m = "202000460058"

pad = []
pad_str = ""


def Length_Extend_Attack(src_hash, append_msg):
    vec = []
    message_len = len(message)

    for i in range(0, len(src_hash), 8):
        vec.append(int(src_hash[i:i + 8], 16))

    # 长度扩展攻击下，通过保存向量状态来得到扩展内容的hash值。而不需要知道前面一段消息的明文，可以随意填充。
    fake_msg = '0' * message_len
    fake_msg = func.bytes_to_list(bytes(fake_msg, encoding='utf-8'))
    fake_msg = padding(fake_msg)  # 将伪造消息填充

    # 利用vec信息对扩展后的消息做hash
    fake_msg.extend(func.bytes_to_list(bytes(append_msg, encoding='utf-8')))
    return gmssl_sm3.sm3_hash(fake_msg, vec)


def padding(msg):
    #  参考gmssl填充规则，对消息进行填充
    mlen = len(msg)
    msg.append(0x80)
    mlen += 1
    reserve1 = mlen % 64
    range_end = 56
    if reserve1 > range_end:
        range_end = range_end + 64
    for i in range(reserve1, range_end):
        msg.append(0x00)
    bit_len = (mlen - 1) * 8
    msg.extend([int(x) for x in struct.pack('>q', bit_len)])
    return msg


# 长度扩展构造的hash值，不需要知道密钥
fake_hash = Length_Extend_Attack(message_hash, append_m)

# 对扩展消息计算正确的hash
new_msg = func.bytes_to_list(bytes(message, encoding='utf-8'))
new_msg = padding(new_msg)
new_msg.extend(func.bytes_to_list(bytes(append_m, encoding='utf-8')))
new_hash = sm3.sm3_hash(new_msg)

print("message: ", message)
print("message_hash:", message_hash)
print("append_message:", append_m, '\n')

print('new_hash', new_hash)
print("fake_hash:", fake_hash, '\n')

if fake_hash == new_hash:
    print('Attack Success')
else:
    print('Failed')
