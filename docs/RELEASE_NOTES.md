# SixOp_OS v0.1 publication preparation

- Canonical project name: SixOp_OS_40; display name: SixOp_OS; firmware: SixOp_OS_40_L.hex.
- Pinned Teensy platform 5.1.0, Teensyduino 1.60 and GCC 11.3.1.
- Queue overflow now schedules All Notes Off instead of silently dropping a release.
- TS MIDI Clock continues to strike free voices after MIDI notes have been played. Held pad/MIDI voices retain ownership.
- Synthesis, pinout, pot takeover and layer LED mapping are unchanged.
- Added standalone English manual, Quick Start and developer guide.

Hardware status: the author tested the original sound and controls successfully. The two MIDI changes above need a short hardware regression check before publishing the rebuilt HEX as hardware-verified. Native audio, firmware build and software regression results are recorded in VALIDATION.md.
