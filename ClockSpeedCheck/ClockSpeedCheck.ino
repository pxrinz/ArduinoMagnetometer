/*
  ClockSpeedCheck.ino

  Reports the Arduino's compiled/nominal clock speed (F_CPU) and runs a
  timed benchmark loop to estimate the *effective* speed the sketch is
  actually running at. Comparing the two helps catch cases where the
  selected board/clock setting in the IDE doesn't match the chip's real
  clock source (e.g. a wrong fuse setting such as CKDIV8 silently
  dividing the clock by 8, or a bootloader built for the wrong crystal).

  Note: micros()/millis() are themselves derived from F_CPU, so this is
  not an absolute, independent frequency measurement - it's a relative
  benchmark. A large mismatch (e.g. off by a clean factor of 2, 4, or 8)
  is a strong signal of a clock/fuse misconfiguration.
*/

const unsigned long BENCHMARK_MS = 1000UL;   // benchmark window length
volatile unsigned long opCount = 0;          // counts loop iterations

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for native USB boards (e.g. Leonardo/Micro) to enumerate
  }

  Serial.println(F("=== Arduino Clock Speed Check ==="));

  Serial.print(F("Compiled F_CPU: "));
  Serial.print(F_CPU / 1000000.0, 3);
  Serial.println(F(" MHz"));

  Serial.println(F("Running benchmark..."));
  unsigned long ops = runBenchmark(BENCHMARK_MS);

  Serial.print(F("Iterations in "));
  Serial.print(BENCHMARK_MS);
  Serial.print(F(" ms: "));
  Serial.println(ops);

  Serial.print(F("Effective rate: "));
  Serial.print((ops * 1000.0) / BENCHMARK_MS, 1);
  Serial.println(F(" iterations/sec"));

  Serial.println(F("Tip: run this on a known-good board at the same"));
  Serial.println(F("clock speed to get a baseline iteration count, then"));
  Serial.println(F("compare. A ~2x/4x/8x mismatch usually means a wrong"));
  Serial.println(F("clock/fuse setting rather than normal variance."));
}

void loop() {
  // nothing to do; setup() already reported the results once
}

// Busy loop for durationMs milliseconds, counting how many iterations
// complete. Returns the iteration count.
unsigned long runBenchmark(unsigned long durationMs) {
  opCount = 0;
  unsigned long start = millis();
  while (millis() - start < durationMs) {
    __asm__ __volatile__("nop");
    opCount++;
  }
  return opCount;
}
