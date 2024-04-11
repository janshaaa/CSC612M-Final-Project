#include <csc612m_inferencing.h>
#include <Arduino.h>

/** Audio buffers, pointers, and selectors */
typedef struct {
    int16_t *buffer;
    uint32_t n_samples;
} inference_t;

static inference_t inference;
static bool debug_nn = true; // Set this to true to see debug information
// Define Serial object for communication with PySerial

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo");

    // Initialize the inference buffer
    inference.buffer = (int16_t *)malloc(EI_CLASSIFIER_RAW_SAMPLE_COUNT * sizeof(int16_t));
    if (inference.buffer == NULL) {
        Serial.println("ERR: Could not allocate audio buffer");
        return;
    }
    inference.n_samples = EI_CLASSIFIER_RAW_SAMPLE_COUNT;

    // Wait for PySerial data
    Serial.println("Waiting for data...");
}

void loop() {
    // Wait for data from PySerial
    while (!Serial.available()) {
        delay(10);
    }

    // Read data from PySerial into the inference buffer
    uint32_t bytesRead = Serial.readBytes((char *)inference.buffer, inference.n_samples * sizeof(int16_t));

    if (bytesRead != inference.n_samples * sizeof(int16_t)) {
        Serial.println("ERR: Failed to read data from PySerial");
        return;
    }

    // Run inference
    signal_t signal;
    signal.total_length = inference.n_samples;
    signal.get_data = &streamed_audio_signal_get_data;
    ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);
    if (r != EI_IMPULSE_OK) {
        Serial.printf("ERR: Failed to run classifier (%d)\n", r);
        return;
    }

    // Print the predictions
    Serial.println("Predictions:");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        Serial.print(result.classification[ix].label);
        Serial.print(": ");
        Serial.println(result.classification[ix].value, 5);
    }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    Serial.print("Anomaly score: ");
    Serial.println(result.anomaly, 3);
#endif

    // Wait for PySerial data for next inference
    Serial.println("Waiting for data...");
}

int streamed_audio_signal_get_data(size_t offset, size_t length, float *out_ptr) {
    numpy::int16_to_float(&inference.buffer[offset / sizeof(int16_t)], out_ptr, length);
    return 0;
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif
