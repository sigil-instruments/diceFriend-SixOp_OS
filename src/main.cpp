/* ============================================================================
   SixOp_OS v0.1  —  DX7-style 6-operator FM, 3 voices, for diceFriend
   Teensy 4.0 + PT8211, LEFT hardware.  Framework pilot / OS template.
   ============================================================================

   ┌──────────────────────────────────────────────────────────────────────────┐
   │ 0. READ ME FIRST  (humans and AI assistants)                             │
   └──────────────────────────────────────────────────────────────────────────┘

   WHAT THIS IS
   A complete, working diceFriend OS that doubles as a template for writing
   your own.  The sound engine is a small DX7-style FM synth: every voice has
   six sine "operators" that modulate each other according to an ALGORITHM.
   Three such voices run at once.

     AS mode (Autonomous Sound) .. a three-voice FM drone that plays by itself.
     TS mode (Touch Sound) ....... silent until you play: the 3 touch pads,
                                   MIDI notes and CV triggers strike voices.

   "DX7-style" means the architecture (6 operators, DX algorithm routings,
   operator feedback, ratios, envelopes, LFO, lo-fi DAC colour).  It is NOT a
   bit-exact DX7 emulation and it does not load DX7 patches (SysEx).  The
   algorithm numbers follow the DX7 chart; if you add more, check them
   against the official DX7 algorithm diagram.

   FILES
     README.md ........... project overview, build and source packaging.
     src/main.cpp ........ THIS FILE.  Everything except the USB name.
     src/usb_identity.c .. USB device name "diceFriend v.01" (keep it).
     platformio.ini ...... build settings (USB type MIDI + Serial).
     test/render_native.cpp  renders the sound engine to a WAV on a computer,
                             no hardware needed.

   BUILD (PlatformIO)
     - Board teensy40, framework arduino, build flag -D USB_MIDI_SERIAL.
     - Tested toolchain: Teensyduino core 1.60 (TEENSYDUINO=160) + GCC 11.3.1.
     - Library: CapacitiveSensor (ships with Teensyduino).

   ┌──────────────────────────────────────────────────────────────────────────┐
   │ CONTROLS                                                                 │
   └──────────────────────────────────────────────────────────────────────────┘
   The SHIFT switch selects a layer: NS (normal) or SH (shift).  Six pots per
   layer, so 12 parameters per mode.  AS and TS have separate parameter sets.

   AS — DRONE                NS layer               SH layer
     pot 1 ..................  PITCH (C1..C6)          LFO DEPTH (vibrato+timbre)
     pot 2 ..................  CHORD (8 voicings)      LFO RATE
     pot 3 ..................  ALGORITHM (8)           DETUNE (operators+voices)
     pot 4 ..................  RATIO (harmonic→metal)  PULSE (self re-strike rate,
                                                        fully left = pure drone)
     pot 5 ..................  DEPTH (FM index)        GRIT (DX-style lo-fi DAC)
     pot 6 ..................  FEEDBACK                SPACE (stereo echo)
     pads ...................  T1 BRIGHT  T2 METAL  T3 GROWL (pressure = amount,
                               each touch also re-strikes its voice)

   TS — PLAYED               NS layer               SH layer
     pot 1..3 ...............  PITCH voice 1/2/3       ALGORITHM / RATIO / DEPTH
     pot 4..6 ...............  DECAY voice 1/2/3       FEEDBACK / CONTOUR / DETUNE
     pads ...................  T1/T2/T3 play voice 1/2/3. Holding keeps the gate
                               open; TINE/BELL decay, BASS/BRASS sustain.
     GRIT, SPACE and LFO RATE are set in AS and stay active in TS.

   DICE button
     tap (NS) ............ AS: randomize NS layer   TS: next CHARACTER
                           (TINE / BELL / BASS / BRASS = envelope families)
     tap (SH, < 1 s) ..... randomize the SH layer of the current mode
     hold 3 s (NS) ....... switch AS <-> TS
     hold 10 s (SH) ...... BOOTLOADER  (see the box below)

   POTS take over after you move them 8 % (then glide 200 ms).  Switching
   layer/mode, DICE and MIDI CC re-arm them, so nothing jumps.

   CV
     CV1 ... trigger/clock.  AS: re-strikes the drone.  TS: strikes all voices.
     CV2 ... pulses = same as CV1; steady voltage = extra FM DEPTH.
     Clock priority: CV > MIDI clock > internal PULSE (CV owns for 20 s,
     internal returns after 30 s of silence).

   MIDI (USB)  — listens on channels 1..15, sends on channel 16
     Note On ...... AS: sets drone root + audible strike.  TS: 3-voice poly.
     Note Off ..... TS: releases the note (Note On velocity 0 does the same).
                    TS MIDI notes also release automatically after 30 s.
                    AS: ignored (it is a drone).
     Clock ........ 24 PPQN; each quarter note triggers an AS step or free TS
                    voices, subject to CV priority. Held pad/MIDI voices keep
                    their ownership; free voices remain clock-playable.
     Start ........ runs transport and resets clock count.
     Continue ..... resumes transport.
     Stop ......... pauses MIDI clock steps and releases TS voices except pads.
                    CV triggers remain active; AS drone keeps sounding.
     CC 20-31 ..... the 12 parameters of the current mode (NS 20-25, SH 26-31)
     CC 1 ......... mod wheel -> LFO depth.   CC 74 -> brightness.
     Pitch bend ... +/- 2 semitones.          Channel pressure -> brightness.
     CC 120/123 ... TS: both release voices and send All Notes Off on ch 16.
                    Envelope/echo tails may remain. AS: both are ignored.
     OUT (ch 16) .. notes from steps and pads, CC 20-31 from pots,
                    CC 74 from CV2, MIDI Clock converted from CV.
     Channel messages on incoming ch 16 are ignored to avoid replaying our
     own notes and CCs. Clock/Start/Stop/Continue have no MIDI channel and
     are still received. Do not route our outgoing clock back into this input.
     Received clock/transport/CC/bend/pressure are not echoed directly.

   LEDs
     NS / SH ...... which layer the pots are on (see the LED INVERSION box)
     LED 1 ........ step / trigger / note activity
     LED 2 ........ CV2 level
     LED 3 / 4 .... output level left / right
     Overlays: mode change flash; turning the ALGORITHM pot shows its index
     0..7 in binary on LEDs 2-4 for 1.2 s (not the DX algorithm number).
     DICE/MIDI algorithm changes do not show this overlay. CHARACTER is
     shown one-hot after a TS DICE tap; bootloader has a countdown.

   ╔══════════════════════════════════════════════════════════════════════════╗
   ║ BOOTLOADER  (firmware updates and emergency PROGRAM button access)     ║
   ╠══════════════════════════════════════════════════════════════════════════╣
   ║ To enter the bootloader, use the options below in order. The physical   ║
   ║ PROGRAM button is accessible through the emergency hole in the case.  ║
   ║                                                                          ║
   ║  1. Just upload.  With a USB type that includes Serial (this OS uses    ║
   ║     MIDI + Serial) the upload tool can usually reboot the Teensy        ║
   ║     into the bootloader by itself.                                       ║
   ║  2. Serial: send the line  BOOTLOADER  in a serial monitor (115200).    ║
   ║  3. Gesture while running: SHIFT switch to SH, hold DICE 10 s.          ║
   ║     After 1 s the four LEDs count up; in the last second they blink.    ║
   ║     Let go any time before that to cancel.  Flipping SHIFT cancels too. ║
   ║  4. Power-up rescue: SHIFT to SH, hold DICE, plug in USB, keep holding  ║
   ║     3 s (LEDs blink).  Works even if the main loop of the OS is broken. ║
   ║  5. Emergency PROGRAM button access through the enclosure hole:        ║
   ║     Locate the small hole below and to the right of the USB-C port.    ║
   ║     See docs/BOOTLOADER_RECOVERY.md and its annotated photo.           ║
   ║     Gently insert a thin straight wire or a blunt needle. The button   ║
   ║     is a few millimetres below the enclosure surface; press gently     ║
   ║     until you feel a slight click, then remove the tool.               ║
   ║     Do not force it or push a sharp object deep into the enclosure.    ║
   ║                                                                          ║
   ║ IF YOU WRITE YOUR OWN OS: keep section 10 (bootRescueAtPowerUp, the    ║
   ║ 10 s gesture and enterBootloader) and keep calling serviceDice() inside ║
   ║ every long loop. These provide convenient software recovery before    ║
   ║ the physical PROGRAM button needs to be reached through its hole.     ║
   ╚══════════════════════════════════════════════════════════════════════════╝

   ╔══════════════════════════════════════════════════════════════════════════╗
   ║ LED INVERSION  (NS / SH indicator)  — read before touching it           ║
   ╠══════════════════════════════════════════════════════════════════════════╣
   ║ On LEFT hardware the two layer LEDs are deliberately driven "crossed":  ║
   ║                                                                          ║
   ║   switch pin 1 LOW  -> shift = false -> pin 2 (LED_NS) HIGH, pin 3 LOW  ║
   ║   switch pin 1 HIGH -> shift = true  -> pin 2 LOW, pin 3 (LED_SH) HIGH  ║
   ║                                                                          ║
   ║ in code:   digitalWrite(LED_NS, !shift);  digitalWrite(LED_SH, shift);   ║
   ║                                                                          ║
   ║ This inversion must exist EXACTLY ONCE along the whole path:            ║
   ║ physical switch -> how SHIFT is read -> LED pin numbers -> how the LED  ║
   ║ is written -> physical diode.  It is verified on hardware (same as      ║
   ║ FractureOS v14 / GruzOS).  If the LEDs look "wrong" in your OS, check   ║
   ║ the whole path — do not add a second "!" somewhere to fix it, and do    ║
   ║ not remove this one because it "looks like a bug".  It is not a bug.    ║
   ║ LEDs 1-4 (pins 4,5,6,9) are normal: HIGH / 255 = on.                    ║
   ╚══════════════════════════════════════════════════════════════════════════╝

   POT INVERSION (LEFT): the pots are mounted so that the raw reading goes
   down when you turn clockwise.  readPot() returns 1 - raw, so in the rest of
   the code 0.0 = fully left and 1.0 = fully right.  Do it in one place only.

   ┌──────────────────────────────────────────────────────────────────────────┐
   │ HOW TO MAKE YOUR OWN OS FROM THIS FILE                                  │
   └──────────────────────────────────────────────────────────────────────────┘
   Sections 1-2 and 5-11 are the "platform": pins, pots, pads, DICE, CV,
   MIDI, LEDs, bootloader.  Section 3 (namespace fm) is the sound engine and
   section 4 connects it to the Teensy Audio Library.  To write a new OS:
     a) replace section 3 with your own engine (keep render(l, r) per sample,
        prepare() per 128-sample block, no memory allocation, no USB, no
        Serial inside it — it runs in the audio interrupt),
     b) rename the 12 parameters in section 6 and map them in buildPatch(),
     c) decide what a "step" does in step() (section 8),
     d) change OS_NAME.  Keep the USB name "diceFriend v.01".

   ┌──────────────────────────────────────────────────────────────────────────┐
   │ IF YOU ARE AN AI ASSISTANT EDITING THIS FILE                            │
   └──────────────────────────────────────────────────────────────────────────┘
   These rules come from bugs found on real hardware.  Follow them.
     - Do NOT change pin numbers.  Pin 7 is the DAC (PT8211 DIN), not a button.
     - Never call analogRead() on PIN_CV1: it carries an interrupt; reading it
       disables the interrupt.
     - Keep the LED inversion exactly once (box above) and the pot inversion
       only in readPot().
     - Touch pads use the CapacitiveSensor library.  touchRead() does not
       exist on Teensy 4.0.
     - Output goes through AudioOutputPT8211 only.
     - In receiveMidi(): only read and change state.  Never call usbMIDI.send*
       or send_now() there.  Outgoing MIDI is queued and sent by serviceOut().
     - Never echo received MIDI back to USB (no "thru" on the same port).
     - No Serial.print in hot paths; diagnostics() checks availableForWrite()
       so it never blocks when no terminal is open (iPad hosts block).
     - Use clampf() (NaN-safe) for anything that can blow up, e.g. feedback.
     - Code that runs in the audio interrupt (section 3) must not allocate,
       print, or touch USB.  Code in loop() talks to the engine only through
       the AudioSixOp wrapper, which locks the audio interrupt briefly.
     - Prefer small, targeted changes.  Do not "clean up" working platform code.
   ========================================================================== */

