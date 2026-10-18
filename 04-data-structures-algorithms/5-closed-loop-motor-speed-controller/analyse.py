import sys
import statistics as st

def load(path):
    meta, rows = {}, []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            if line.startswith("#"):
                for pair in line[1:].split(","):
                    if "=" in pair:
                        key, value = pair.strip().split("=")
                        meta[key] = int(value)
                continue
            parts = line.split(",")
            if len(parts) != 5:
                continue # half-received line at the start or the end
            try:
                rows.append([int(p) for p in parts])
            except ValueError:
                continue
    return meta, rows


def analyse(path):
    meta, rows = load(path)
    scale = meta.get("scale", 32)
    rows = [r for r in rows if r[4] != 0] # drop the fake tick-0 line

    ticks = [r[4] for r in rows]
    gaps = [(a, b) for a, b in zip(ticks, ticks[1:]) if b - a != 1]

    raw = [r[0] for r in rows]
    filt = [r[1] / scale for r in rows]
    pid = [r[3] for r in rows]

    print(f"{path}: mode={meta.get('mode', '?')} samples={len(rows)} gaps={len(gaps)}")
    print(f" raw    mean {st.mean(raw):.3f}  std  {st.pstdev(raw):.3f}")
    print(f" filt   mean {st.mean(filt):.3f}  std  {st.pstdev(filt):.3f}")
    print(f" pid    min {min(pid)}  max {max(pid)}  p-p  {max(pid) - min(pid)}  std  {st.pstdev(pid):.2f}")


for path in sys.argv[1:]:
    analyse(path)