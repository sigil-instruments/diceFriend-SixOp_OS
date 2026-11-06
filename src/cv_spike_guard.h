#pragma once
// =============================================================================
// diceFriend CV1 spike guard v1 (2026-11-06)
//
// Problem: an unpatched CV1 input can pick up single noise spikes (audio cable,
// USB supply, touching the case). In every OS that follows the CV -> MIDI ->
// internal clock priority, ONE such edge handed transport to CV for 20 s and
// silenced a running MIDI clock. Several spikes = 1-2 minute stalls, then a
// burst of queued steps (confirmed on hardware in Drzazga_OS).
//
// Rule: an edge counts only if
//   1. the pulse is real: CV1 is still HIGH 50 us after the rising edge, and
//   2. CV is already a confirmed clock (an accepted edge within the last 20 s),
//      or the previous edge came 1 ms .. 10 s earlier (two-edge confirmation).
// Rejected edges are ignored completely: no step, no CV ownership, no tempo
// measurement, no MIDI lock. Cost: the first pulse after silence does not
// play, and triggers spaced more than 10 s apart are ignored.
//
// Usage, as the FIRST statement of the CV1 RISING interrupt:
//     if (!cvguard::accept(PIN_CV1)) return;
// digitalRead() is safe on the interrupt pin. Never call analogRead() on CV1:
// on Teensy 4.0 it reconfigures the pin and silently disables the interrupt.
// The 50 us busy-wait runs inside the ISR, once per edge.
// =============================================================================
#include <Arduino.h>

namespace cvguard {
constexpr uint32_t kMinWidthUs = 50;           // shorter HIGH = noise spike
constexpr uint32_t kMinIntervalUs = 1000;      // same as the usual 1 ms CV1 debounce
constexpr uint32_t kMaxIntervalUs = 10000000;  // 10 s: slow step clocks still confirm
constexpr uint32_t kHoldUs = 20000000;         // CV ownership window (CV -> MIDI lock)

inline bool accept(uint8_t pin) {
  static uint32_t lastRawUs = 0, lastAcceptedUs = 0;
  static bool haveRaw = false, haveAccepted = false;
  const uint32_t t0 = micros();
  while (uint32_t(micros() - t0) < kMinWidthUs) {
  }
  if (digitalRead(pin) == LOW) return false;  // too short: not even remembered
  const bool owning = haveAccepted && uint32_t(t0 - lastAcceptedUs) < kHoldUs;
  const uint32_t dt = t0 - lastRawUs;
  const bool ok = owning || (haveRaw && dt >= kMinIntervalUs && dt <= kMaxIntervalUs);
  lastRawUs = t0;
  haveRaw = true;
  if (ok) {
    lastAcceptedUs = t0;
    haveAccepted = true;
  }
  return ok;
}
}  // namespace cvguard
