import serial

ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=0.03)

with open("file.bin", "rb") as file:
    content = file.read()
    total_chunks = (len(content) + 128 - 1) // 128
    chunks = []

    for i in range(0, total_chunks):
        new_chunk = content[i * 128:i * 128 + 128]

        if len(new_chunk) != 128:
            new_chunk = new_chunk + ((128 - len(new_chunk) % 128) * b'\xff')

        covered_sum = content[0] + 128 + sum(new_chunk)
        chunks.append(new_chunk)

