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

def burst_corrupt(data: bytes, length: int, offset:int) -> bytes:
    data_int = int.from_bytes(data, 'big')

    # calculate middle random bit width
    mid_length = length - 2

    lowest_bit = 1
    middle_bits = random.randint(0, (1 << mid_length) - 1) << 1
    highest_bit = 1 << (length - 1)

    constructed_pattern = highest_bit | middle_bits | lowest_bit

    mask = constructed_pattern << offset
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
    print(f"Reflected poly: {reflected_poly}\n\r")
    offset = 0

    burst_misses = 0
    random_misses = 0
    trials = 0

    normal_crc = crc16(test_chunk)

    total_bits = len(test_chunk) * 8
    max_valid_offset = total_bits - 17

    for i in range(1000000):
        offset = random.randint(0, max_valid_offset)
        corrupted_burst = burst_corrupt(test_chunk, 17, offset)
        burst_crc = crc16(corrupted_burst)

        random_crc = crc16(random_corrupt(test_chunk, 6))

        trials += 1

        if burst_crc == normal_crc:
            burst_misses += 1
        
        if random_crc == normal_crc:
            random_misses += 1

    burst_coef_missed = burst_misses / trials
    random_coef_missed = random_misses / trials

    print(f"Normal CRC: {normal_crc}\n\r")
    print(f"Burst misses: {burst_misses} | Total trials {trials} | Misses/trials {burst_coef_missed}\n")
    print(f"Random misses: {random_misses} | Total trials {trials} | Misses/trials {random_coef_missed}\n")



    # swapped = swap_corrupt(test_chunk, 5)
    # swapped_arr = bytearray(swapped)
    # print(f"Swapped at i=5 CRC: {crc16(swapped)}\n")
    # print(f"i=5 {swapped_arr[5]} and i=6 {swapped_arr[6]}\n\r")

    # bits_flipped = bit_flips_corrupt(test_chunk, 5, 10, 2)
    # bits_flipped_arr = bytearray(bits_flipped)
    # print(f"Bit 2 in bytes 5 and 10 flipped CRC: {crc16(bit_flips_corrupt(test_chunk, 5, 10, 2))}\n")
    # print(f"i=5 {bits_flipped_arr[5]} and i=10 {bits_flipped_arr[10]}\n\r")

    # burst_16 = burst_corrupt(test_chunk, 16)
    # burst_16_arr = bytearray(burst_16)
    # print(f"Burst 16 CRC: {crc16(burst_corrupt(test_chunk, 16))}\n")
    # print(f"i=127 {burst_16_arr[127]}, i=126 {burst_16_arr[126]} and i=125 {burst_16_arr[125]}\n\r")

    # burst_17 = burst_corrupt(test_chunk, 17)
    # burst_17_arr = bytearray(burst_17)
    # print(f"Burst 17 CRC: {crc16(burst_corrupt(test_chunk, 17))}\n")
    # print(f"i=127 {burst_17_arr[127]}, i=126 {burst_17_arr[126]}, i=125 {burst_17_arr[125]} and i=124 {burst_17_arr[124]}\n\r")

    # print(f"Random 5 corrupted bits CRC: {crc16(random_corrupt(test_chunk, 5))}\n\r")


if __name__ == '__main__':
    main()