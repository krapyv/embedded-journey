#!/usr/bin/env python3
"""Decode the single-frame sensor layout (ID 0x100, DLC 8):
   bytes 0-1  temperature int16, 0.01 degC, MSB first
   bytes 2-5  pressure uint32, Q24.8 Pa, MSB first
   byte  6    sequence counter uint8 (wraps 255 -> 0)
   byte  7    reserved (expected 0)
"""
import sys
import struct
import can

FRAME_ID = 0x100

def main():
    channel = sys.argv[1] if len(sys.argv) > 1 else "can0"
    bus = can.interface.Bus(channel=channel, interface="socketcan")
    print(f"Listening on {channel} for ID 0x{FRAME_ID:03X}... (Ctrl+C to stop)")

    last_seq = None
    last_time = None
    gaps = 0

    try:
        for msg in bus:
            if msg.arbitration_id != FRAME_ID or msg.is_extended_id:
                continue
            if msg.dlc != 8:
                print(f"[WARN ] unexpected DLC {msg.dlc}: {msg.data.hex()}")
                continue

            temp_raw, press_raw, seq, reserved = struct.unpack(">hIBB", bytes(msg.data))
            temp_c = temp_raw / 100.0
            press_hpa = press_raw / 256.0 / 100.0

            note = ""
            if last_seq is not None:
                step = (seq - last_seq) % 256
                if step != 1:
                    gaps += 1
                    note = f"  <-- GAP: {last_seq} -> {seq} ({step - 1} lost)"
            dt = "" if last_time is None else f"  dt={(msg.timestamp - last_time) * 1000:6.1f} ms"
            if reserved != 0:
                note += f"  (reserved byte = 0x{reserved:02X})"

            print(f"seq={seq:3d}  {temp_c:6.2f} degC  {press_hpa:8.2f} hPa{dt}{note}")

            last_seq = seq
            last_time = msg.timestamp
    except KeyboardInterrupt:
        print(f"\nStopped. Gaps detected: {gaps}")
    except can.CanOperationError as e:
        print(f"\nCAN interface error: {e}. Gaps detected so far: {gaps}")
    finally:
        bus.shutdown()

if __name__ == "__main__":
    main()