#include <cmath>
#include <cstdint>
#include <cstring>

#ifndef SIXOP_NATIVE_TEST          // defined only by test/render_native.cpp
#include <Arduino.h>
#include <Audio.h>
#include <CapacitiveSensor.h>
#endif

#define OS_NAME    "SixOp_OS"
#define OS_VERSION "0.1"

/* ============================================================================
   1. HARDWARE MAP  (diceFriend v2, Teensy 4.0, LEFT) — verified, do not edit
   ========================================================================== */
namespace hw {
constexpr uint8_t PIN_SHIFT = 1;    // SHIFT switch, INPUT_PULLUP, SH layer = HIGH
constexpr uint8_t PIN_DICE  = 8;    // DICE button, INPUT_PULLUP, pressed = LOW
constexpr uint8_t PIN_CV1   = 10;   // CV1 trigger -> interrupt (never analogRead it)
constexpr uint8_t PIN_CV2   = 22;   // CV2 analog input
constexpr uint8_t LED_NS    = 2;    // layer LEDs (see LED INVERSION box)
constexpr uint8_t LED_SH    = 3;
constexpr uint8_t LEDS[4]   = {4, 5, 6, 9};             // PWM, active HIGH
constexpr uint8_t POTS[6]   = {14, 15, 16, 17, 18, 19};
constexpr uint8_t TOUCH_SEND = 11;                      // shared send pin, 1 MOhm
constexpr uint8_t TOUCH_RECV[3] = {23, 0, 24};          // T1, T2, T3
// Audio: AudioOutputPT8211 uses DIN=7, LRCLK=20, BCK=21 (fixed by the library).
}

/* ============================================================================
   2. SMALL HELPERS (shared by engine and platform)
   ========================================================================== */
// NaN-safe clamp: "!(x >= lo)" is true for NaN, so NaN becomes lo.
static inline float clampf(float x, float lo = 0.0f, float hi = 1.0f) {
  if (!(x >= lo)) return lo;
  if (x > hi) return hi;
  return x;
}
static inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }

/* ============================================================================
   3. SOUND ENGINE  —  namespace fm   (runs inside the audio interrupt)
   ----------------------------------------------------------------------------
   Pure C++ with <cmath>.  No Arduino calls, so it also compiles on a computer
   (test/render_native.cpp).  Replace this whole section to make another OS.

   Signal flow per voice:
       OP6 -> ... -> OP1 according to the ALGORITHM   (modulators bend the
       phase of the operators below them; carriers are summed to the output)
   then: 3 voices -> pan -> GRIT (lo-fi) -> SPACE (echo) -> DC block -> limiter
   ========================================================================== */
namespace fm {

constexpr float kSampleRate = 44117.647f;   // Teensy audio rate (not 44100)
constexpr int   kOps    = 6;
constexpr int   kVoices = 3;
constexpr int   kBlock  = 128;              // AUDIO_BLOCK_SAMPLES

static inline float noteToHz(float note) { return 440.0f * powf(2.0f, (note - 69.0f) / 12.0f); }
// Coefficient that makes a value fall by 60 dB in `seconds` (per sample).
static inline float fallCoef(float seconds) {
  return expf(-6.9f / (fmaxf(seconds, 0.0005f) * kSampleRate));
}
// Per-sample increment for the attack curve (reaches full level in ~seconds).
static inline float attackRate(float seconds) {
  return clampf(1.47f / (fmaxf(seconds, 0.0005f) * kSampleRate), 0.00001f, 1.0f);
}

/* ---- ALGORITHMS ------------------------------------------------------------
   Operators are numbered 1..6 like on the DX7.  OP(n) is the bit for op n.
   mod[j] lists which operators modulate operator j+1.  `carriers` are the
   operators you hear.  `feedback` is the operator that modulates itself.
   To add an algorithm: add a line, keep kNumAlgorithms in sync, and check it
   against the DX7 algorithm chart (higher op numbers modulate lower ones). */
constexpr uint8_t OP(int n) { return uint8_t(1u << (n - 1)); }

struct Algorithm {
  const char* name;
  uint8_t mod[kOps];   // mod[0] = what modulates OP1, ... mod[5] = OP6
  uint8_t carriers;    // bit mask of audible operators
  uint8_t feedback;    // 1..6
};

// Ordered from "one dense stack" to "six sines" so the pot sweeps from
// complex to organ-like.  Index 0..7 is what the ALGORITHM pot selects.
const Algorithm kAlgorithms[] = {
  //  name    OP1 is modulated by          OP2    OP3          OP4    OP5    OP6  carriers                    fb
  {"DX16", {OP(2) | OP(3) | OP(5),        0,     OP(4),       0,     OP(6), 0}, OP(1),                       6},
  {"DX1",  {OP(2),                        0,     OP(4),       OP(5), OP(6), 0}, OP(1) | OP(3),               6},
  {"DX2",  {OP(2),                        0,     OP(4),       OP(5), OP(6), 0}, OP(1) | OP(3),               2},
  {"DX7",  {OP(2),                        0,     OP(4) | OP(5), 0,   OP(6), 0}, OP(1) | OP(3),               6},
  {"DX5",  {OP(2),                        0,     OP(4),       0,     OP(6), 0}, OP(1) | OP(3) | OP(5),       6},
  {"DX19", {OP(2),                        OP(3), 0,           OP(6), OP(6), 0}, OP(1) | OP(4) | OP(5),       6},
  {"DX22", {OP(2),                        0,     OP(6),       OP(6), OP(6), 0}, OP(1) | OP(3) | OP(4) | OP(5), 6},
  {"DX32", {0,                            0,     0,           0,     0,     0}, 0x3F,                        6},
};
constexpr int kNumAlgorithms = sizeof(kAlgorithms) / sizeof(kAlgorithms[0]);

/* ---- RATIO FAMILIES ---------------------------------------------------------
   Frequency multipliers for OP1..OP6.  The RATIO pot travels through these
   families; near each family it rests on exact values (clean, in tune),
   between them it glides (inharmonic, alive).  Later families are bells and
   metal.  OP1 stays at 1 so the pitch is always the played note. */
const float kRatios[][kOps] = {
  {1, 1,      1,     1,    1,      1    },   // unison: round, sine-like
  {1, 1,      1,     2,    1,      3    },   // e-piano / tine family
  {1, 2,      1,     2,    2,      4    },   // reedy, square-ish
  {1, 3,      1,     3,    2,      5    },   // hollow, clarinet-ish
  {1, 4,      2,     5,    3,      7    },   // bright harmonic stack
  {1, 1.414f, 1,     3.5f, 0.5f,   7.07f},   // bell
  {1, 3.5f,   2,     7,    1.414f, 11.3f},   // glass / gong
  {1, 5.19f,  1.73f, 9.8f, 2.3f,   13.4f},   // metal
};
constexpr int kNumRatioFamilies = sizeof(kRatios) / sizeof(kRatios[0]);

// Each modulator gets a slightly different weight so DEPTH does not move all
// of them identically (sounds more like a hand-made DX patch).
const float kModWeight[kOps] = {1.00f, 1.00f, 0.85f, 0.90f, 0.75f, 0.80f};
// Each modulator decays at a slightly different speed -> evolving spectrum.
const float kModDecaySpread[kOps] = {1.00f, 0.80f, 1.25f, 0.65f, 1.50f, 0.70f};
// Detune pattern per operator, in "units" scaled by the DETUNE pot.
const float kOpDetune[kOps] = {0, 1, -1, 2, -2, 3};

/* ---- CHARACTERS (TS) ---------------------------------------------------------
   Envelope families, chosen with a DICE tap in TS.  Times in seconds.
   Carriers use `car`, modulators use `mod`.  DECAY pots scale carrier decay
   and release; CONTOUR scales modulator times.  velIndex = how much velocity
   (pad pressure / MIDI velocity) changes brightness. */
struct EnvShape { float attack, decay, sustain, release; };
struct Character { const char* name; EnvShape car, mod; float velIndex; };
const Character kCharacters[] = {
  {"TINE",  {0.002f, 2.5f, 0.00f, 0.40f}, {0.001f, 0.35f, 0.08f, 0.30f}, 0.90f},
  {"BELL",  {0.002f, 6.0f, 0.00f, 2.00f}, {0.002f, 2.50f, 0.15f, 1.50f}, 0.60f},
  {"BASS",  {0.003f, 1.2f, 0.55f, 0.12f}, {0.001f, 0.18f, 0.25f, 0.10f}, 0.80f},
  {"BRASS", {0.040f, 1.0f, 0.80f, 0.25f}, {0.090f, 0.60f, 0.65f, 0.25f}, 0.50f},
};
constexpr int kNumCharacters = sizeof(kCharacters) / sizeof(kCharacters[0]);

/* ---- PATCH: everything the control side tells the engine ---------------------
   Plain numbers, mostly 0..1.  Written by loop(), read by the audio interrupt
   (copied under a lock in AudioSixOp::setPatch). */
struct Patch {
  bool    drone = true;           // true = AS (held, slow), false = TS (played)
  uint8_t algorithm = 6;          // index into kAlgorithms
  uint8_t character = 0;          // index into kCharacters (TS)
  float ratio = 0.2f;             // 0..1 through kRatios
  float depth = 0.35f;            // FM index
  float feedback = 0.15f;
  float detune = 0.2f;
  float contour = 0.5f;           // TS: modulator envelope time
  float voiceDecay[kVoices] = {0.5f, 0.5f, 0.5f};   // TS: per voice
  float lfoDepth = 0.2f, lfoRate = 0.3f;
  float grit = 0.0f, space = 0.25f;
  float bright = 0.0f;            // extra depth: pads, CV2, CC74, pressure
  float metal = 0.0f;             // ratio push toward inharmonic (AS pad T2)
  float growl = 0.0f;             // extra feedback (AS pad T3)
  float bendSemis = 0.0f;
  float targetNote[kVoices] = {36, 43, 48};
  bool  retune[kVoices] = {true, true, true};  // follow targetNote this block?
};

/* ---- ENVELOPE ------------------------------------------------------------------
   Attack -> decay to sustain -> release.  Exponential like the DX7 EG, but
   with one time per stage instead of four rates/levels. */
struct Env {
  enum Stage : uint8_t { Idle, Attack, Decay, Release };
  Stage stage = Idle;
  float level = 0, atk = 0.01f, dec = 0.9999f, sus = 1.0f, rel = 0.9999f;
  void on()  { stage = Attack; }           // restarts from the current level
  void off() { if (stage != Idle) stage = Release; }
  inline float next() {
    switch (stage) {
      case Attack:  level += atk * (1.3f - level); if (level >= 1.0f) { level = 1.0f; stage = Decay; } break;
      case Decay:   level = sus + (level - sus) * dec; break;
      case Release: level *= rel; if (level < 0.00001f) { level = 0; stage = Idle; } break;
      default: break;
    }
    return level;
  }
};

class Engine {
public:
  Engine() {
    for (int i = 0; i <= kSineSize; ++i) sineTable[i] = sinf(6.28318530718f * i / kSineSize);
    for (int v = 0; v < kVoices; ++v) voices[v].lfoPhase = 0.31f * v;
  }

