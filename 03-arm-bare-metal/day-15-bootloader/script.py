import serial

def retry():
    pass

def abort():
    return "abort"

NACK_ACTIONS = {
    1: retry,
    2: abort,
    3: retry,
    4: abort,
    5: retry,
}

ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=0.03)

with open("file.bin", "rb") as file:
    content = file.read()
    total_chunks = (len(content) + 128 - 1) // 128
    chunks = []
    packets = []

    for i in range(0, total_chunks):
        new_chunk = content[i * 128:i * 128 + 128]

        if len(new_chunk) != 128:
            new_chunk = new_chunk + ((128 - len(new_chunk) % 128) * b'\xff')

        covered_sum = (0xAA + 128 + sum(new_chunk)) & 0xFFFF
        chunks.append(new_chunk)

        high_sum = (covered_sum >> 8) & 0xFF;
        low_sum = covered_sum & 0xFF;
        
        packet = bytes([0xAA, 128]) + new_chunk + bytes([high_sum, low_sum, 0xBB])
        packets.append(packet)