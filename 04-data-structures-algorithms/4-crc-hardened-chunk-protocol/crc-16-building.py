#!/usr/bin/env python3

import random

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

def swap_corrupt(data: bytes, index: int):
    data_array = bytearray(data)
    tmp = data_array[index]
    data_array[index] = data_array[index + 1]
    data_array[index + 1] = tmp

    return bytes(data_array)

def bit_flips_corrupt(data: bytes, index1: int, index2: int, bit_position: int):
    data_int = int.from_bytes(data, 'little')
    mask = 0
    total_bits = len(data) * 8

    corrupt_bit1 = index1 * 8 + bit_position
    corrupt_bit2 = index2 * 8 + bit_position

    mask |= ((1 << corrupt_bit1) | (1 << corrupt_bit2))


    data_int ^= mask

    return data_int.to_bytes(len(data), 'little')

def burst_corrupt(data: bytes, length: int) -> bytes:
    offset = 0
    data_int = int.from_bytes(data, 'big')
    mask = ((1 << length) - 1) << offset
    data_int ^= mask

    return data_int.to_bytes(len(data), 'big')

def random_corrupt(data: bytes, k: int) -> bytes:
    total_bits = len(data)*8

    if k > total_bits:
        raise ValueError(f"Cannot corrupt {k} bits in a {total_bits}-bit packet.")

    data_int = int.from_bytes(data, 'big')

    # Pick k unique bit positions across the entire bit strean
    # positions range from 0 (the rightmost LSB) to total_bit - 1 (the leftmost MSB)
    random_bit_positions = random.sample(range(total_bits), k)

    # construct a master mask by shifting a 1 into every chosen position
    mask = 0
    for position in random_bit_positions:
        mask |= (1 << position)

    data_int ^= mask

    return data_int.to_bytes(len(data), 'big')

def main():
    test_chunk = bytes(range(128))
    print(f"Basic CRC: {crc16(test_chunk)}\n\r")

    swapped = swap_corrupt(test_chunk, 5)
    swapped_arr = bytearray(swapped)
    print(f"Swapped at i=5 CRC: {crc16(swapped)}\n")
    print(f"i=5 {swapped_arr[5]} and i=6 {swapped_arr[6]}\n\r")

    bits_flipped = bit_flips_corrupt(test_chunk, 5, 10, 2)
    bits_flipped_arr = bytearray(bits_flipped)
    print(f"Bit 2 in bytes 5 and 10 flipped CRC: {crc16(bit_flips_corrupt(test_chunk, 5, 10, 2))}\n")
    print(f"i=5 {bits_flipped_arr[5]} and i=10 {bits_flipped_arr[10]}\n\r")

    burst_16 = burst_corrupt(test_chunk, 16)
    burst_16_arr = bytearray(burst_16)
    print(f"Burst 16 CRC: {crc16(burst_corrupt(test_chunk, 16))}\n")
    print(f"i=0 {burst_16_arr[0]}, i=1 {burst_16_arr[1]} and i=2 {burst_16_arr[2]}\n\r")

    burst_17 = burst_corrupt(test_chunk, 17)
    burst_17_arr = bytearray(burst_17)
    print(f"Burst 17 CRC: {crc16(burst_corrupt(test_chunk, 17))}\n")
    print(f"i=0 {burst_17_arr[0]}, i=1 {burst_17_arr[1]}, i=2 {burst_17_arr[2]} and i=3 {burst_17_arr[3]}\n\r")

    print(f"Random 5 corrupted bits CRC: {crc16(random_corrupt(test_chunk, 5))}\n\r")


if __name__ == '__main__':
    main()