/*
  SPI_ClockProbe.ino

  Continuously reads the WHO_AM_I register from an LIS3MDL magnetometer
  over hardware SPI at a fixed ~1 MHz SCK rate. The point of this sketch
  isn't the register value itself - it's to put a real, repeating SPI
  transaction on the wires so SCK (and MOSI/MISO/CS) can be probed with
  an oscilloscope while swapping in different wire lengths between the
  Arduino and the sensor.

  Wiring (hardware SPI, e.g. Uno/Nano):
    SCK  -> pin 13
    MOSI -> pin 11
    MISO -> pin 12
    CS   -> pin 10 (LIS3MDL_CS_PIN below)
    GND  -> GND (keep a matched-length ground return next to whichever
                 signal wire you're stretching, otherwise you're mostly
                 measuring loop-inductance noise, not SPI degradation)

  What to look for on the scope as wire length increases:
    - SCK rise/fall time slowing down (RC effect of wire capacitance)
    - Ringing / overshoot on edges (wire inductance, impedance mismatch)
    - MISO edges lagging or eye-closing relative to SCK
    - WHO_AM_I mismatches printed below = actual bit errors, not just a
      "looks messy on the scope" symptom

  Trigger tip: trigger the scope on the CS falling edge (pin 10) - it
  marks the start of each clean transaction.

  If WHO_AM_I never reads 0x3D even with a short wire, try SPI_MODE0
  instead of SPI_MODE3 below - the LIS3MDL supports both, but some
  breakout boards behave better with one or the other.
*/

#include <SPI.h>

const uint8_t LIS3MDL_CS_PIN = 10;

const uint8_t WHO_AM_I_REG = 0x0F;
const uint8_t WHO_AM_I_EXPECTED = 0x3D;

const uint32_t SPI_CLOCK_HZ = 1000000UL;  // 1 MHz SCK - the signal to probe
SPISettings lis3mdlSPISettings(SPI_CLOCK_HZ, MSBFIRST, SPI_MODE3);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  pinMode(LIS3MDL_CS_PIN, OUTPUT);
  digitalWrite(LIS3MDL_CS_PIN, HIGH);  // CS idles high between transactions

  SPI.begin();

  Serial.println(F("=== LIS3MDL SPI clock probe ==="));
  Serial.print(F("SCK frequency: "));
  Serial.print(SPI_CLOCK_HZ / 1000000.0, 2);
  Serial.println(F(" MHz"));
  Serial.println(F("Probe: SCK=52  MOSI=51  MISO=50  CS=10"));
  Serial.println(F("Reading WHO_AM_I continuously (expect 0x3D)..."));
  Serial.println();
}

void loop() {
  uint8_t whoAmI = readRegister(WHO_AM_I_REG);

  Serial.print(F("WHO_AM_I = 0x"));
  if (whoAmI < 0x10) {
    Serial.print('0');
  }
  Serial.print(whoAmI, HEX);
  Serial.println(whoAmI == WHO_AM_I_EXPECTED ? F("  OK") : F("  MISMATCH"));

  delay(50);  // steady, scope-friendly repetition rate; readable in Serial Monitor too
}

uint8_t readRegister(uint8_t reg) {
  uint8_t value;

  SPI.beginTransaction(lis3mdlSPISettings);
  digitalWrite(LIS3MDL_CS_PIN, LOW);

  SPI.transfer(reg | 0x80);   // MSB=1 selects a read on the LIS3MDL
  value = SPI.transfer(0x00); // dummy byte, clocks the response out on MISO

  digitalWrite(LIS3MDL_CS_PIN, HIGH);
  SPI.endTransaction();

  return value;
}