  // ---- commands from the control side (call with the audio lock held) ----
  void setPatch(const Patch& p) { patch = p; }
  void noteOn(int v, float note, float velocity) {
    Voice& x = voices[v % kVoices];
    x.note = x.targetNote = note;
    x.velocity = clampf(velocity);
    x.freshPitch = true;
    for (Env& e : x.env) e.on();
  }
  void noteOff(int v) { for (Env& e : voices[v % kVoices].env) e.off(); }
  void droneStart() { for (int v = 0; v < kVoices; ++v) { voices[v].velocity = 1; for (Env& e : voices[v].env) e.on(); } }
  void releaseAll() { for (int v = 0; v < kVoices; ++v) noteOff(v); }
  void burst(int v, float amount) { Voice& x = voices[v % kVoices]; x.burst = fmaxf(x.burst, clampf(amount)); }
  void panic() {   // fast silence (mode change to bootloader, CC120)
    for (Voice& x : voices) for (Env& e : x.env) { e.rel = fallCoef(0.02f); e.off(); }
    panicBlocks = 4;
  }
  float energy(int v) const { return voices[v % kVoices].energy; }
  float meterL() const { return meter[0]; }
  float meterR() const { return meter[1]; }
  // True when voice v is no longer audible (carrier envelope finished or ~0).
  bool  voiceSilent(int v) const {
    const Env& e = voices[v % kVoices].env[0];
    return e.stage == Env::Idle || (e.stage != Env::Attack && e.level < 0.002f);
  }

  // ---- once per 128-sample block: turn the Patch into per-operator numbers ----
  void prepare() {
    const Algorithm& alg = kAlgorithms[patch.algorithm % kNumAlgorithms];
    fbOp = alg.feedback - 1;
    carrierMask = alg.carriers;
    int nCar = 0; for (int j = 0; j < kOps; ++j) if (carrierMask & (1 << j)) ++nCar;
    carrierNorm = 1.0f / powf(float(nCar), 0.75f);

    // RATIO: position between two families, with flat "rest" zones.
    float r = clampf(patch.ratio + patch.metal * 0.35f) * (kNumRatioFamilies - 1);
    int fam = int(r); if (fam > kNumRatioFamilies - 2) fam = kNumRatioFamilies - 2;
    float f = clampf((r - fam - 0.15f) / 0.70f); f = f * f * (3 - 2 * f);

    float depth = clampf(patch.depth + patch.bright * 0.45f);
    float depthCurve = depth * depth * (0.6f + 0.4f * depth);
    float fbAmt = powf(clampf(patch.feedback + patch.growl * 0.5f), 1.5f) * 0.42f;
    float detuneCents = patch.detune * patch.detune * 14.0f;
    float lfoHz = 0.05f * powf(240.0f, patch.lfoRate);          // 0.05 .. 12 Hz
    float lfoDepth = clampf(patch.lfoDepth + patch.growl * 0.25f);
    const Character& ch = kCharacters[patch.character % kNumCharacters];
    float contourMul = 0.1f * powf(60.0f, patch.contour);       // x0.1 .. x6
    float swell = patch.contour > 0.7f ? (patch.contour - 0.7f) * 2.5f : 0.0f;
    float glideK = patch.drone ? 1.0f - expf(-kBlock / (0.08f * kSampleRate)) : 1.0f;
    float burstFall = expf(-kBlock / (0.35f * kSampleRate));
    fbAmount = fbAmt;

    for (int v = 0; v < kVoices; ++v) {
      Voice& x = voices[v];
      if (patch.retune[v]) x.targetNote = patch.targetNote[v];
      x.note += (x.targetNote - x.note) * (x.freshPitch ? 1.0f : glideK);

      // LFO (triangle), each voice at a slightly different speed.
      static const float kLfoSpread[kVoices] = {1.0f, 1.13f, 0.87f};
      x.lfoPhase += lfoHz * kLfoSpread[v] * kBlock / kSampleRate;
      if (x.lfoPhase >= 1.0f) x.lfoPhase -= 1.0f;
      float lfo = 1.0f - 4.0f * fabsf(x.lfoPhase - 0.5f);        // -1..1
      float vibrato = lfo * lfoDepth * lfoDepth * 0.35f;          // semitones
      float spread = patch.drone ? (v - 1) * patch.detune * 0.12f : 0.0f;   // AS voice spread

      float hz = noteToHz(x.note + patch.bendSemis + vibrato + spread);
      float highest = 1.0f;
      for (int j = 0; j < kOps; ++j) {
        float ratio = lerpf(kRatios[fam][j], kRatios[fam + 1][j], f);
        float det = powf(2.0f, kOpDetune[j] * detuneCents / 1200.0f);
        float opHz = clampf(hz * ratio * det, 1.0f, 18000.0f);
        if (!(carrierMask & (1 << j)) && opHz > highest) highest = opHz;
        x.incTarget[j] = opHz / kSampleRate;
        if (x.freshPitch) x.inc[j] = x.incTarget[j];
      }
      x.freshPitch = false;

      // Less FM at high pitch, otherwise it turns into aliasing noise.
      float guard = clampf(9000.0f / (highest + hz), 0.15f, 1.0f);
      float velIndex = patch.drone ? 1.0f : 1.0f - ch.velIndex * (1.0f - x.velocity);
      float amd = 1.0f + lfo * lfoDepth * 0.6f;
      x.burst *= burstFall;
      float index = depthCurve * 1.4f * guard * velIndex * amd * (1.0f + x.burst * 1.5f);
      // TS gets x2: single played notes would otherwise be ~10 dB under the drone.
      x.amp = (patch.drone ? 1.0f : 2.0f * (0.35f + 0.65f * x.velocity)) * (1.0f + x.burst * 0.25f);

      for (int j = 0; j < kOps; ++j) {
        bool car = carrierMask & (1 << j);
        x.level[j] = car ? 1.0f : index * kModWeight[j];
        Env& e = x.env[j];
        if (patch.drone) {                     // AS: slow fade in, hold forever
          e.atk = attackRate(car ? 1.2f : 2.5f);
          e.dec = 0.9999f; e.sus = 1.0f;
          if (!panicBlocks) e.rel = fallCoef(1.5f);
        } else if (car) {                      // TS carrier: DECAY pot
          float dm = 0.08f * powf(50.0f, patch.voiceDecay[v]);   // x0.08 .. x4
          e.atk = attackRate(ch.car.attack);
          e.dec = fallCoef(ch.car.decay * dm);
          e.sus = ch.car.sustain;
          if (!panicBlocks) e.rel = fallCoef(ch.car.release * fmaxf(dm, 0.25f));
        } else {                               // TS modulator: CONTOUR pot
          e.atk = attackRate(ch.mod.attack + swell);
          e.dec = fallCoef(ch.mod.decay * contourMul * kModDecaySpread[j]);
          e.sus = ch.mod.sustain;
          if (!panicBlocks) e.rel = fallCoef(ch.mod.release * contourMul);
        }
      }
    }
    if (panicBlocks) --panicBlocks;

    // GRIT: DX-style lo-fi.  Fewer bits and a lower effective sample rate.
    gritOn = patch.grit > 0.02f;
    gritSteps = powf(2.0f, 15.0f - patch.grit * 10.0f);
    gritHold = 1 + int(patch.grit * patch.grit * 7.0f);
    spaceSmooth += 0.13f * (patch.space - spaceSmooth);
  }

