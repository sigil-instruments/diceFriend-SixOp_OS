# SixOp_OS v0.2 (2026-11-06)

- CV1 spike guard (`src/cv_spike_guard.h`, called first in the CV1 interrupt). An unpatched CV1 could catch a single noise spike; under the CV > MIDI > internal clock priority that spike took the clock for 20 s and silenced a running MIDI Clock. Now a pulse must stay high for at least 50 µs, and CV1 takes over only after two edges 1 ms to 10 s apart. Once CV1 owns the clock, every edge counts at once.
- Cost: the first CV1 pulse after silence does not play; triggers more than 10 s apart are ignored.
- Synthesis, controls, MIDI and pinout are unchanged.

Hardware status: the same guard was tested on hardware in other diceFriend OSes (MIDI Clock with an unpatched CV1, CV clock, CV triggers). SixOp_OS v0.2 itself has not been played on hardware yet.

# SixOp_OS v0.1 publication preparation

- Canonical project name: SixOp_OS_40; display name: SixOp_OS; firmware: SixOp_OS_40_L.hex.
- Pinned Teensy platform 5.1.0, Teensyduino 1.60 and GCC 11.3.1.
- Queue overflow now schedules All Notes Off instead of silently dropping a release.
- TS MIDI Clock continues to strike free voices after MIDI notes have been played. Held pad/MIDI voices retain ownership.
- Synthesis, pinout, pot takeover and layer LED mapping are unchanged.
- Added standalone English manual, Quick Start and developer guide.

Hardware status: the author tested the original sound and controls successfully. The two MIDI changes above need a short hardware regression check before publishing the rebuilt HEX as hardware-verified. Native audio, firmware build and software regression results are recorded in VALIDATION.md.
