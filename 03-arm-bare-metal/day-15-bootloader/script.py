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
    packets = []

    j = 0
    timeout_counter = 0

    for i in range(0, total_chunks):
        new_chunk = content[i * 128:i * 128 + 128]

        if len(new_chunk) != 128:
            new_chunk = new_chunk + ((128 - len(new_chunk) % 128) * b'\xff')

        covered_sum = (0xAA + 128 + sum(new_chunk)) & 0xFFFF

        high_sum = (covered_sum >> 8) & 0xFF;
        low_sum = covered_sum & 0xFF;
        
        packet = bytes([0xAA, 128]) + new_chunk + bytes([high_sum, low_sum, 0xBB])
        packets.append(packet)

    while j < total_chunks:
        ser.write(packets[j])

        received_answer = ser.read(1)

        if not received_answer:
            timeout_counter += 1

            if timeout_counter >= 2:
                print("There is an error!")
                break
        else:
            timeout_counter = 0
            numerical_response = int.from_bytes(received_answer, "little")

            if numerical_response == 0:
                j += 1
            else:
                action = NACK_ACTIONS.get(numerical_response)
                action_result = action()

                if action_result == "abort":
                    break

    last_sentinel = bytes([0xAA, 0])
    ser.write(last_sentinel)

    sentinel_response = ser.read(1)

    if not sentinel_response:
        print("The device failed to ACK/NACK the sentinel with length = 0")
    else:
        numerical_sentinel_response = int.from_bytes(sentinel_response, "little")

        if numerical_sentinel_response != 0:
            print(f"The device sent data {sentinel_response} that is not what was expected")
        else:
            print("The device has successfully ACKed the last sentinel")