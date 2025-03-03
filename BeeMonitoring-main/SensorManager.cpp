#include "SensorManager.h"
#include "SystemConfig.h"
#include <driver/i2s.h>

const i2s_port_t I2S_PORT = I2S_NUM_0;


// Define the task handle
TaskHandle_t xTaskHandle_SensorManager = NULL;
SemaphoreHandle_t xSemaphore_Sensor = NULL;
bool sensorDataReady = false;

// Define the global sensor data
SensorData_t sensorData = { 0 };


// Timer interval for reading sensors (in milliseconds)
const uint32_t SENSOR_READ_INTERVAL_MS = 1000;

void modbusTask(void* pvParameters);
void SensorManagerTask(void* pvParameters);

void startSensorManagerTask() {

  xSemaphore_Sensor = xSemaphoreCreateMutex();
  if (xSemaphore_Sensor == NULL) {
    Serial.println("Failed to create mutex for sensor");
    return;
  }

  xTaskCreatePinnedToCore(
    SensorManagerTask,           // Task function
    "Sensor Manager",            // Task name
    4096,                        // Stack size
    NULL,                        // Task parameters
    1,                           // Task priority
    &xTaskHandle_SensorManager,  // Task handle
    1                            // Core ID
  );
}



void SensorManagerTask(void* pvParameters) {
#ifdef DEBUG_SENSORS
  Serial.print("Sensor Manager running on core ");
#endif
  Serial.println(xPortGetCoreID());

// Initialize sensor peripherals here
#ifdef DEBUG_SENSORS
  Serial.println("Initializing sensors...");
#endif

  // Init I2S Sensor
  esp_err_t err;

  // The I2S config as per the example
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
  };

  // The pin config as per the setup
  const i2s_pin_config_t pin_config = {
    .bck_io_num = 26,                   // Serial Clock (SCK)
    .ws_io_num = 25,                    // Word Select (WS)
    .data_out_num = I2S_PIN_NO_CHANGE,  // not used (only for speakers)
    .data_in_num = 27                   // Serial Data (SD)
  };

  // Configuring the I2S driver and pins.
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true);
  }
  Serial.println("I2S driver installed.");

  // Main task loop
  for (;;) {
    // Locking the semaphore for exclusive access
    if (xSemaphoreTake(xSemaphore_Sensor, portMAX_DELAY) == pdTRUE) {
      Serial.println("Starting 5000-sample averaging...");

      // Variables for averaging
      float amplitude_sum = 0;
      int sample_count = 0;
      const int SAMPLE_TARGET = 5000; // Perform exactly 5000 samples

      // Collect exactly 5000 samples
      for (int i = 0; i < SAMPLE_TARGET; i++) {
        int32_t sample = 0;
        int bytes_read = i2s_pop_sample(I2S_PORT, (char*)&sample, portMAX_DELAY);
        if (bytes_read > 0) {
          // Normalize and accumulate absolute value
          float normalized_sample = sample / 8388607.0; // Assuming 24-bit microphone
          amplitude_sum += fabs(normalized_sample);
          sample_count++;
        }
      }

      // Compute the average amplitude after 5000 samples
      float average_amplitude = amplitude_sum / sample_count;
      sensorData.averaged_sound_level = average_amplitude;

      // Store the result in sensorData
      sensorData.sound_level = average_amplitude;
      sensorDataReady = true;

      // Output the average amplitude for debugging
      Serial.printf("Average Amplitude over 5000 samples: %.6f\n", average_amplitude);

      // Release the semaphore
      xSemaphoreGive(xSemaphore_Sensor);
    }

    // Delay to prevent continuous reading
    vTaskDelay(pdMS_TO_TICKS(2000)); // Adjust delay as needed
  }
}
