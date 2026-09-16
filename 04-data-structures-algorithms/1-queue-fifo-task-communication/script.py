#!/usr/bin/env python3

import sys
import can
import time

MAX_ID = 0x7FF

def StandardMessage(**kwargs) -> can.Message:
    return can.Message(is_extended_id=False, **kwargs)

def main():
    channel = sys.argv[1] if len(sys.argv) > 1 else "can0"

    bus = can.interface.Bus(channel=channel, interface='socketcan')
    print(f"Sending data frames on {channel}... (Ctrl+C to stop)")

    try:
        for i in range(0x01, 0x800):
            data_arr = []
            for j in range(0, 8):
                data_arr.append((i + j) & 0xFF)
            msg = StandardMessage(arbitration_id=i, data=data_arr)
            try:
                bus.send(msg)
                print(f"Frame with ID {i} was sent on {bus.channel_info}")
            except can.CanError:
                print("Frame was not sent")

            time.sleep(0.009)
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        bus.shutdown()

if __name__ == "__main__":
    main()
