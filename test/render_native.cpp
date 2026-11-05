// Renders the SixOp_OS sound engine on a computer, no Teensy needed.
//   g++ -O2 -std=c++17 test/render_native.cpp -o render && ./render
// Writes sixop_preview.wav (AS drone, then TS notes) and prints peak/RMS.
#define SIXOP_NATIVE_TEST
#include "../src/main.cpp"
#include <cstdio>
#include <vector>

static fm::Engine engine;     // static: the engine holds ~70 KB of buffers
int main() {
  std::vector<int16_t> pcm;
  float peak = 0; double rms = 0; long n = 0; bool nan = false;
  auto run = [&](float seconds) {
    int blocks = int(seconds * fm::kSampleRate / fm::kBlock);
    for (int b = 0; b < blocks; ++b) {
      engine.prepare();
      for (int i = 0; i < fm::kBlock; ++i) {
        float l, r; engine.render(l, r);
        if (!(l == l) || !(r == r)) nan = true;
        peak = fmaxf(peak, fmaxf(fabsf(l), fabsf(r))); rms += l * l + r * r; n += 2;
        pcm.push_back(int16_t(l * 32767)); pcm.push_back(int16_t(r * 32767));
      }
    }
  };
  fm::Patch p;                                  // AS drone, defaults like the OS
  p.drone = true; p.algorithm = 6; p.ratio = .22f; p.depth = .38f; p.feedback = .18f;
  p.detune = .3f; p.lfoDepth = .22f; p.lfoRate = .3f; p.space = .28f;
  p.targetNote[0] = 36; p.targetNote[1] = 43; p.targetNote[2] = 48;
  engine.setPatch(p); engine.droneStart(); run(4);
  for (int k = 0; k < 4; ++k) { for (int v = 0; v < 3; ++v) engine.burst(v, .9f); run(1); }   // steps
  for (int a = 0; a < fm::kNumAlgorithms; ++a) { p.algorithm = a; p.ratio = a / 7.0f; engine.setPatch(p); run(0.6f); }
  p.feedback = 1; p.depth = 1; p.metal = 1; p.growl = 1; p.grit = .6f; engine.setPatch(p); run(2);   // worst case
  engine.releaseAll(); run(1.5f);
  // TS: characters and a chord
  fm::Patch t; t.drone = false; t.algorithm = 4; t.ratio = .14f; t.depth = .42f; t.space = .2f;
  for (int c = 0; c < fm::kNumCharacters; ++c) {
    t.character = c; engine.setPatch(t);
    engine.noteOn(0, 48, .9f); run(.25f); engine.noteOn(1, 52, .7f); run(.25f); engine.noteOn(2, 55, .6f); run(.8f);
    engine.releaseAll(); run(1.2f);
  }
  FILE* f = fopen("sixop_preview.wav", "wb");
  uint32_t bytes = pcm.size() * 2, sr = 44118, br = sr * 4;
  fwrite("RIFF", 1, 4, f); uint32_t x = 36 + bytes; fwrite(&x, 4, 1, f); fwrite("WAVEfmt ", 1, 8, f);
  x = 16; fwrite(&x, 4, 1, f); uint16_t fmt[2] = {1, 2}; fwrite(fmt, 2, 2, f); fwrite(&sr, 4, 1, f); fwrite(&br, 4, 1, f);
  uint16_t al[2] = {4, 16}; fwrite(al, 2, 2, f); fwrite("data", 1, 4, f); fwrite(&bytes, 4, 1, f);
  fwrite(pcm.data(), 2, pcm.size(), f); fclose(f);
  printf("samples=%ld peak=%.3f rms=%.3f nan=%d\n", n / 2, peak, sqrt(rms / n), nan);
  return nan ? 1 : 0;
}
