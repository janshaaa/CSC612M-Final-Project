

#include <PDM.h>
#include <csc612m_inferencing.h>

#define SERIAL_BAUD_RATE 115200

/** Audio buffers, pointers, and selectors */
typedef struct {
    int16_t *buffer;
    uint32_t n_samples;
} inference_t;

static inference_t inference;
static signed short sampleBuffer[4096];
static bool debug_nn = true; 
static bool check = false; 

bool stream_read_success = false;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo");

    ei_printf("Inferencing settings:\n");
    ei_printf("\tInterval: %.2f ms.\n", (float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));

    if (microphone_inference_start() == false) {
        ei_printf("ERR: Could not allocate audio buffer\r\n");
        return;
    }
}

void loop() {
    ei_printf("Reading streamed data...\n");

    bool stream_read_success = read_streamed_data();


    if (!stream_read_success) {
        ei_printf("ERR: Failed to read streamed data...\n");
        return;
    }
    else{
        ei_printf("Streamed data read successfully\n");
    }
    

    signal_t signal;
    signal.total_length = inference.n_samples; 
    signal.get_data = &streamed_audio_signal_get_data;
    ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);
    if (r != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", r);
        return;
    }

    ei_printf("Predictions ");
    ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);
    ei_printf(": \n");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
    }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif
}

static void pdm_data_ready_inference_callback(void) {
    int bytesAvailable = PDM.available();
    int bytesRead = PDM.read((char *)&sampleBuffer[0], bytesAvailable);
    for (int i = 0; i < bytesRead >> 1; i++) {
        inference.buffer[i] = sampleBuffer[i];
    }
}

static bool microphone_inference_start() {
    inference.buffer = (int16_t *)malloc(EI_CLASSIFIER_RAW_SAMPLE_COUNT * sizeof(int16_t));
    if (inference.buffer == NULL) {
        return false;
    }

    inference.n_samples = EI_CLASSIFIER_RAW_SAMPLE_COUNT;

    PDM.onReceive(&pdm_data_ready_inference_callback);
    PDM.setBufferSize(4096);

    if (!PDM.begin(1, EI_CLASSIFIER_FREQUENCY)) {
        ei_printf("Failed to start PDM!");
        microphone_inference_end();
        return false;
    }

    PDM.setGain(127);

    return true;
}

static void microphone_inference_end() {
    PDM.end();
    free(inference.buffer);
}

bool read_streamed_data() {
    uint32_t bytesRead = 0;
    bool check = false;
    
    while (bytesRead < inference.n_samples * sizeof(int16_t)) {
        while (Serial.available() > 0 && bytesRead < inference.n_samples * sizeof(int16_t)) {
            int16_t value;
            Serial.readBytes((char *)&value, sizeof(int16_t));
            inference.buffer[bytesRead / sizeof(int16_t)] = value;
            ei_printf("Bytes read! ");
            bytesRead += sizeof(int16_t);
        }
        
        // if (sizeof(bytesRead) > inference.n_samples * sizeof(int16_t)) {
        //     ei_printf("Data read exceeds buffer size!\n");
        //     int size = sizeof(bytesRead);
        //     check = true;
        //     break;
        // }
    }
    
    // if(check == true){
    //     ei_printf("Data read exceeds buffer size!\n");
    // }

    return bytesRead == inference.n_samples * sizeof(int16_t);
}


int streamed_audio_signal_get_data(size_t offset, size_t length, float *out_ptr) {
    numpy::int16_to_float(&inference.buffer[offset / sizeof(int16_t)], out_ptr, length);
    return 0;
}
