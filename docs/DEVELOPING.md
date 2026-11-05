# Building your own diceFriend OS

SixOp_OS is a complete commented reference project. Copy the project, change OS_NAME and replace the sound engine while retaining the hardware and USB identity.

1. Read the opening guide in src/main.cpp. It contains the full control map, pinout, layer LED truth table and recovery paths.
2. Replace namespace fm (section 3). Keep prepare() per block and render(left, right) per sample. Audio interrupt code must not allocate memory, print or access USB.
3. Adapt the parameter names and buildPatch(). Values are normalized to 0-1. Preserve takeover at startup, layer/mode changes, DICE and incoming panel CC.
4. Define musical steps and pad actions. Preserve audible MIDI Note On in AS, matching releases in TS, and CV > MIDI > internal clock ownership.
5. Keep receiveMidi() free of USB sends. Queue local output and drain it in serviceOut(); never echo host messages onto the same USB port.
6. Preserve the entire switch-to-LED mapping, pot inversion, PT8211 output and CapacitiveSensor pinout. Do not add another LED or pot inversion.
7. Keep bootloader gesture, startup rescue and Serial BOOTLOADER command.

Build with the pinned platformio.ini. Native audio preview tests synthesis only; it does not establish hardware or MIDI correctness. Before releasing a derivative, check audible AS MIDI notes, TS note release, clock takeover, local output without echo, mode changes and recovery on hardware.

The MIT license permits derivatives. Retain the copyright and license notice; name and document your derivative so users can identify its author and hardware target. Do not imply official Sigil Instruments approval of a community derivative.

Third-party code is not included in this source package: Teensyduino, Teensy Audio and CapacitiveSensor are supplied through the pinned PlatformIO framework and retain their respective notices. Inspect those dependency notices before redistributing their source or binary components.
