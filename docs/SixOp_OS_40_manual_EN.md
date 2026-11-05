# SixOp_OS

Version 0.1 - diceFriend, Teensy 4.0. English manual.

SixOp_OS is the open-source showcase OS for diceFriend: a complete, commented example that can be studied, modified and used as a starting point for new systems. As an instrument, it is a three-voice, six-operator FM design. Eight algorithm routings, operator feedback, envelope characters, vibrato, lo-fi colour and stereo echo range from soft drones and electric-piano attacks to metallic bells and rough bass. It is inspired by DX7 architecture; it is not a bit-exact emulation and does not import DX7 SysEx patches.

## Quick start

- Connect USB power and the stereo audio output; begin with a low listening level.
- The instrument starts in AS, an autonomous three-voice drone. Select the NS layer and use P1 for root, P2 for chord and P3 for algorithm.
- Start with moderate DEPTH and low FEEDBACK. Raise them gradually for more complex spectra.
- Select SH to add vibrato, detuning and stereo echo. PULSE fully left keeps a continuous drone; raising it adds repeated strikes.
- Hold DICE on NS for 3 seconds to enter TS. Play T1-T3; P1-P3 set their pitches and P4-P6 their decays.
- Tap DICE in TS to cycle TINE, BELL, BASS and BRASS envelope characters.

## Layers and knob takeover

The two layer LEDs show NS or SH. Switching layers or modes, randomizing with DICE, and receiving a panel MIDI CC re-arm the affected controls. Stored values remain unchanged until a knob moves at least 8% of its full travel from the position captured when armed. It then glides to the filtered knob position over 200 ms. Crossing the stored value is not required. Startup also retains the initial patch until this movement threshold is met.

## AS controls

| Knob | NS layer | SH layer |
|---|---|---|
| P1 | PITCH: root, C1-C6 | LFO DEPTH: vibrato and timbre |
| P2 | CHORD: eight voicings | LFO RATE |
| P3 | ALGORITHM: eight routings | DETUNE: operators and voices |
| P4 | RATIO: harmonic to metallic | PULSE: repeated strikes; fully left is drone |
| P5 | DEPTH: FM index | GRIT: lo-fi DAC colour |
| P6 | FEEDBACK | SPACE: stereo echo |

AS continuously sustains a three-voice chord. T1 adds brightness, T2 adds metallic colour and T3 adds growl. Touch pressure controls the amount; every new touch also re-strikes its corresponding voice. CV, MIDI Clock and internal PULSE re-strike the drone. A MIDI Note On sets the root and produces an immediate audible strike; its pitch is folded into the C1-C6 range. Note Off does not stop the drone.

## TS controls

| Knob | NS layer | SH layer |
|---|---|---|
| P1 | Voice 1 pitch | ALGORITHM |
| P2 | Voice 2 pitch | RATIO |
| P3 | Voice 3 pitch | DEPTH |
| P4 | Voice 1 decay | FEEDBACK |
| P5 | Voice 2 decay | CONTOUR |
| P6 | Voice 3 decay | DETUNE |

T1-T3 open their respective voices. Holding a pad holds the gate; TINE and BELL naturally decay, while BASS and BRASS can sustain. The held pad follows its pitch knob. GRIT, SPACE and LFO RATE retain their AS settings in TS. MIDI mod wheel supplies TS LFO depth.

Incoming MIDI notes use three-voice allocation. Repeated notes reuse their voice; otherwise a free voice is used, then the oldest non-pad voice. Pads have priority and cannot be stolen by MIDI. Note Off and Note On with velocity zero release the matching MIDI note and channel. MIDI-held voices expire after 30 seconds as a stuck-note safeguard. CV and MIDI Clock strike voices that are not held by a pad or MIDI note; after those notes are released, free voices remain clock-playable.

## DICE

- NS tap in AS: randomize the NS layer and audition the new scene.
- NS tap in TS: next envelope character, TINE / BELL / BASS / BRASS.
- SH tap, shorter than 1 second: randomize the SH layer of the current mode.
- NS hold, 3 seconds: switch AS/TS and release previous notes.
- SH hold, 10 seconds: enter the bootloader.

