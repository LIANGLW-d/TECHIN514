#include <Arduino.h>

// Define the pin connections for voltage measurement
const int VOUT1_PIN = 6;  // Pin for VOUT1
const int VOUT2_PIN = 2;  // Pin for VOUT2

void setup() {
    // Initialize Serial communication for debugging
    Serial.begin(115200);

    // Attach ADC to the specified pins
    adcAttachPin(VOUT1_PIN);  
    adcAttachPin(VOUT2_PIN);

    // Set ADC attenuation to handle higher voltage ranges
    analogSetAttenuation(ADC_11db);  // For voltage up to ~3.3V
}

void loop() {
    // Read ADC values from the pins
    int adc1 = analogRead(VOUT1_PIN);
    int adc2 = analogRead(VOUT2_PIN);

    // Convert ADC values to actual voltage (assuming 3.3V reference)
    float voltage1 = (adc1 / 4095.0) * 3.3;  // ESP32 ADC resolution is 12-bit (0-4095)
    float voltage2 = (adc2 / 4095.0) * 3.3;

    // Print the voltage values to the Serial Monitor
    Serial.print("VOUT1: "); Serial.print(voltage1); Serial.println(" V");
    Serial.print("VOUT2: "); Serial.print(voltage2); Serial.println(" V");
    Serial.println("----------------------");

    // Add a delay to make readings easier to observe
    delay(1000);  // 1 second delay
}
