# Publication preparation validation

Date: 2026-11-02.

- PlatformIO build passed: Teensy platform 5.1.0, Teensyduino 1.60, GCC 11.3.1, USB_MIDI_SERIAL.
- Native renderer passed: 1,158,144 stereo frames, peak 0.954, RMS 0.280, no NaNs.
- Queue regression passed: overflow schedules CC123, queued attacks are discarded, subsequent Note On/Off ordering is preserved, 10,000 wraparound operations pass.
- TS clock's persistent note-driven latch has been removed; occupied MIDI/pad voices still retain priority.
- Synth engine namespace fm is byte-for-byte unchanged from the original tested source.
- Manual and one-page A4 Quick Start checked against code and rendered for visual review.
- Hardware: original version tested successfully by the author. Rebuilt release has not been flashed by this preparation task. Confirm queue-overflow recovery and MIDI Clock in TS after playing/releasing MIDI notes on hardware.

Source package excludes local build output, editor state, credentials and native audio previews.
