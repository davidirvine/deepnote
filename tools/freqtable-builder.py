chord = [
    # Example: root is D
    # F6 1396.91 +3 semitones
    # D6 1174.66 +0 semitones
    # E5 659.25  +2 semitones
    # D5 587.33  +0 semitones
    # A4 440.00  +7 semitones
    # D3 146.83  +0 semitones
    # A2 110.0   +7 semitones
    # D2 73.42   +0 semitones
    # D1 36.71   +0 semitones
    {'octave': 6, 'offset': 3}, # High octave to low
    {'octave': 6, 'offset': 0},
    {'octave': 5, 'offset': 2},
    {'octave': 5, 'offset': 0},
    {'octave': 4, 'offset': 7},
    {'octave': 3, 'offset': 0},
    {'octave': 2, 'offset': 7},
    {'octave': 2, 'offset': 0},
    {'octave': 1, 'offset': 0}
]

note_numbers = {
    "C": 0,
    "C#": 1,
    "D": 2,
    "D#": 3,
    "E": 4,
    "F": 5,
    "F#": 6,
    "G": 7,
    "G#": 8,
    "A": 9,
    "A#": 10,
    "B": 11
}

freq_table = {}
for root_note in note_numbers.values():
    voice_freqs = []
    for voice_note in chord:
        # see: https://en.wikipedia.org/wiki/Piano_key_frequencies
        key = -8 + (voice_note['octave'] * 12) + voice_note['offset'] + root_note
        voice_freqs.append(440 * 2 ** ((key - 49) / 12))
    freq_table[root_note] = voice_freqs

for key in freq_table.keys():
    voice_notes = freq_table.get(key)
    print("{} {} {:.2f}f, {:.2f}f, {:.2f}f, {:.2f}f, {:.2f}f, {:.2f}f, {:.2f}f, {:.2f}f, {:.2f}f {},".format(
            key,
            '{', 
                round(voice_notes[0], 2),
                round(voice_notes[1], 2),
                round(voice_notes[2], 2),
                round(voice_notes[3], 2),
                round(voice_notes[4], 2),
                round(voice_notes[5], 2),
                round(voice_notes[6], 2),
                round(voice_notes[7], 2),
                round(voice_notes[8], 2),
            '}'
    ))
