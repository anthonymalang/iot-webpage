#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

#include "Credentials.h"

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

#include "UUID.h"

#define SEALEVELPRESSURE_HPA (1013.25)
#define BOOT_PIN 0
#define LED_PIN 48  
#define NUM_LEDS 1

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

Adafruit_BME680 bme(&Wire);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

UUID uuid;

const char *base_location = "/data/temp/";
bool send_data;
volatile bool button_pressed = false;

void ARDUINO_ISR_ATTR button_isr() {
  button_pressed = true;
}

void setup() {
  pinMode(BOOT_PIN, INPUT_PULLUP);
  attachInterrupt(BOOT_PIN, button_isr, RISING);
  Serial.begin(115200);

  while (!Serial)
    ;

  /*******************************************************************
    SENSOR SETUP 
  ********************************************************************/

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1)
      ;
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);  // 320*C for 150 ms


  /*******************************************************************
    WIFI SETUP 
  ********************************************************************/

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /*******************************************************************
    FIREBASE SETUP 
  ********************************************************************/

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

  uint32_t seed1 = random(999999999);
  uint32_t seed2 = random(999999999);
  uuid.seed(seed1, seed2);

  /*******************************************************************
    LED SETUP 
  ********************************************************************/

  strip.begin();
  strip.setBrightness(15); // Set brightness (0-255)
  strip.setPixelColor(0, strip.Color(255, 0, 0));
  strip.show();


  send_data = false;
}

void loop() {
  if (button_pressed) {
    if (!send_data) {
      strip.setPixelColor(0, strip.Color(0, 255, 0));
      Serial.print("Send data enabled\n");
    } else {
      strip.setPixelColor(0, strip.Color(255, 0, 0));
      Serial.print("Send data disabled\n");
    }
    send_data = !send_data;
    strip.show();
    button_pressed = false;
  }

  if (!bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  Serial.print("Temperature = ");
  Serial.print(bme.temperature * (9.0 / 5.0) + 32);
  Serial.println(" *F");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.println();

  if (send_data) {
    uuid.generate();

    char location[128];
    strcpy(location, base_location);
    strcat(location, uuid.toCharArray());
    Serial.printf("%s\n", location);
    if (!Firebase.setFloat(fbdo, location, bme.temperature)) {
      Serial.printf("%s\n", fbdo.errorReason().c_str());
    }
  }

  delay(1000);
}
