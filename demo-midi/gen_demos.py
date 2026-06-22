import struct, os

os.makedirs('/home/jo/AnalogSynth/demo-midi', exist_ok=True)

def var_len(value):
    if value < 0: value = 0  # safety
    buf = bytearray()
    buf.append(value & 0x7F)
    value >>= 7
    while value:
        buf.append(0x80 | (value & 0x7F))
        value >>= 7
    buf.reverse()
    return bytes(buf)

def create_midi(filename, tempo_bpm, tracks_data):
    ticks_per_quarter = 480
    tempo_us = int(60_000_000 / tempo_bpm)
    
    header = b'MThd' + struct.pack('>IHHH', 6, 1, len(tracks_data), ticks_per_quarter)
    
    track_chunks = []
    for track_notes in tracks_data:
        # Sort by tick
        track_notes = sorted(track_notes, key=lambda x: x[0])
        track = bytearray()
        track += b'\x00\xFF\x51\x03' + struct.pack('>I', tempo_us)[1:]
        track += b'\x00\xFF\x58\x04\x04\x02\x18\x08'
        
        current_tick = 0
        for tick, event_type, note, vel in track_notes:
            delta = tick - current_tick
            track += var_len(delta)
            track += bytes([0x90, note, vel]) if event_type == 'on' else bytes([0x80, note, 0])
            current_tick = tick
        
        track += var_len(0) + b'\xFF\x2F\x00'
        chunk = b'MTrk' + struct.pack('>I', len(track)) + bytes(track)
        track_chunks.append(chunk)
    
    with open(filename, 'wb') as f:
        f.write(header)
        for chunk in track_chunks:
            f.write(chunk)
    sz = os.path.getsize(filename)
    print(f"  {os.path.basename(filename):30s} {sz:6d} bytes")

q = 480; h = q//2; e = h//2

# === 1. Chord Progression ===
chord = []
roots = [45, 41, 48, 43]  # A, F, C, G
for i, root in enumerate(roots):
    t = i * q * 4
    offs = [0, 3, 7, 12] if i < 2 else [0, 4, 7, 12]
    for o in offs:
        chord.append((t, 'on', root+o, 75))
        chord.append((t+q*4-1, 'off', root+o, 0))
create_midi('/home/jo/AnalogSynth/demo-midi/01-chord-progression.mid', 100, [chord])

# === 2. Bass Line ===
bass = []
ns = [36, 39, 41, 43, 46, 48, 51, 53]
pat = [0, 2, 4, 3, 1, 0, 5, 7, 6, 5, 3, 4, 2, 0, 1, 3]
for i, idx in enumerate(pat):
    t = i * e
    bass.append((t, 'on', ns[idx], 100))
    bass.append((t+e-3, 'off', ns[idx], 0))
create_midi('/home/jo/AnalogSynth/demo-midi/02-bass-line.mid', 128, [bass])

# === 3. Arp Chord ===
arp = []
for n in [48, 51, 55, 58]:
    arp.append((0, 'on', n, 80))
    arp.append((q*16, 'off', n, 0))
create_midi('/home/jo/AnalogSynth/demo-midi/03-arp-chord-cmin7.mid', 120, [arp])

# === 4. Lead Melody ===
ml = [60,63,65,67,68,67,65,63,60,58,56,55,56,58,60,63,65,67,68,72,75,72,70,68,67,65,63,65,67,65,63,60]
dl = [q,q,q,q,q,q,q,q,h,h,q,q,q,q,q,q,q,q,h,q,q,q,q,q,q,q,q,q,q,q,q,q*2]
lead = []
t = 0
for n, d in zip(ml, dl):
    lead.append((t, 'on', n, 90))
    lead.append((t+d-3, 'off', n, 0))
    t += d
create_midi('/home/jo/AnalogSynth/demo-midi/04-lead-melody.mid', 120, [lead])

# === 5. Full Demo ===
t1 = [(t, e, n, v) for t, e, n, v in chord]
t2 = [(t+q*8, e, n, v) for t, e, n, v in bass]
t3 = [(t+q*16, e, n, v) for t, e, n, v in lead]
create_midi('/home/jo/AnalogSynth/demo-midi/05-full-demo.mid', 120, [t1, t2, t3])

print("\nDone — 5 demo MIDI files in demo-midi/")
