raw_data = '01000000014ef42f889709621db7c0ecca0b1b2091721ead72b3641b2e896106dc10518b06010000006a4730440220276dc0d6822dc84f60d5d68b10a19ea0041b451175b67f94d874206198c08aa802202060755d1061b9a45ebf366a9abbdebd86f3d81e068950e001de73c8b385c461012102dcecddaba2d7ef86cdbad5aa79aecd00593801d99deff79ca796b0dcd136908cffffffff01411e00000000000017a9148901d45c15f2734ce3e5bc1654d362cd316242558700000000'


def little_endian(s):
    invs = ''
    l = len(s)
    for i in range(l // 2):
        invs += s[l - i * 2 - 2:l - i * 2]
    return invs


OP = {'87': 'OP_EQUAL', '88': 'OP_EUQALVERIFY', 'a9': 'OP_HASH160'}

version = little_endian(raw_data[:8])
input_cnt = raw_data[8:10]
previous_output_hash = little_endian(raw_data[10:74])
previous_output_index = little_endian(raw_data[74:82])
in_script_len = raw_data[82:84]
scriptSig = raw_data[84:84 + int(in_script_len, 16) * 2]
sequence = raw_data[84 + int(in_script_len, 16) * 2:84 + int(in_script_len, 16) * 2 + 8]
print('ver:', int(version, 16))
print('vin_sz:', int(input_cnt, 16))
print('prev_hash:', previous_output_hash)
print('output_index:', int(previous_output_index, 16))
print('\ninput script:', scriptSig)
print('script analyse:')
sig = 'r='+scriptSig[10:10 + 64] + ' s=' + scriptSig[74 + 4:74 + 4 + 64]
Hashtype = scriptSig[74 + 4 + 64:74 + 4 + 64 + 2]
pubkey = scriptSig[74 + 4 + 64 + 2:]
print('PUSH_DATA',sig, Hashtype, 'pubkey:', pubkey[4:])

print('\nsequence:', int(sequence, 16))

idx = 84 + int(in_script_len, 16) * 2 + 8

output_cnt = raw_data[idx:idx + 2]
output_value = little_endian(raw_data[idx + 2:idx + 18])
out_script_len = raw_data[idx + 18:idx + 20]
scriptPubKey = raw_data[idx + 20:idx + 20 + int(out_script_len, 16) * 2]
print('vout_sz:', int(output_cnt, 16))
print('output_value:', int(output_value, 16))
print('\noutput_script:', scriptPubKey)
print('script_analyse:', OP[scriptPubKey[:2]], scriptPubKey[2:-2], OP[scriptPubKey[-2:]])

idx = idx + 20 + int(out_script_len, 16) * 2

locktime = raw_data[idx:]
print('\nblock lock time:', locktime)
