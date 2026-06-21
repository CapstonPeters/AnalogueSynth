import struct, os

def write_midi_file(path):
    """Write a multi-track MIDI file to test the synth across styles."""
    
    tempo = 500000  # 120 BPM
    ticks_per_beat = 480
    
    def var_len(value):
        if value < 0:
            value = 0
        buf = bytearray()
        buf.append(value & 0x7F)
        value >>= 7
        while value > 0:
            buf.append(0x80 | (value & 0x7F))
            value >>= 7
        buf.reverse()
        return bytes(buf)
    
    def make_track(track_name, events):
        data = bytearray()
        name_bytes = track_name.encode('ascii')[:127]
        data.append(0x00); data.append(0xFF); data.append(0x03)
        data.append(len(name_bytes)); data.extend(name_bytes)
        data.append(0x00); data.append(0xFF); data.append(0x51)
        data.append(0x03); data.extend(struct.pack('>I', tempo)[1:])
        data.append(0x00); data.append(0xFF); data.append(0x58)
        data.append(0x04); data.extend([4, 2, 24, 8])
        
        last_time = 0
        for ev in events:
            delta = ev['time'] - last_time
            last_time = ev['time']
            data.extend(var_len(delta))
            if ev['type'] == 'note_on':
                data.extend([0x90 | ev['channel'], ev['note'], ev['velocity']])
            elif ev['type'] == 'note_off':
                data.extend([0x80 | ev['channel'], ev['note'], 64])
            elif ev['type'] == 'program':
                data.extend([0xC0 | ev['channel'], ev['program']])
            elif ev['type'] == 'cc':
                data.extend([0xB0 | ev['channel'], ev['cc'], ev['value']])
            elif ev['type'] == 'pitch_bend':
                val = ev['value'] + 8192
                lsb = val & 0x7F
                msb = (val >> 7) & 0x7F
                data.extend([0xE0 | ev['channel'], lsb, msb])
        
        data.append(0x00); data.append(0xFF); data.append(0x2F); data.append(0x00)
        track_data = bytes(data)
        return b'MTrk' + struct.pack('>I', len(track_data)) + track_data
    
    # Track 1: Lead melody (C minor)
    t1 = []
    t = 0
    t1.append({'time': 0, 'type': 'program', 'channel': 0, 'program': 0})
    t1.append({'time': 0, 'type': 'cc', 'channel': 0, 'cc': 7, 'value': 110})
    t1.append({'time': 0, 'type': 'cc', 'channel': 0, 'cc': 1, 'value': 40})
    
    melody = [60, 63, 67, 72, 75, 74, 72, 67, 65, 63, 60]
    for i, note in enumerate(melody):
        t1.append({'time': t, 'type': 'note_on', 'channel': 0, 'note': note, 'velocity': 90 + (i % 3) * 10})
        t += int(ticks_per_beat * 0.85)
        t1.append({'time': t, 'type': 'note_off', 'channel': 0, 'note': note, 'velocity': 64})
        t += int(ticks_per_beat * 0.15)
    
    # Pitch bend on last held note
    for bend in range(-4000, 4000, 200):
        t1.append({'time': t, 'type': 'pitch_bend', 'channel': 0, 'value': bend})
        t += 40
    
    # Track 2: Bass line
    t2 = []
    t = 0
    t2.append({'time': 0, 'type': 'program', 'channel': 1, 'program': 1})
    t2.append({'time': 0, 'type': 'cc', 'channel': 1, 'cc': 7, 'value': 100})
    
    bass = [36, 36, 48, 36, 39, 39, 48, 39, 43, 43, 55, 43, 41, 41, 53, 41]
    eighth = ticks_per_beat // 2
    for note in bass:
        t2.append({'time': t, 'type': 'note_on', 'channel': 1, 'note': note, 'velocity': 100})
        t += int(eighth * 0.8)
        t2.append({'time': t, 'type': 'note_off', 'channel': 1, 'note': note, 'velocity': 64})
        t += int(eighth * 0.2)
    
    # Track 3: Pad chords
    t3 = []
    t = 0
    t3.append({'time': 0, 'type': 'program', 'channel': 2, 'program': 2})
    t3.append({'time': 0, 'type': 'cc', 'channel': 2, 'cc': 7, 'value': 70})
    t3.append({'time': 0, 'type': 'cc', 'channel': 2, 'cc': 1, 'value': 60})
    
    chords = [
        ([60, 63, 67], ticks_per_beat * 8),
        ([65, 68, 72], ticks_per_beat * 8),
        ([67, 70, 74], ticks_per_beat * 8),
        ([60, 63, 67], ticks_per_beat * 8),
    ]
    for chord_notes, duration in chords:
        for note in chord_notes:
            t3.append({'time': t, 'type': 'note_on', 'channel': 2, 'note': note, 'velocity': 72})
        t += duration
        for note in chord_notes:
            t3.append({'time': t, 'type': 'note_off', 'channel': 2, 'note': note, 'velocity': 64})
    
    # Track 4: Arpeggio
    t4 = []
    t = ticks_per_beat * 4
    t4.append({'time': t, 'type': 'program', 'channel': 3, 'program': 3})
    t4.append({'time': t, 'type': 'cc', 'channel': 3, 'cc': 7, 'value': 85})
    
    arp = [60, 63, 67, 70, 72, 75, 79, 72, 67, 63]
    step = ticks_per_beat // 4
    for note in arp:
        t4.append({'time': t, 'type': 'note_on', 'channel': 3, 'note': note, 'velocity': 80})
        t += int(step * 0.9)
        t4.append({'time': t, 'type': 'note_off', 'channel': 3, 'note': note, 'velocity': 64})
        t += int(step * 0.1)
    
    # Track 5: Portamento lead
    t5 = []
    t = 0
    t5.append({'time': 0, 'type': 'program', 'channel': 4, 'program': 4})
    t5.append({'time': 0, 'type': 'cc', 'channel': 4, 'cc': 7, 'value': 105})
    t5.append({'time': 0, 'type': 'cc', 'channel': 4, 'cc': 65, 'value': 127})
    t5.append({'time': 0, 'type': 'cc', 'channel': 4, 'cc': 5, 'value': 40})
    
    glide = [72, 67, 74, 70, 67, 65, 72, 74, 77, 79, 84, 79, 77, 72]
    for note in glide:
        t5.append({'time': t, 'type': 'note_on', 'channel': 4, 'note': note, 'velocity': 95})
        t += ticks_per_beat * 2
        t5.append({'time': t, 'type': 'note_off', 'channel': 4, 'note': note, 'velocity': 64})
    
    tracks = [
        ("Lead Synth", t1),
        ("Bass Line", t2),
        ("Pads", t3),
        ("Arpeggio", t4),
        ("Portamento Lead", t5),
    ]
    
    for name, events in tracks:
        events.sort(key=lambda e: e['time'])
    
    num_tracks = len(tracks)
    header = b'MThd' + struct.pack('>I', 6)
    header += struct.pack('>H', 1)
    header += struct.pack('>H', num_tracks)
    header += struct.pack('>H', ticks_per_beat)
    
    track_chunks = b''
    for name, events in tracks:
        track_chunks += make_track(name, events)
    
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'wb') as f:
        f.write(header)
        f.write(track_chunks)
    
    total = sum(len(ev) for _, ev in tracks)
    print(f"Written {path} ({len(header) + len(track_chunks)} bytes, {total} events)")

write_midi_file('/home/jo/AnalogSynth/test/demo-midi/test-track.mid')