  // ---- once per sample ----
  inline void render(float& left, float& right) {
    static const float kPanL[kVoices] = {0.85f, 0.71f, 0.52f};
    static const float kPanR[kVoices] = {0.52f, 0.71f, 0.85f};
    float l = 0, r = 0;
    for (int v = 0; v < kVoices; ++v) {
      float s = voiceSample(voices[v]);
      voices[v].energy += 0.0015f * (fabsf(s) - voices[v].energy);
      l += s * kPanL[v];
      r += s * kPanR[v];
    }
    l *= 0.62f; r *= 0.62f;

    if (gritOn) {                               // sample & hold + bit crush
      if (++gritCount >= gritHold) {
        gritCount = 0;
        gritL = roundf(l * gritSteps) / gritSteps;
        gritR = roundf(r * gritSteps) / gritSteps;
      }
      l = gritL; r = gritR;
    }

    // SPACE: two crossed delay lines (~136 ms / ~179 ms), ping-pong.
    float dl = delayL[(delayPos + kDelaySize - 5981) & kDelayMask];
    float dr = delayR[(delayPos + kDelaySize - 7877) & kDelayMask];
    float fbk = 0.12f + 0.45f * spaceSmooth;
    delayL[delayPos] = l + dr * fbk;
    delayR[delayPos] = r + dl * fbk;
    delayPos = (delayPos + 1) & kDelayMask;
    l = l * (1 - 0.35f * spaceSmooth) + dl * spaceSmooth * 0.8f;
    r = r * (1 - 0.35f * spaceSmooth) + dr * spaceSmooth * 0.8f;

    // DC blocker, then a soft limiter that only acts on peaks above 0.82.
    float hl = l - dcInL + 0.9993f * dcOutL;  dcInL = l; dcOutL = hl;
    float hr = r - dcInR + 0.9993f * dcOutR;  dcInR = r; dcOutR = hr;
    float peak = fmaxf(fabsf(hl), fabsf(hr)), g = 1.0f;
    if (peak > 0.82f) { float ex = peak - 0.82f; g = (0.82f + 0.16f * ex / (0.16f + ex)) / peak; }
    left  = clampf(hl * g, -0.98f, 0.98f);
    right = clampf(hr * g, -0.98f, 0.98f);
    if (!(left == left) || !(right == right)) { left = right = 0; resetFilters(); }   // NaN guard
    meter[0] += 0.002f * (fabsf(left) - meter[0]);
    meter[1] += 0.002f * (fabsf(right) - meter[1]);
  }

private:
  static constexpr int kSineSize = 1024;
  static constexpr int kDelaySize = 8192, kDelayMask = kDelaySize - 1;

  struct Voice {
    float phase[kOps] = {}, inc[kOps] = {}, incTarget[kOps] = {}, level[kOps] = {};
    Env env[kOps];
    float fb1 = 0, fb2 = 0;                 // feedback history (DX: average of two)
    float note = 48, targetNote = 48;
    float velocity = 0, amp = 1, burst = 0, lfoPhase = 0, energy = 0;
    bool freshPitch = true;
  } voices[kVoices];

  Patch patch;
  float sineTable[kSineSize + 1] = {};
  float delayL[kDelaySize] = {}, delayR[kDelaySize] = {};
  int delayPos = 0;
  int fbOp = 5;
  uint8_t carrierMask = 1;
  float carrierNorm = 1, fbAmount = 0, spaceSmooth = 0.25f;
  bool gritOn = false; float gritSteps = 32768; int gritHold = 1, gritCount = 0;
  float gritL = 0, gritR = 0;
  float dcInL = 0, dcOutL = 0, dcInR = 0, dcOutR = 0;
  float meter[2] = {0, 0};
  int panicBlocks = 0;

  // Sine of a phase given in cycles (1.0 = one full turn), table + interpolation.
  inline float sine(float cycles) const {
    cycles -= floorf(cycles);
    float pos = cycles * kSineSize;
    int i = int(pos);
    if (unsigned(i) >= unsigned(kSineSize)) i = 0;       // also catches NaN
    return sineTable[i] + (sineTable[i + 1] - sineTable[i]) * (pos - i);
  }

  // One voice, one sample.  Operators are computed 6 -> 1 so every
  // modulator is ready before the operator it modulates.
  inline float voiceSample(Voice& x) {
    const Algorithm& alg = kAlgorithms[patch.algorithm % kNumAlgorithms];
    float out[kOps];
    float sum = 0;
    for (int j = kOps - 1; j >= 0; --j) {
      float mod = 0;
      for (int k = j + 1; k < kOps; ++k) if (alg.mod[j] & (1 << k)) mod += out[k];
      if (j == fbOp) mod += fbAmount * (x.fb1 + x.fb2) * 0.5f;
      x.inc[j] += 0.02f * (x.incTarget[j] - x.inc[j]);     // smooth pitch/ratio moves
      x.phase[j] += x.inc[j];
      if (x.phase[j] >= 1.0f) x.phase[j] -= 1.0f;
      float s = sine(x.phase[j] + mod) * x.env[j].next();
      if (j == fbOp) { x.fb2 = x.fb1; x.fb1 = s; }
      out[j] = s * x.level[j];
      if (carrierMask & (1 << j)) sum += out[j];
    }
    if (!(x.fb1 == x.fb1)) x.fb1 = x.fb2 = 0;               // NaN guard
    return sum * carrierNorm * x.amp;
  }

  void resetFilters() {
    dcInL = dcOutL = dcInR = dcOutR = 0;
    memset(delayL, 0, sizeof(delayL)); memset(delayR, 0, sizeof(delayR));
    for (Voice& x : voices) x.fb1 = x.fb2 = 0;
  }
};

}  // namespace fm

#ifndef SIXOP_NATIVE_TEST   // ---- everything below needs the Teensy ----

/* ============================================================================
   4. AUDIO GLUE — connects fm::Engine to the Teensy Audio Library
   ----------------------------------------------------------------------------
   update() is called by the audio interrupt every 128 samples (~2.9 ms).
   loop() must not touch `engine` directly; use the wrappers below.  They
   pause the audio interrupt for a few microseconds so the engine never sees
   a half-written command.
   ========================================================================== */
class AudioSixOp : public AudioStream {
public:
  AudioSixOp() : AudioStream(0, nullptr) {}
  void setPatch(const fm::Patch& p)          { AudioNoInterrupts(); engine.setPatch(p);       AudioInterrupts(); }
  void noteOn(int v, float note, float vel)  { AudioNoInterrupts(); engine.noteOn(v, note, vel); AudioInterrupts(); }
  void noteOff(int v)                        { AudioNoInterrupts(); engine.noteOff(v);        AudioInterrupts(); }
  void burst(int v, float amount)            { AudioNoInterrupts(); engine.burst(v, amount);  AudioInterrupts(); }
  void droneStart()                          { AudioNoInterrupts(); engine.droneStart();      AudioInterrupts(); }
  void releaseAll()                          { AudioNoInterrupts(); engine.releaseAll();      AudioInterrupts(); }
  void panic()                               { AudioNoInterrupts(); engine.panic();           AudioInterrupts(); }
  void meters(float& l, float& r)            { AudioNoInterrupts(); l = engine.meterL(); r = engine.meterR(); AudioInterrupts(); }
  bool voiceSilent(int v)                    { AudioNoInterrupts(); bool s = engine.voiceSilent(v); AudioInterrupts(); return s; }

  void update() override {
    audio_block_t* l = allocate();
    audio_block_t* r = allocate();
    if (!l || !r) { if (l) release(l); if (r) release(r); return; }
    engine.prepare();
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
      float a, b;
      engine.render(a, b);
      l->data[i] = int16_t(a * 32767.0f);
      r->data[i] = int16_t(b * 32767.0f);
    }
    transmit(l, 0); transmit(r, 1);
    release(l); release(r);
  }
private:
  fm::Engine engine;
};

AudioSixOp        synth;
AudioOutputPT8211 dac;                       // diceFriend v2 DAC — only this one
AudioConnection   patchL(synth, 0, dac, 0), patchR(synth, 1, dac, 1);

/* ============================================================================
   5. PANEL INPUT — pots, DICE, touch pads   (shared diceFriend standard,
      same behaviour as GruzOS; reuse these in your own OS)
   ========================================================================== */

// LEFT hardware: invert the physical reading here and nowhere else.
// 0.0 = fully left, 1.0 = fully right.
float readPot(int i) { return 1.0f - analogRead(hw::POTS[i]) / 1023.0f; }

