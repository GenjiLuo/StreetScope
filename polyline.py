#!/usr/bin/python
import struct

def pad(l, n=8):
    return [0] * (n - len(l)) + l

def int2bin(x, n=8):
    return pad(map(int, bin(x)[2:]), n)

def bin2int(bits):
    return reduce(lambda x, y: 2*x+y, bits)

def decode_one(vals):
    vals = reversed(vals)
    bits = []
    for val in vals:
        bits.extend(int2bin(val, 5))
    bits = pad(bits, 32)
    neg = bits[31]
    if neg:
        bits = [1-b for b in bits]
    bits = [neg] + bits[:31]
    chars = ''.join(chr(bin2int(bits[x:x+8])) for x in range(0, 32, 8))
    return struct.unpack('>i', chars)[0] / 1e5

# decode a string containing some number of polyline-encoded values
def decode_nums(st):
    res = []
    curr = []
    for c in st:
        val = ord(c) - 63
        if val >= 32:
            curr.append(val - 32)
        else:
            curr.append(val)
            res.append(decode_one(curr))
            curr = []
    return res

# decode a string, interpreting it as a lat-long pair followed by
# lat-long deltas
def decode(st):
    vals = decode_nums(st)
    res = [(vals[0], vals[1])]
    for i in range(1, len(vals)/2):
        res.append((res[-1][0] + vals[2*i], res[-1][1] + vals[2*i+1]))
    return res