The gesture belongs to the layer where the press began. Changing SHIFT during a hold cancels it. Releasing a SH hold between 1 and 10 seconds also cancels it. AS randomization preserves PULSE. TS randomization preserves the six NS pitch/decay values.

## CV and clock priority

CV1 accepts trigger/clock edges with an approximately 1 ms filter. It re-strikes AS or the free TS voices. CV2 accepts pulses as a secondary clock; steady voltage adds FM depth. CV1 blocks CV2 steps while it owns the clock. A cable already high at startup does not create a CV2 edge.

Clock priority is CV, then MIDI, then internal PULSE. MIDI steps are blocked for 20 seconds after the last valid CV edge. Internal timing returns only after 30 seconds without an active external source. Between 20 and 30 seconds after the last CV edge, the instrument waits unless MIDI is active. Internal PULSE operates only in AS. A continuous AS drone remains audible while clocks are waiting or stopped.

## USB MIDI In

- MIDI + Serial device name: diceFriend v.01.
- Channel messages: channels 1-15. Channel 16 is reserved for local output and ignored on input.
- Note On: sets AS root and strikes, or allocates a pitched TS voice. Velocity controls the strike.
- Note Off / Note On velocity zero: releases the matching TS voice.
- Clock: 24 PPQN; one quarter note produces a step, subject to CV priority.
- Start: runs transport and resets the clock counter. Continue resumes it.
- Stop / System Reset: pauses MIDI-clock steps and releases TS voices except held pads. CV triggers remain active and AS remains a drone. Note On re-enables transport.
- CC20-25: NS parameters P1-P6 of the current mode. CC26-31: SH parameters P1-P6.
- CC1: mod wheel / LFO depth. CC74 and channel pressure: brightness.
- Pitch Bend: +/-2 semitones, shared by the instrument.
- CC120/123: releases TS voices and queues All Notes Off on output channel 16. Envelope and echo tails can remain; AS ignores these messages.

## USB MIDI Out

Local steps and pads generate notes on channel 16. Panel movements send CC20-31, and smoothed CV2 sends rate-limited CC74. CV timing can generate Start, 24 PPQN Clock and Stop in the supported 20-300 BPM range. Internal PULSE generates notes but does not generate MIDI Clock. Incoming channel-16 messages are ignored; received MIDI messages are not directly retransmitted. Keep generated clock out of a DAW route feeding the same instrument input.

Output is queued outside the receiver. If the queue fills, pending events are discarded and All Notes Off is scheduled before subsequent events, protecting the host from a silently lost release.

## LEDs

- NS and SH: active knob layer.
- First lower LED: step, trigger and note activity.
- Second lower LED: CV2 level.
- Third and fourth lower LEDs: left and right output level.
- Turning the algorithm knob temporarily shows index 0-7 in binary on the last three lower LEDs. This is the local list index, not the DX algorithm number.
- TS DICE shows the character with a one-hot pattern. Mode changes flash the lower LEDs. A bootloader hold shows a countdown.

## Bootloader and recovery

Normal PlatformIO upload can reboot the MIDI + Serial device automatically. Alternatively send the line BOOTLOADER through the Serial monitor at 115200, or hold SH + DICE for 10 seconds. Release early or change SHIFT to cancel. For startup rescue, select SH, hold DICE while connecting USB, and keep holding for 3 seconds. The internal Teensy PROGRAM button is the final recovery option.

## Patch ideas

- Soft chord: moderate DEPTH, low FEEDBACK, slight DETUNE and SPACE. Move ALGORITHM before increasing modulation.
- Clocked metal: raise RATIO and DEPTH, then use external CV1 or MIDI Clock. Add GRIT carefully.
- Played electric piano: switch to TS, select TINE, set three pitches and medium decays, then add a little SPACE in AS before returning to TS.

## Open source and license

SixOp_OS is published as the open-source showcase system for diceFriend. Its GitHub package includes the commented reference implementation, a native audio renderer and developer instructions. Code and documentation use the MIT License; dependencies keep their own licenses.