// Pot takeover: a parameter keeps its value until the pot is moved 8 % away
// from where it was when the layer changed; then it glides to the pot in
// 200 ms (no jump) and follows it.  Call update() every 5 ms.
struct PotTakeover {
  float anchor = 0, filtered = 0, start = 0, elapsed = 0;
  bool active = false, transition = false;
  void arm(float physical) { anchor = filtered = physical; active = transition = false; elapsed = 0; }
  // Returns true while the pot owns the value.
  bool update(float physical, float& value, float dt = 0.005f) {
    filtered += (1 - expf(-dt / 0.020f)) * (physical - filtered);
    if (!active) {
      if (fabsf(filtered - anchor) < 0.08f) return false;
      active = transition = true; start = value; elapsed = 0;
      return true;                                   // first frame keeps old value
    }
    if (transition) {
      elapsed += dt;
      float t = elapsed / 0.200f;
      if (t >= 1) { t = 1; transition = false; }
      value = start + (filtered - start) * (t * t * (3 - 2 * t));
    } else {
      value = filtered;
    }
    if (!transition && fabsf(value - physical) < 0.001f) value = physical;   // reach 0 and 1
    return true;
  }
};

// DICE gestures.  The gesture is locked to the layer it started in;
// flipping SHIFT during a hold cancels it.
struct DiceButton {
  enum Event { None, Short, Long, Boot };
  bool raw = false, down = false, fired = false, gestureShift = false, rawShift = false;
  uint32_t changedAt = 0, pressedAt = 0;
  Event update(bool pressed, uint32_t now, bool shift) {
    if (pressed != raw) { raw = pressed; changedAt = now; if (pressed && !down) rawShift = shift; }
    if (down && shift != gestureShift) fired = true;                     // cancel
    if (raw != down && uint32_t(now - changedAt) >= 30) {               // 30 ms debounce
      down = raw;
      if (down) { pressedAt = now; gestureShift = rawShift; fired = (shift != gestureShift); }
      else if (!fired) {
        // In SH only a tap (< 1 s) counts; a longer hold that is released
        // before 10 s is a cancelled bootloader attempt and does nothing.
        if (!gestureShift || uint32_t(now - pressedAt) < 1000) return Short;
      }
    }
    if (down && !fired && uint32_t(now - pressedAt) >= (gestureShift ? 10000u : 3000u)) {
      fired = true;
      return gestureShift ? Boot : Long;
    }
    return None;
  }
  // 0..1 progress of a running bootloader hold (for the LED countdown).
  float bootProgress(uint32_t now) const {
    if (!down || fired || !gestureShift) return 0;
    uint32_t held = now - pressedAt;
    return held < 1000 ? 0 : clampf((held - 1000) / 9000.0f);
  }
};

// Touch pad with its own baseline and noise tracking (GruzOS TouchGate).
struct TouchGate {
  float baseline = 0, noise = 0, delta = 0;
  bool initialized = false, on = false;
  uint32_t started = 0, lastGood = 0;
  unsigned confirm = 0;
  bool update(long normalizedRaw, uint32_t now) {
    if (normalizedRaw < 0) {                       // sensor timeout
      confirm = 0; if (uint32_t(now - lastGood) >= 30) on = false;
      return on;
    }
    lastGood = now;
    float raw = float(normalizedRaw);
    if (!initialized) { initialized = true; baseline = raw; started = now; }
    if (uint32_t(now - started) < 300) { if (raw < baseline) baseline = raw; on = false; return false; }
    delta = raw - baseline;
    float threshold = fmaxf(3200.0f, noise * 8);
    bool desired = on ? delta > threshold * 0.5625f : delta > threshold;
    if (desired != on) { if (++confirm >= 2) { on = desired; confirm = 0; } } else confirm = 0;
    // Follow slow idle drift only; never learn a held finger as the baseline.
    if (!on && fabsf(delta) < threshold * 0.25f) {
      noise += 0.025f * (fabsf(delta) - noise);
      baseline += (delta < 0 ? 0.08f : 0.002f) * delta;
    } else if (!on && delta < 0) baseline += 0.08f * delta;
    return on;
  }
  float pressure() const {                         // 0..1, rough finger pressure
    if (!on) return 0;
    return clampf(0.3f + (delta - fmaxf(3200.0f, noise * 8)) / 6000.0f);
  }
};

CapacitiveSensor touch1(hw::TOUCH_SEND, hw::TOUCH_RECV[0]);
CapacitiveSensor touch2(hw::TOUCH_SEND, hw::TOUCH_RECV[1]);
CapacitiveSensor touch3(hw::TOUCH_SEND, hw::TOUCH_RECV[2]);
CapacitiveSensor* touchSensors[3] = {&touch1, &touch2, &touch3};

/* ============================================================================
   6. PARAMETERS  —  12 per mode (6 NS + 6 SH), all 0..1
   ========================================================================== */
enum class Mode : uint8_t { AS, TS };
Mode mode = Mode::AS;

enum AsParam { AS_PITCH, AS_CHORD, AS_ALGO, AS_RATIO, AS_DEPTH, AS_FEEDBACK,          // NS
               AS_LFO_DEPTH, AS_LFO_RATE, AS_DETUNE, AS_PULSE, AS_GRIT, AS_SPACE };     // SH
enum TsParam { TS_PITCH1, TS_PITCH2, TS_PITCH3, TS_DECAY1, TS_DECAY2, TS_DECAY3,        // NS
               TS_ALGO, TS_RATIO, TS_DEPTH, TS_FEEDBACK, TS_CONTOUR, TS_DETUNE };       // SH

const char* const AS_NAMES[12] = {"PITCH","CHORD","ALGO","RATIO","DEPTH","FEEDBACK",
                                  "LFO DEPTH","LFO RATE","DETUNE","PULSE","GRIT","SPACE"};
const char* const TS_NAMES[12] = {"PITCH1","PITCH2","PITCH3","DECAY1","DECAY2","DECAY3",
                                  "ALGO","RATIO","DEPTH","FEEDBACK","CONTOUR","DETUNE"};

// Start-up sound.  AS: low C drone, DX22 (organ-ish pad).  TS: C/E/G, DX5, TINE.
float asParams[12] = {0.20f, 0.45f, 0.80f, 0.22f, 0.38f, 0.18f,
                      0.22f, 0.30f, 0.30f, 0.00f, 0.00f, 0.28f};
float tsParams[12] = {0.3334f, 0.3889f, 0.4306f, 0.50f, 0.50f, 0.50f,
                      0.56f, 0.14f, 0.42f, 0.12f, 0.45f, 0.15f};
uint8_t tsCharacter = 0;

float* params() { return mode == Mode::AS ? asParams : tsParams; }

// Musical mapping of the 0..1 values.
int   algoFrom(float v)  { int i = int(v * fm::kNumAlgorithms); return i < 0 ? 0 : (i >= fm::kNumAlgorithms ? fm::kNumAlgorithms - 1 : i); }
float asRootNote()       { return 24.0f + roundf(asParams[AS_PITCH] * 60.0f); }      // C1..C6
float tsPadNote(int v)   { return 24.0f + roundf(tsParams[TS_PITCH1 + v] * 72.0f); } // C1..C7

// AS CHORD: semitone offsets of voice 2 and voice 3 from the root.
const int8_t kChords[8][2] = {{0, 0}, {0, 12}, {7, 12}, {5, 12}, {3, 7}, {4, 7}, {7, 14}, {-12, 7}};
const char* const kChordNames[8] = {"UNISON","OCTAVE","5TH+OCT","4TH+OCT","MINOR","MAJOR","OPEN 5THS","SUB+5TH"};
int chordFrom(float v) { int i = int(v * 8); return i < 0 ? 0 : (i > 7 ? 7 : i); }
int currentAlgo() { return mode == Mode::AS ? algoFrom(asParams[AS_ALGO]) : algoFrom(tsParams[TS_ALGO]); }

/* ============================================================================
   7. CLOCK, CV AND MIDI
   ========================================================================== */
constexpr uint8_t MIDI_OUT_CH = 16;          // our output; incoming ch 16 is ignored

// Clock priority: CV > MIDI > internal.  CV owns for 20 s after its last
// pulse; MIDI for 30 s; internal clock returns after 30 s of silence.
struct ClockPolicy {
  enum Source { Internal, Midi, Cv, Waiting };
  uint32_t lastCv = 0, lastMidi = 0;
  bool seenCv = false, seenMidi = false;
  void cv(uint32_t now)   { lastCv = now;   seenCv = true; }
  void midi(uint32_t now) { lastMidi = now; seenMidi = true; }
  bool cvOwns(uint32_t now) const { return seenCv && uint32_t(now - lastCv) < 20000; }
  Source source(uint32_t now) const {
    if (cvOwns(now)) return Cv;
    if (seenMidi && uint32_t(now - lastMidi) < 30000) return Midi;
    if (seenCv && uint32_t(now - lastCv) < 30000) return Waiting;
    return Internal;
  }
} clocks;

// CV1 interrupt: only counts edges and stores the time (>1 ms apart).
volatile uint32_t cv1EdgeUs = 0, cv1Sequence = 0;
uint32_t cv1SeenSequence = 0;
uint32_t cv1LastMs = 0;                       // last CV1 edge (ms), for CV2 priority
void cv1ISR() {
  uint32_t now = micros();
  if (uint32_t(now - cv1EdgeUs) > 1000) { cv1EdgeUs = now; ++cv1Sequence; }
}

// CV2: analog.  Pulses act as triggers, steady voltage as a modulation.
float cv2Smooth = 0, cv2Mod = 0;
bool  cv2High = false, cv2Clocking = false;
uint32_t cv2LastEdgeMs = 0, lastCv2ReadMs = 0;

// Outgoing MIDI goes through this queue and is sent only from serviceOut().
struct MidiMsg { uint8_t type, a, b; };
struct Outbox {
  MidiMsg data[64]; uint8_t r = 0, w = 0;
  void push(uint8_t type, uint8_t a = 0, uint8_t b = 0) {
    uint8_t n = (w + 1) & 63;
    if (n == r) {
      // Overflow recovery: discard pending attacks/CC/clock and schedule
      // All Notes Off. Never silently lose a release and leave a host note on.
      r = w = 0;
      data[w++] = {0xB0, 123, 0};
      return;
    }
    data[w] = {type, a, b}; w = n;
  }
  bool pop(MidiMsg& m) { if (r == w) return false; m = data[r]; r = (r + 1) & 63; return true; }
} outbox;

