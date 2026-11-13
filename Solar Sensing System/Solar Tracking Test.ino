
const byte SENSOR_COUNT = 4;

const byte SENSOR_PINS[SENSOR_COUNT] = {
  A0, A1, A2, A3
};

const float ADC_REFERENCE_V = 5.0;
const float DIODE_SUPPLY_V = 5.0;
const float RESISTOR_OHMS = 100e3;
const float SENSITIVITY_A_PER_LUX = 10.0e-9;

const int SAMPLES = 64;
const int CALIBRATION_READINGS = 8;

// Ambient light baseline for each sensor
float ambientLux[SENSOR_COUNT];

// Corrected readings
float sensorLux[SENSOR_COUNT];

// Sensor orientations
float sensorAngles[SENSOR_COUNT];


// ============================================================
// READ SENSOR AND CONVERT TO LUX
// ============================================================

float readLux(byte sensorIndex) {

  unsigned long adcSum = 0;

  for (int i = 0; i < SAMPLES; i++) {
    adcSum += analogRead(SENSOR_PINS[sensorIndex]);
    delay(2);
  }

  float adc = adcSum / (float)SAMPLES;

  float resistorV =
    adc * ADC_REFERENCE_V / 1023.0;

  float diodeV = DIODE_SUPPLY_V - resistorV;

  if (adc >= 1020.0 || diodeV <= 0.0) {
    return -1.0;
  }

  float currentA = resistorV / RESISTOR_OHMS;

  float lux =
    currentA / SENSITIVITY_A_PER_LUX;

  return lux;
}


// ============================================================
// CALIBRATE AMBIENT LIGHT
// ============================================================

void calibrateAmbient() {

  Serial.println();
  Serial.println(F("Starting ambient light calibration..."));
  Serial.println(F("Keep tracking lamp OFF or pointed away."));
  Serial.println(F("Room lights can remain ON."));
  Serial.println();

  for (int sensor = 0; sensor < SENSOR_COUNT; sensor++) {

    float luxSum = 0.0;
    int validReadings = 0;

    for (int n = 0; n < CALIBRATION_READINGS; n++) {

      float reading = readLux(sensor);

      if (reading >= 0.0) {
        luxSum += reading;
        validReadings++;
      }
    }

    if (validReadings > 0) {
      ambientLux[sensor] = luxSum / validReadings;
    } else {
      ambientLux[sensor] = 0.0;
    }

    Serial.print(F("Sensor "));
    Serial.print(sensor);
    Serial.print(F(" ambient baseline: "));
    Serial.print(ambientLux[sensor], 2);
    Serial.println(F(" lux"));
  }

  Serial.println();
  Serial.println(F("Calibration complete."));
  Serial.println(F("Beginning sun tracking."));
  Serial.println();
}


// ============================================================
// CALCULATE SUN ANGLE
// ============================================================

float calculateSunAngle() {

  float weightedSum = 0.0;
  float totalLux = 0.0;

  for (int i = 0; i < SENSOR_COUNT; i++) {

    float measuredLux = readLux(i);

    if (measuredLux < 0.0) {
      sensorLux[i] = -1.0;
      continue;
    }

    // Subtract ambient illumination
    sensorLux[i] = measuredLux - ambientLux[i];

    // Ignore negative values
    if (sensorLux[i] < 0.0) {
      sensorLux[i] = 0.0;
    }

    weightedSum +=
      sensorAngles[i] * sensorLux[i];

    totalLux += sensorLux[i];
  }

  if (totalLux <= 0.0) {
    return NAN;
  }

  return weightedSum / totalLux;
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(9600);
  analogReference(DEFAULT);

  for (int i = 0; i < SENSOR_COUNT; i++) {

    sensorAngles[i] =
      i * 180.0 / (SENSOR_COUNT - 1);

    analogRead(SENSOR_PINS[i]);
  }

  delay(10);

  Serial.println(F("BPX65 Sun Tracking Array"));

  calibrateAmbient();
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop() {

  float sunAngle = calculateSunAngle();

  for (int i = 0; i < SENSOR_COUNT; i++) {

    Serial.print(F("S"));
    Serial.print(i);
    Serial.print(F("="));

    if (sensorLux[i] < 0.0) {
      Serial.print(F("SATURATED"));
    } else {
      Serial.print(sensorLux[i], 2);
    }

    Serial.print(F(" lux"));

    if (i < SENSOR_COUNT - 1) {
      Serial.print(F(" | "));
    }
  }

  Serial.println();

  Serial.print(F("Estimated sun angle: "));

  if (isnan(sunAngle)) {
    Serial.println(F("NO VALID LIGHT"));
  } else {
    Serial.print(sunAngle, 2);
    Serial.println(F(" degrees"));
  }

  Serial.println();

  delay(500);
}
