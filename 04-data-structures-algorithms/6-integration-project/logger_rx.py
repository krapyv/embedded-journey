#!/usr/bin/env python3

import argparse
import csv
import struct
import sys
import time

import serial

SYNC = b"\xAA\x55"
TYPE_CAN = 0x01
TYPE_SENSOR = 0x02
RECORD_LEN = {TYPE_CAN: 20, TYPE_SENSOR: 22}
STATUS_NAME = {1: "fresh", 2: "retrying", 3: "dead"}

# fast-channel conversion (must match the firmware)
EMA_SCALE = 1 << 5          # k + frac = 1 + 4
COUNTS_PER_REV = 3840
CONTROL_TICK_S = 0.015


def crc16_x25(data: bytes) -> int:
    """CRC-16/X-25: reflected poly 0x1021 (0x8408), init 0xFFFF, xorout 0xFFFF."""
    reg = 0xFFFF
    for b in data:
        reg ^= b
        for _ in range(8):
            reg = (reg >> 1) ^ 0x8408 if reg & 1 else reg >> 1
    return reg ^ 0xFFFF


assert crc16_x25(b"123456789") == 0x906E, "CRC implementation does not match X-25"


class Stats:
    def __init__(self):
        self.can_ok = 0
        self.sensor_ok = 0
        self.crc_errors = 0
        self.unknown_type = 0
        self.skipped_bytes = 0
        self.cangen_gaps = 0          # frames missing according to the cangen counter
        self.last_cangen = None
        self.last_sensor = None

    def summary(self) -> str:
        s = (f"CAN ok={self.can_ok}  sensor ok={self.sensor_ok}  "
             f"CRC err={self.crc_errors}  unknown type={self.unknown_type}  "
             f"skipped bytes={self.skipped_bytes}  cangen gaps={self.cangen_gaps}")
        if self.last_sensor:
            t, p, rpm, st = self.last_sensor
            s += f"\n  last sensor: {t:.2f} degC  {p:.2f} hPa  {rpm:.1f} RPM  [{st}]"
        return s


def decode_can(rec: bytes, st: Stats, writer):
    ts, can_id, dlc = struct.unpack(">IHB", rec[3:10])
    data = rec[10:18]
    st.can_ok += 1

    # cangen -D i puts a little-endian incrementing counter in the payload
    counter = int.from_bytes(data[:4], "little")
    if st.last_cangen is not None:
        expected = (st.last_cangen + 1) & 0xFFFFFFFF
        if counter != expected:
            st.cangen_gaps += (counter - expected) & 0xFFFFFFFF
    st.last_cangen = counter

    if writer:
        writer.writerow(["can", ts, hex(can_id), dlc, data[:min(dlc, 8)].hex(), "", "", "", ""])


def decode_sensor(rec: bytes, st: Stats, writer):
    ts, temp, press, fast, status = struct.unpack(">IiIiB", rec[3:20])
    temp_c = temp / 100.0
    press_hpa = press / 256.0 / 100.0
    counts_per_tick = fast / EMA_SCALE
    rpm = counts_per_tick / (COUNTS_PER_REV * CONTROL_TICK_S) * 60.0
    st_name = STATUS_NAME.get(status, f"?{status}")
    st.sensor_ok += 1
    st.last_sensor = (temp_c, press_hpa, rpm, st_name)

    if writer:
        writer.writerow(["sensor", ts, "", "", "", f"{temp_c:.2f}", f"{press_hpa:.2f}", f"{rpm:.2f}", st_name])


def parse(buf: bytearray, st: Stats, writer):
    """Consume as many complete records from buf as possible."""
    while True:
        idx = buf.find(SYNC)
        if idx < 0:
            # keep a possible half sync byte at the end
            keep = 1 if buf[-1:] == SYNC[:1] else 0
            st.skipped_bytes += len(buf) - keep
            del buf[:len(buf) - keep]
            return
        if idx > 0:
            st.skipped_bytes += idx
            del buf[:idx]

        if len(buf) < 3:
            return
        rtype = buf[2]
        length = RECORD_LEN.get(rtype)
        if length is None:
            st.unknown_type += 1
            del buf[:1]          # false sync, rescan from the next byte
            continue
        if len(buf) < length:
            return

        rec = bytes(buf[:length])
        crc_rx = (rec[-2] << 8) | rec[-1]
        if crc16_x25(rec[2:-2]) != crc_rx:
            st.crc_errors += 1
            del buf[:1]          # resync: maybe the real sync is inside
            continue

        if rtype == TYPE_CAN:
            decode_can(rec, st, writer)
        else:
            decode_sensor(rec, st, writer)
        del buf[:length]


def main():
    ap = argparse.ArgumentParser(description="Decode the STM32 logger stream")
    ap.add_argument("--port", default="/dev/ttyUSB0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--csv", help="optional CSV output file")
    args = ap.parse_args()

    st = Stats()
    buf = bytearray()
    csv_file = open(args.csv, "w", newline="") if args.csv else None
    writer = None
    if csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["type", "timestamp_ms", "can_id", "dlc", "data",
                         "temp_c", "press_hpa", "rpm", "status"])

    try:
        with serial.Serial(args.port, args.baud, timeout=0.1) as ser:
            ser.reset_input_buffer()
            last_print = time.monotonic()
            print(f"Listening on {args.port} @ {args.baud}. Ctrl+C to stop.")
            while True:
                chunk = ser.read(4096)
                if chunk:
                    buf.extend(chunk)
                    parse(buf, st, writer)
                if time.monotonic() - last_print >= 1.0:
                    print(st.summary(), flush=True)
                    last_print = time.monotonic()
    except KeyboardInterrupt:
        pass
    except serial.SerialException as e:
        print(f"Serial error: {e}", file=sys.stderr)
    finally:
        if csv_file:
            csv_file.close()
        print("\n=== FINAL ===")
        print(st.summary())


if __name__ == "__main__":
    main()