// Notes we send with an automatic Note Off after a short gate.
struct TimedNote { uint8_t note = 0; uint32_t offAt = 0; bool on = false; };
TimedNote timedNotes[6];
uint32_t lastStepNoteMs = 0;
void sendTimedNote(uint8_t note, uint8_t vel, uint32_t gateMs) {
  uint32_t now = millis();
  for (auto& t : timedNotes) if (t.on && t.note == note) { outbox.push(0x80, note, 0); t.on = false; }
  for (auto& t : timedNotes) if (!t.on) {
    t = {note, now + gateMs, true};
    outbox.push(0x90, note, vel);
    return;
  }
}

// MIDI Clock out, converted from CV: 24 pulses spread over the last CV period.
struct ClockOut {
  bool started = false; uint8_t left = 0; uint32_t nextUs = 0, stepUs = 0;
  void cvEdge(uint32_t periodUs) {
    if (periodUs < 200000UL || periodUs > 3000000UL) return;   // 20..300 BPM only
    if (!started) { outbox.push(0xFA); started = true; }         // Start
    outbox.push(0xF8);
    stepUs = periodUs / 24; left = 23; nextUs = micros() + stepUs;
  }
  void service() {
    if (left && int32_t(micros() - nextUs) >= 0) { outbox.push(0xF8); --left; nextUs += stepUs; }
  }
  void stop() { if (started) outbox.push(0xFC); started = false; left = 0; }
} clockOut;

/* ============================================================================
   8. MODE LOGIC — what AS and TS actually do
   ========================================================================== */
bool shiftLayer = false, lastShift = false, bootRequested = false;
DiceButton dice;
TouchGate  touchGate[3];
bool   pads[3] = {};
float  padPressure[3] = {};
PotTakeover pots[6];

// MIDI-side state
bool     transportRunning = true;   // MIDI Stop pauses clock steps
uint8_t  midiTicks = 0;
float    modWheel = 0, cc74 = 0, chanPressure = 0, bendSemis = 0;

// TS voice ownership: who is playing each of the 3 voices.
enum class Owner : uint8_t { None, Pad, Midi, Clock };
struct TsVoice {
  Owner owner = Owner::None;
  uint8_t midiNote = 255, midiCh = 0, outNote = 255;
  uint32_t startedAt = 0, clockOffAt = 0;
} tsVoice[3];

// LED / display state
uint32_t lastActivityMs = 0, modeFlashUntil = 0, infoUntil = 0, lastStepMs = 0;
enum class Info : uint8_t { None, Algorithm, Character } info = Info::None;
uint8_t infoValue = 0;

void armPickup() { for (int i = 0; i < 6; ++i) pots[i].arm(readPot(i)); }

void showInfo(Info what, uint8_t value) { info = what; infoValue = value; infoUntil = millis() + 1200; }

// ---- starting a TS voice from any source ----
void tsStart(int v, Owner who, float note, float vel, uint32_t now) {
  synth.noteOn(v, note, vel);
  tsVoice[v].owner = who;
  tsVoice[v].startedAt = now;
  lastActivityMs = now;
}
void tsStop(int v) {
  synth.noteOff(v);
  if (tsVoice[v].outNote != 255) { outbox.push(0x80, tsVoice[v].outNote, 0); tsVoice[v].outNote = 255; }
  tsVoice[v] = TsVoice();
}
void releaseEverything() {
  for (int v = 0; v < 3; ++v) tsStop(v);
  for (auto& t : timedNotes) if (t.on) { outbox.push(0x80, t.note, 0); t.on = false; }
  synth.releaseAll();
  outbox.push(0xB0, 123, 0);                    // All Notes Off on our channel
}

// ---- a "step": one beat from CV, MIDI clock, MIDI note or internal PULSE ----
enum class StepFrom : uint8_t { Cv, MidiClock, MidiNote, Internal };
void step(StepFrom from, float velocity = 0.8f) {
  uint32_t now = millis();
  lastStepMs = lastActivityMs = now;
  bool sendNote = from != StepFrom::MidiNote && uint32_t(now - lastStepNoteMs) >= 20;   // no echo, max 50/s
  if (mode == Mode::AS) {
    // AS: the drone keeps sounding; a step is a bright FM "re-strike".
    for (int v = 0; v < 3; ++v) synth.burst(v, 0.45f + 0.55f * velocity);
    if (sendNote) { sendTimedNote(uint8_t(asRootNote()), uint8_t(40 + 80 * velocity), 90); lastStepNoteMs = now; }
  } else {
    // TS: strike every voice that is not held by a finger or a MIDI note.
    for (int v = 0; v < 3; ++v) {
      if (tsVoice[v].owner == Owner::Pad || tsVoice[v].owner == Owner::Midi) continue;
      tsStart(v, Owner::Clock, tsPadNote(v), velocity, now);
      tsVoice[v].clockOffAt = now + 160;
      if (sendNote) sendTimedNote(uint8_t(tsPadNote(v)), uint8_t(40 + 80 * velocity), 90);
    }
    if (sendNote) lastStepNoteMs = now;
  }
}

// ---- MIDI note in TS: simple 3-voice allocation, pads have priority ----
void tsMidiNoteOn(uint8_t note, uint8_t vel, uint8_t ch, uint32_t now) {
  int pick = -1;
  for (int v = 0; v < 3; ++v)                                  // same note again
    if (tsVoice[v].owner == Owner::Midi && tsVoice[v].midiNote == note && tsVoice[v].midiCh == ch) pick = v;
  if (pick < 0) for (int v = 0; v < 3; ++v)                    // a free, quiet voice
    if (tsVoice[v].owner == Owner::None && synth.voiceSilent(v)) { pick = v; break; }
  if (pick < 0) for (int v = 0; v < 3; ++v)                    // any free voice
    if (tsVoice[v].owner == Owner::None) { pick = v; break; }
  if (pick < 0) {                                              // steal oldest non-pad
    uint32_t oldest = 0;
    for (int v = 0; v < 3; ++v) {
      if (tsVoice[v].owner == Owner::Pad) continue;
      uint32_t age = now - tsVoice[v].startedAt;
      if (pick < 0 || age > oldest) { pick = v; oldest = age; }
    }
  }
  if (pick < 0) return;                                        // all 3 pads held
  tsStart(pick, Owner::Midi, note, vel / 127.0f, now);
  tsVoice[pick].midiNote = note;
  tsVoice[pick].midiCh = ch;
}
void tsMidiNoteOff(uint8_t note, uint8_t ch) {
  for (int v = 0; v < 3; ++v)
    if (tsVoice[v].owner == Owner::Midi && tsVoice[v].midiNote == note && tsVoice[v].midiCh == ch) {
      synth.noteOff(v); tsVoice[v] = TsVoice();
    }
}

// ---- pads ----
void padPressed(int i, uint32_t now) {
  lastActivityMs = now;
  if (mode == Mode::AS) {
    synth.burst(i, 0.9f);                                       // every touch re-strikes
    sendTimedNote(uint8_t(asRootNote() + (i == 0 ? 0 : kChords[chordFrom(asParams[AS_CHORD])][i - 1])), 100, 120);
  } else {
    if (tsVoice[i].owner == Owner::Midi) tsVoice[i] = TsVoice();    // the finger wins
    tsStart(i, Owner::Pad, tsPadNote(i), 0.85f, now);
    tsVoice[i].outNote = uint8_t(tsPadNote(i));
    outbox.push(0x90, tsVoice[i].outNote, 100);
  }
}
void padReleased(int i) {
  if (mode == Mode::TS && tsVoice[i].owner == Owner::Pad) tsStop(i);
}

// ---- randomize one layer of the current mode (DICE) ----
float randomFarFrom(float current, float lo, float hi, float minDelta = 0.2f) {
  for (int k = 0; k < 20; ++k) {
    float c = lo + random(10001) / 10000.0f * (hi - lo);
    if (fabsf(c - current) >= minDelta || hi - lo < minDelta) return c;
  }
  return clampf(current + (current < (lo + hi) * 0.5f ? minDelta : -minDelta), lo, hi);
}
void randomizeLayer(bool shift) {
  // {low, high} range per parameter.  {-1,-1} = leave this one alone.
  static const float asRange[12][2] = {{.10f,.60f},{0,1},{0,1},{0,.9f},{.15f,.70f},{0,.50f},
                                       {0,.60f},{.10f,.70f},{0,.70f},{-1,-1},{0,.35f},{.10f,.60f}};
  static const float tsRange[12][2] = {{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},
                                       {0,1},{0,.9f},{.15f,.75f},{0,.50f},{.20f,.80f},{0,.50f}};
  const float (*range)[2] = mode == Mode::AS ? asRange : tsRange;
  float* p = params();
  for (int i = shift ? 6 : 0; i < (shift ? 12 : 6); ++i)
    if (range[i][0] >= 0) p[i] = randomFarFrom(p[i], range[i][0], range[i][1]);
  armPickup();
  if (mode == Mode::AS) step(StepFrom::Internal, 0.9f);             // hear the new sound
}

void setMode(Mode m) {
  if (m == mode) return;
  releaseEverything();
  mode = m;
  if (mode == Mode::AS) synth.droneStart();
  modeFlashUntil = millis() + 600;
  armPickup();
}

