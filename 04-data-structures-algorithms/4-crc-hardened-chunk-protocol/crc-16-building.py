#!/usr/bin/env python3

def reflect(byte, size=8):
    reversed_byte = 0

    for _ in range(size):
        # Peeling off the lowest bit of the number
        bit = byte & 1

        reversed_byte = (reversed_byte << 1) | bit

        # Shifting right by 1 bit
        byte = (byte >> 1)
    
    return reversed_byte

init = 0xFFFF
xorout = 0xFFFF
poly = 0x1021
reflected_poly = reflect(poly, size=16)

def crc16(data: bytes) -> int:
    register = init
    
    byte_len = len(data)

    for i in range(byte_len):
        register ^= data[i]

        for _ in range(0, 8):
            bit = register & 1

            if bit == 1:
                register = (register >> 1) ^ reflected_poly
            else:
                register = (register >> 1)
    
    register ^= xorout

    return register

def swap_corrupt(data: bytes, index):
    tmp = data[index]
    data[index] = data[index + 1]
    data[index + 1] = data[index]

    return data

def bit_flips_corrupt(data: bytes, index1, index2, bit_position):
    byte1 = data[index1]
    byte2 = data[index2]

    in_byte1 = byte1 & (1 << bit_position)
    if in_byte1 == 1:
        byte1 = byte1 & ~(1 << bit_position)
    else:
        byte1 = byte1 | (1 << bit_position)

    data[index1] = byte1

    in_byte2 = byte2 & (1 << bit_position)
    if in_byte2 == 1:
        byte2 = byte2 & ~(1 << bit_position)
    else:
        byte2 = byte2 | (1 << bit_position)
    
    data[index2] = byte2

    return data

def burst_corrupt(data: bytes, length):
    return data

def random_corrupt(data: bytes, k):
    return data

def main():
    print(crc16("123456789".encode('ascii')))

if __name__ == '__main__':
    main()