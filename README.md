# SixOp_OS v0.1

Play it as a complete instrument, or use its commented code to build your own diceFriend OS.

- [Download the ready-to-install HEX](firmware/SixOp_OS_40_L.hex)
- [Read the English manual](docs/SixOp_OS_40_manual_EN.md)
- [Download the manual PDF](docs/SixOp_OS_40_manual_EN.pdf)
- [Download the Quick Start card](docs/SixOp_OS_Quick_Start_A4_EN.pdf)
- [Browse all official diceFriend OS downloads](https://github.com/sigil-instruments/diceFriend)

## Make your own OS

Fork this repository or download it with Code > Download ZIP. Open the folder in PlatformIO, build the `teensy40` environment, then follow [the developer guide](docs/DEVELOPING.md) to replace the synthesis engine. Keep the hardware pinout, controls and USB identity described in the source comments.

Publish your derivative on your own GitHub account and [open an issue here](https://github.com/sigil-instruments/diceFriend-SixOp_OS/issues/new) with its name, author, source link, license and hardware test status. We can add it to the community list below. Improvements to this starter project are welcome through pull requests.

## Community OSes

Your project can be the first entry. Community projects retain their authorship and individual test status.

A six-operator FM synthesizer and experimental diceFriend OS template for
Teensy 4.0, PT8211 audio output and LEFT hardware. It provides three voices,
eight algorithm routings, operator feedback, envelopes, LFO, lo-fi colour
and stereo echo. It is inspired by DX7 architecture; it is not a bit-exact
emulation and does not load DX7 SysEx patches.

AS is an autonomous drone mode. TS plays voices from touch pads, MIDI notes
and CV triggers. The complete control map, MIDI behaviour, LED indications,
bootloader gestures and instructions for adapting the engine are in the
opening comment of [src/main.cpp](src/main.cpp).

## Build and upload

Use PlatformIO with the Arduino framework and `teensy40` environment:

```sh
pio run -e teensy40
pio run -e teensy40 -t upload
```

The pinned toolchain is PlatformIO Teensy platform 5.1.0, Teensyduino 1.60
and GCC 11.3.1. The explicit package versions are in `platformio.ini`.
CapacitiveSensor is supplied by Teensyduino. USB is MIDI + Serial and the
device name stays `diceFriend v.01` when adapting this template.

For firmware updates and recovery, see the BOOTLOADER box in
`src/main.cpp`: automatic upload, the Serial `BOOTLOADER` command, the
SH + DICE 10-second gesture, and the power-up rescue gesture.

If the normal update methods fail, the Teensy PROGRAM button can also be
pressed through the small emergency access hole in the enclosure.
See the [illustrated emergency access guide](docs/BOOTLOADER_RECOVERY.md).

## Native audio preview

This renders the engine on a computer without Teensy hardware:

```sh
c++ -O2 -std=c++17 test/render_native.cpp -o render
./render
```

It writes `sixop_preview.wav` in the current directory and reports peak,
RMS and detected NaNs. It does not test the hardware controls or USB MIDI.

## Source packaging

Include `README.md`, `LICENSE`, `platformio.ini`, `.gitignore`, `.gitattributes`,
`src/main.cpp`, `src/usb_identity.c`, `test/render_native.cpp` and `docs/`.

Keep `.pio/`, `.vscode/`, caches and generated native preview files out of
public source packages. Local editor and build files can contain the
computer account name and absolute paths. `.gitignore` excludes them from
normal Git additions; `.gitattributes` also excludes them from `git archive`
exports. Neither file filters a ZIP made by directly compressing the folder.

## License

SixOp_OS source code and documentation are released under the
[MIT License](LICENSE). You may use, modify, share and sell copies, including
as part of closed-source projects, provided you retain the copyright and
license notice. There is no requirement to publish your modifications.

Third-party dependencies remain subject to their own licenses.

## User and developer documentation

- [English manual](docs/SixOp_OS_40_manual_EN.md)
- [English manual PDF](docs/SixOp_OS_40_manual_EN.pdf)
- [One-page Quick Start](docs/SixOp_OS_Quick_Start_A4_EN.pdf)
- [Adapting this project](docs/DEVELOPING.md)
- [Release changes and validation](docs/RELEASE_NOTES.md)

The original sound and controls were tested on hardware by the author.
The release changes to MIDI overflow recovery and TS clock behaviour require
a short hardware regression check; a native render does not exercise USB MIDI.