// ---- build the engine Patch from the parameters (called every loop) ----
fm::Patch buildPatch() {
  fm::Patch p;
  p.bendSemis = bendSemis;
  float expression = clampf(cv2Mod * 0.6f + cc74 * 0.5f + chanPressure * 0.5f);
  p.grit  = asParams[AS_GRIT];                     // GRIT/SPACE/LFO RATE are global
  p.space = asParams[AS_SPACE];
  p.lfoRate = asParams[AS_LFO_RATE];
  if (mode == Mode::AS) {
    p.drone     = true;
    p.algorithm = algoFrom(asParams[AS_ALGO]);
    p.ratio     = asParams[AS_RATIO];
    p.depth     = asParams[AS_DEPTH];
    p.feedback  = asParams[AS_FEEDBACK];
    p.detune    = asParams[AS_DETUNE];
    p.lfoDepth  = clampf(asParams[AS_LFO_DEPTH] + modWheel * 0.6f);
    p.bright    = clampf(padPressure[0] + expression);
    p.metal     = padPressure[1];
    p.growl     = padPressure[2];
    float root = asRootNote();
    const int8_t* c = kChords[chordFrom(asParams[AS_CHORD])];
    p.targetNote[0] = root; p.targetNote[1] = root + c[0]; p.targetNote[2] = root + c[1];
    for (int v = 0; v < 3; ++v) p.retune[v] = true;
  } else {
    p.drone     = false;
    p.character = tsCharacter;
    p.algorithm = algoFrom(tsParams[TS_ALGO]);
    p.ratio     = tsParams[TS_RATIO];
    p.depth     = tsParams[TS_DEPTH];
    p.feedback  = tsParams[TS_FEEDBACK];
    p.contour   = tsParams[TS_CONTOUR];
    p.detune    = tsParams[TS_DETUNE];
    p.lfoDepth  = modWheel * 0.8f;
    p.bright    = expression;
    for (int v = 0; v < 3; ++v) {
      p.voiceDecay[v] = tsParams[TS_DECAY1 + v];
      p.targetNote[v] = tsPadNote(v);
      p.retune[v] = tsVoice[v].owner == Owner::Pad;   // a held pad follows its PITCH pot
    }
  }
  return p;
}

/* ============================================================================
   9. SERVICE ROUTINES (called from loop)
   ========================================================================== */
void enterBootloader();

void serviceDice() {
  uint32_t now = millis();
  DiceButton::Event e = dice.update(digitalRead(hw::PIN_DICE) == LOW, now, digitalRead(hw::PIN_SHIFT) == HIGH);
  if (e == DiceButton::Long) {
    setMode(mode == Mode::AS ? Mode::TS : Mode::AS);
  } else if (e == DiceButton::Short) {
    if (dice.gestureShift) randomizeLayer(true);
    else if (mode == Mode::AS) randomizeLayer(false);
    else { tsCharacter = (tsCharacter + 1) % fm::kNumCharacters; showInfo(Info::Character, tsCharacter); armPickup(); }
  } else if (e == DiceButton::Boot) {
    bootRequested = true;
  }
}

void serviceCv1() {
  noInterrupts(); uint32_t seq = cv1Sequence, edgeUs = cv1EdgeUs; interrupts();
  if (seq == cv1SeenSequence) return;
  static uint32_t lastEdgeUs = 0;
  uint32_t period = edgeUs - lastEdgeUs; lastEdgeUs = edgeUs;
  cv1SeenSequence = seq;
  uint32_t edgeMs = millis() - (micros() - edgeUs) / 1000;   // physical edge time
  clocks.cv(edgeMs);
  cv1LastMs = edgeMs;
  midiTicks = 0;
  clockOut.cvEdge(period);
  step(StepFrom::Cv);
}

void serviceCv2(uint32_t now) {
  if (now == lastCv2ReadMs) return;               // 1 kHz is plenty
  lastCv2ReadMs = now;
  float raw = analogRead(hw::PIN_CV2) / 1023.0f;
  cv2Smooth += 0.035f * (raw - cv2Smooth);
  bool high = cv2High ? raw > 0.018f : raw > 0.030f;   // hysteresis (RunOS values)
  if (high && !cv2High) {
    uint32_t gap = now - cv2LastEdgeMs;
    // Ignore edges closer than 50 ms (ADC noise must not latch it).
    if (gap > 50) {
      if (gap < 3000) cv2Clocking = true;
      // If CV1 is clocking (edge in the last 20 s), CV1 wins and CV2 is ignored.
      bool cv1Owns = cv1SeenSequence != 0 && uint32_t(now - cv1LastMs) < 20000;
      if (!cv1Owns) {
        clocks.cv(now); midiTicks = 0;
        clockOut.cvEdge(gap * 1000UL);
        step(StepFrom::Cv);
      }
    }
    cv2LastEdgeMs = now;
  }
  cv2High = high;
  if (cv2Clocking && uint32_t(now - cv2LastEdgeMs) > 20000) cv2Clocking = false;
  cv2Mod = cv2Clocking ? 0 : cv2Smooth;           // steady CV2 -> extra FM depth
}

// Reads every waiting MIDI message.  ONLY changes state and queues output.
// Never sends from here (the Teensy has 6 RX buffers; a blocked send while
// a DAW floods us can lock MIDI up).
void receiveMidi() {
  while (usbMIDI.read()) {
    serviceDice();
    if (bootRequested) return;
    serviceCv1();                                  // CV may arrive mid-burst
    uint8_t type = usbMIDI.getType(), a = usbMIDI.getData1(), b = usbMIDI.getData2(), ch = usbMIDI.getChannel();
    uint32_t now = millis();
    bool channelMsg = type < 0xF0;
    if (channelMsg && (ch < 1 || ch == MIDI_OUT_CH)) continue;      // our own echo

    if (type == usbMIDI.NoteOn && b > 0) {
      transportRunning = true;
      if (mode == Mode::AS) {
        int n = a; while (n < 24) n += 12; while (n > 84) n -= 12;   // fold into C1..C6
        asParams[AS_PITCH] = (n - 24) / 60.0f;
        if (!shiftLayer) pots[0].arm(readPot(0));
        step(StepFrom::MidiNote, b / 127.0f);                       // audible event
      } else {
        tsMidiNoteOn(a, b, ch, now);
      }
    } else if (type == usbMIDI.NoteOff || (type == usbMIDI.NoteOn && b == 0)) {
      if (mode == Mode::TS) tsMidiNoteOff(a, ch);
    } else if (type == usbMIDI.Clock) {
      clocks.midi(now);
      if (clocks.cvOwns(now)) { midiTicks = 0; continue; }          // CV has priority
      if (transportRunning && ++midiTicks >= 24) { midiTicks = 0; step(StepFrom::MidiClock); }
    } else if (type == usbMIDI.Start || type == usbMIDI.Continue) {
      transportRunning = true;
      if (type == usbMIDI.Start) midiTicks = 0;
    } else if (type == usbMIDI.Stop || type == usbMIDI.SystemReset) {
      transportRunning = false; midiTicks = 0;
      if (mode == Mode::TS) for (int v = 0; v < 3; ++v) if (tsVoice[v].owner != Owner::Pad) tsStop(v);
    } else if (type == usbMIDI.ControlChange) {
      if (a >= 20 && a <= 31) {
        int k = a - 20;
        params()[k] = b / 127.0f;
        if ((k >= 6) == shiftLayer) pots[k % 6].arm(readPot(k % 6));   // pot re-arms
      } else if (a == 1)  modWheel = b / 127.0f;
      else if (a == 74)   cc74 = b / 127.0f;
      else if (a == 120 || a == 123) { if (mode == Mode::TS) releaseEverything(); }
    } else if (type == usbMIDI.PitchBend) {
      int value = (int(a) | (int(b) << 7)) - 8192;
      bendSemis = value / 8192.0f * 2.0f;
    } else if (type == usbMIDI.AfterTouchChannel) {
      chanPressure = a / 127.0f;
    }
  }
}

// Pots and layer LEDs, every 5 ms.
uint8_t lastCcSent[12];
void servicePanel(float dt) {
  uint32_t now = millis();
  shiftLayer = digitalRead(hw::PIN_SHIFT) == HIGH;
  if (shiftLayer != lastShift) { armPickup(); lastShift = shiftLayer; }
  int algoBefore = currentAlgo();
  if (!dice.down && !dice.raw) {                   // pots freeze while DICE is held
    float* p = params();
    for (int i = 0; i < 6; ++i) {
      int k = i + (shiftLayer ? 6 : 0);
      if (pots[i].update(readPot(i), p[k], dt)) {
        uint8_t cc = uint8_t(p[k] * 127.0f + 0.5f);
        static uint32_t lastCcMs[12] = {};
        if (abs(int(cc) - int(lastCcSent[k])) >= 2 && uint32_t(now - lastCcMs[k]) >= 20) {
          outbox.push(0xB0, uint8_t(20 + k), cc);  // pot -> MIDI CC out
          lastCcSent[k] = cc; lastCcMs[k] = now;
        }
      }
    }
  }
  int algoAfter = currentAlgo();
  if (algoAfter != algoBefore) showInfo(Info::Algorithm, uint8_t(algoAfter));
}

// Touch pads, every 12 ms.  MIDI and DICE are serviced between the three
// (slow) capacitive measurements so nothing waits too long.
void serviceTouch() {
  for (int i = 0; i < 3; ++i) {
    receiveMidi();
    serviceDice();
    if (bootRequested) return;
    long value = touchSensors[i]->capacitiveSensorRaw(4);
    long normalized = value < 0 ? value : value * 5 / 2;    // same scale as 10-sample reads
    bool before = pads[i];
    pads[i] = touchGate[i].update(normalized, millis());
    padPressure[i] += 0.3f * (touchGate[i].pressure() - padPressure[i]);
    if (pads[i] && !before) padPressed(i, millis());
    if (!pads[i] && before) padReleased(i);
  }
}

// TS housekeeping: clock-struck voices get released, stale MIDI notes expire.
void serviceVoices(uint32_t now) {
  if (mode != Mode::TS) return;
  for (int v = 0; v < 3; ++v) {
    if (tsVoice[v].owner == Owner::Clock && int32_t(now - tsVoice[v].clockOffAt) >= 0) { synth.noteOff(v); tsVoice[v] = TsVoice(); }
    if (tsVoice[v].owner == Owner::Midi && uint32_t(now - tsVoice[v].startedAt) > 30000) { synth.noteOff(v); tsVoice[v] = TsVoice(); }
  }
}

