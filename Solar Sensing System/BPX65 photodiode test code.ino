
// Arduino Uno R3, four BPX65 photodiodes.
// Sensors connected to A0, A1, A2 and A3.

const byte SENSOR_PINS[4] = {A0, A1, A2, A3};

const float ADC_REFERENCE_V = 5.0;
const float DIODE_SUPPLY_V = 5.0;
const float RESISTOR_OHMS = 50e3;
const float SENSITIVITY_A_PER_LUX = 10.0e-9;

// Dark offsets for each sensor (measured with sensors covered).
const float DARK_OFFSET_V[4] = {
  0.0, 0.0, 0.0, 0.0
};

const int SAMPLES = 64;
const byte SENSOR_COUNT = 4;

void setup() {
  Serial.begin(9600);
  analogReference(DEFAULT);

  // Discard first ADC conversion for each sensor.
  for (byte i = 0; i < SENSOR_COUNT; i++) {
    analogRead(SENSOR_PINS[i]);
  }

  delay(10);

  Serial.println(F("Four BPX65 sensor test"));
  Serial.println(F("Sensor | ADC | V_resistor | V_diode | Current (uA) | Lux"));
}

void loop() {

  for (byte sensor = 0; sensor < SENSOR_COUNT; sensor++) {

    unsigned long sum = 0;

    // Average ADC readings.
    for (int i = 0; i < SAMPLES; i++) {
      sum += analogRead(SENSOR_PINS[sensor]);
      delay(2);
    }

    float adc = sum / (float)SAMPLES;

    // Convert ADC reading to voltage.
    float resistorV = adc * ADC_REFERENCE_V / 1023.0;
    float diodeV = DIODE_SUPPLY_V - resistorV;

    // Remove dark offset.
    float signalV = resistorV - DARK_OFFSET_V[sensor];

    if (signalV < 0.0) signalV = 0.0;

    // Calculate photocurrent and approximate illuminance.
    float currentA = signalV / RESISTOR_OHMS;
    float luxEstimate = currentA / SENSITIVITY_A_PER_LUX;

    // Print results.
    Serial.print(F("A"));
    Serial.print(sensor);

    Serial.print(F(" | "));
    Serial.print(adc, 1);

    Serial.print(F(" | "));
    Serial.print(resistorV, 4);

    Serial.print(F(" | "));
    Serial.print(diodeV, 4);

    Serial.print(F(" | "));
    Serial.print(currentA * 1.0e6, 2);

    Serial.print(F(" | "));

    if (adc >= 1020.0 || diodeV <= 0.0) {
      Serial.println(F("OUT_OF_RANGE"));
    } else {
      Serial.println(luxEstimate, 0);
    }
  }

  Serial.println(F("------------------------------"));
  delay(500);
}