// Internal PULSE (AS only) when no external clock is active.
void serviceInternalClock(uint32_t now) {
  static ClockPolicy::Source previous = ClockPolicy::Internal;
  ClockPolicy::Source source = clocks.source(now);
  if (source != previous) {
    midiTicks = 0;
    if (previous == ClockPolicy::Cv) clockOut.stop();
    previous = source;
  }
  if (mode != Mode::AS || source != ClockPolicy::Internal) return;
  float pulse = asParams[AS_PULSE];
  if (pulse < 0.03f) return;                                   // pure drone
  uint32_t periodMs = uint32_t(4000.0f * powf(0.03f, (pulse - 0.03f) / 0.97f));   // 4 s .. 120 ms
  if (uint32_t(now - lastStepMs) >= periodMs) step(StepFrom::Internal, 0.6f);
}

// Sends queued MIDI.  The only place that calls usbMIDI.send*.
uint32_t lastCv2CcMs = 0; uint8_t lastCv2Cc = 255;
void serviceOut(uint32_t now) {
  for (auto& t : timedNotes) if (t.on && int32_t(now - t.offAt) >= 0) { outbox.push(0x80, t.note, 0); t.on = false; }
  clockOut.service();
  if (uint32_t(now - lastCv2CcMs) >= 30) {                     // CV2 -> CC74, rate limited
    lastCv2CcMs = now;
    uint8_t v = uint8_t(cv2Smooth * 127);
    if (lastCv2Cc == 255 || abs(int(v) - int(lastCv2Cc)) >= 2) { outbox.push(0xB0, 74, v); lastCv2Cc = v; }
  }
  MidiMsg m; bool sent = false;
  for (int i = 0; i < 6 && outbox.pop(m); ++i) {
    switch (m.type) {
      case 0x90: usbMIDI.sendNoteOn(m.a, m.b, MIDI_OUT_CH); break;
      case 0x80: usbMIDI.sendNoteOff(m.a, 0, MIDI_OUT_CH); break;
      case 0xB0: usbMIDI.sendControlChange(m.a, m.b, MIDI_OUT_CH); break;
      case 0xF8: usbMIDI.sendRealTime(usbMIDI.Clock); break;
      case 0xFA: usbMIDI.sendRealTime(usbMIDI.Start); break;
      case 0xFC: usbMIDI.sendRealTime(usbMIDI.Stop); break;
      default: break;
    }
    sent = true;
  }
  if (sent) usbMIDI.send_now();
}

// LEDs, every 20 ms.
void serviceLeds(uint32_t now) {
  // Layer LEDs: the deliberate LEFT inversion, exactly once (see box at top).
  digitalWrite(hw::LED_NS, !shiftLayer);
  digitalWrite(hw::LED_SH, shiftLayer);

  int pwm[4] = {0, 0, 0, 0};
  float boot = dice.bootProgress(now);
  if (boot > 0) {                                  // bootloader countdown
    if (boot > 0.89f) { int on = (now / 60) % 2 ? 255 : 0; for (int& x : pwm) x = on; }
    else for (int i = 0; i < 4; ++i) pwm[i] = boot * 4.5f > i + 0.5f ? 255 : 0;
  } else if (int32_t(modeFlashUntil - now) > 0) {  // mode change
    int on = mode == Mode::TS ? ((now / 100) % 2 ? 255 : 0) : 160;
    for (int& x : pwm) x = on;
  } else if (info != Info::None && int32_t(infoUntil - now) > 0) {
    if (info == Info::Algorithm) {                 // binary on LEDs 2..4 (LED 2 = 4)
      pwm[0] = 40;
      for (int b = 0; b < 3; ++b) pwm[1 + b] = (infoValue >> (2 - b)) & 1 ? 255 : 0;
    } else {                                       // character, one-hot
      for (int i = 0; i < 4; ++i) pwm[i] = i == infoValue ? 255 : 0;
    }
  } else {
    float ml, mr; synth.meters(ml, mr);
    pwm[0] = uint32_t(now - lastActivityMs) < 60 ? 255 : 0;
    pwm[1] = int(clampf(cv2Smooth * 1.5f) * 255);
    pwm[2] = int(clampf(ml * 4.0f) * 255);
    pwm[3] = int(clampf(mr * 4.0f) * 255);
  }
  for (int i = 0; i < 4; ++i) analogWrite(hw::LEDS[i], pwm[i]);
}

// Status line on Serial every 5 s (or on '?').  Never blocks: if the host is
// not reading, the line is skipped.  Also accepts the BOOTLOADER command.
void diagnostics(uint32_t now) {
  static char line[16]; static uint8_t len = 0;
  static uint32_t lastMs = 0;
  bool requested = false;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '?') requested = true;
    if (c == '\n' || c == '\r') {
      line[len] = 0;
      if (strcmp(line, "BOOTLOADER") == 0) bootRequested = true;
      len = 0;
    } else if (len < sizeof(line) - 1) line[len++] = c;
  }
  if (!requested && uint32_t(now - lastMs) < 5000) return;
  lastMs = now;
  char text[200];
  int n = snprintf(text, sizeof(text), "%s %s %s algo=%s char=%s CPU=%.1f%% max=%.1f%% mem=%u clk=%d\n",
                   OS_NAME, OS_VERSION, mode == Mode::AS ? "AS" : "TS",
                   fm::kAlgorithms[currentAlgo()].name,
                   fm::kCharacters[tsCharacter].name,
                   AudioProcessorUsage(), AudioProcessorUsageMax(), unsigned(AudioMemoryUsageMax()),
                   int(clocks.source(now)));
  if (n > 0 && n < int(sizeof(text)) && Serial.availableForWrite() >= n) Serial.write((const uint8_t*)text, n);
}

/* ============================================================================
   10. BOOTLOADER  —  keep this section in every OS (see the box at the top)
   ========================================================================== */

// Runs first thing in setup(), before audio or USB work, so it still works
// when the rest of the OS is broken.  SH + hold DICE while powering up, 3 s.
void bootRescueAtPowerUp() {
  delay(5);
  if (!(digitalRead(hw::PIN_DICE) == LOW && digitalRead(hw::PIN_SHIFT) == HIGH)) return;
  uint32_t t0 = millis();
  while (digitalRead(hw::PIN_DICE) == LOW && digitalRead(hw::PIN_SHIFT) == HIGH) {
    bool on = (millis() / 100) % 2;
    for (uint8_t pin : hw::LEDS) digitalWrite(pin, on);
    if (millis() - t0 >= 3000) {
      for (uint8_t pin : hw::LEDS) digitalWrite(pin, HIGH);
      delay(150);
      _reboot_Teensyduino_();                      // jumps to the bootloader, never returns
    }
  }
  for (uint8_t pin : hw::LEDS) digitalWrite(pin, LOW);   // released early: normal start
}

// Called from loop() only (never from MIDI RX or the CV interrupt).
void enterBootloader() {
  synth.panic();
  for (auto& t : timedNotes) if (t.on) usbMIDI.sendNoteOff(t.note, 0, MIDI_OUT_CH);
  for (auto& v : tsVoice) if (v.outNote != 255) usbMIDI.sendNoteOff(v.outNote, 0, MIDI_OUT_CH);
  usbMIDI.sendControlChange(123, 0, MIDI_OUT_CH);
  usbMIDI.send_now();
  for (uint8_t pin : hw::LEDS) analogWrite(pin, 255);
  delay(30);                                       // let the fade-out and USB finish
  AudioNoInterrupts();
  _reboot_Teensyduino_();
}

/* ============================================================================
   11. SETUP AND LOOP
   ========================================================================== */
void setup() {
  pinMode(hw::PIN_SHIFT, INPUT_PULLUP);
  pinMode(hw::PIN_DICE, INPUT_PULLUP);
  pinMode(hw::LED_NS, OUTPUT); pinMode(hw::LED_SH, OUTPUT);
  for (uint8_t pin : hw::LEDS) pinMode(pin, OUTPUT);
  bootRescueAtPowerUp();                           // <- keep this first

  Serial.begin(115200);
  AudioMemory(16);
  analogReadResolution(10);
  analogReadAveraging(4);
  analogWriteResolution(8);
  pinMode(hw::PIN_CV1, INPUT);                     // plain INPUT (pulldown can kill weak CV)
  pinMode(hw::PIN_CV2, INPUT);
  for (CapacitiveSensor* s : touchSensors) { s->set_CS_AutocaL_Millis(0xFFFFFFFF); s->set_CS_Timeout_Millis(5); }
  attachInterrupt(digitalPinToInterrupt(hw::PIN_CV1), cv1ISR, RISING);

  randomSeed(analogRead(hw::PIN_CV2) + micros());
  cv2Smooth = analogRead(hw::PIN_CV2) / 1023.0f;
  cv2High = cv2Smooth > 0.030f;                    // a cable plugged in high is not an edge
  shiftLayer = lastShift = digitalRead(hw::PIN_SHIFT) == HIGH;
  for (int k = 0; k < 12; ++k) lastCcSent[k] = uint8_t(params()[k] * 127);
  armPickup();                                     // patch values hold until pots move 8 %
  synth.setPatch(buildPatch());
  synth.droneStart();                              // start in AS
  modeFlashUntil = millis() + 600;
}

void loop() {
  serviceDice();
  if (bootRequested) { enterBootloader(); return; }
  serviceCv1();
  receiveMidi();
  if (bootRequested) { enterBootloader(); return; }

  uint32_t now = millis();
  static uint32_t lastPanelMs = 0, lastTouchMs = 0, lastLedMs = 0;
  if (uint32_t(now - lastPanelMs) >= 5) { servicePanel((now - lastPanelMs) * 0.001f); lastPanelMs = now; }
  serviceCv2(now);
  if (uint32_t(now - lastTouchMs) >= 12) { lastTouchMs = now; serviceTouch(); }
  if (bootRequested) { enterBootloader(); return; }

  now = millis();
  serviceCv1();
  serviceInternalClock(now);
  serviceVoices(now);
  static uint32_t lastPatchMs = 0;
  if (now != lastPatchMs) { lastPatchMs = now; synth.setPatch(buildPatch()); }   // 1 kHz is plenty
  serviceOut(now);
  if (uint32_t(now - lastLedMs) >= 20) { lastLedMs = now; serviceLeds(now); }
  diagnostics(now);
}

#endif  // SIXOP_NATIVE_TEST
