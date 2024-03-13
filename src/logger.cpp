#include <Arduino.h>
// wifi
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
// for I2C to sensor
#include <Wire.h>
#include <SensirionI2CScd4x.h>
SensirionI2CScd4x scd4x;
// for connection to influxdb
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <secrets.h>

#define DEVICE "ESP8266"
#define TZ_INFO "UTC0"

int wifiError = 0;
int sensorError = 0;
int influxError = 0;

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
// Declare Data point
Point influxsensor("environment");

void printUint16Hex(uint16_t value);
void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2);

void setup()
{
    // SETUP LED
    // use LED for errors, LOW is ON
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // SETUP SERIAL
    Serial.begin(115200);
    while (!Serial)
    {
        delay(100);
    }

    // SETUP I2C
    Wire.begin(4, 5);
    scd4x.begin(Wire); // sensor

    // SETUP wifi
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to wifi");
    while (wifiMulti.run() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }
    Serial.println();

    // SETUP INFLUXDB
    // Accurate time is necessary for certificate validation and writing in batches
    // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
    // Syncing progress and the time will be printed to Serial.
    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
    // Check server connection
    if (client.validateConnection())
    {
        Serial.print("Connected to InfluxDB: ");
        Serial.println(client.getServerUrl());
    }
    else
    {
        Serial.print("InfluxDB connection failed: ");
        Serial.println(client.getLastErrorMessage());
        influxError = 1;
    }
    // Add tags to the data point
    influxsensor.addTag("device", DEVICE);
    influxsensor.addTag("SSID", WiFi.SSID());

    // SETUP SENSOR
    // stop potentially previously started measurement
    uint16_t error;
    char errorMessage[256];
    error = scd4x.stopPeriodicMeasurement();
    if (error)
    {
        Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        sensorError = 1;
    }
    uint16_t serial0;
    uint16_t serial1;
    uint16_t serial2;
    error = scd4x.getSerialNumber(serial0, serial1, serial2);
    if (error)
    {
        Serial.print("Error trying to execute getSerialNumber(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        sensorError = 1;
    }
    else
    {
        printSerialNumber(serial0, serial1, serial2);
    }

    // Start Measurement
    error = scd4x.startPeriodicMeasurement();
    if (error)
    {
        Serial.print("Error trying to execute startPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        sensorError = 1;
    }

    Serial.println("Waiting for first measurement... (5 sec)");
    if (wifiError || sensorError || influxError)
    {
        digitalWrite(LED_BUILTIN, LOW);
    }
    else
    {
        digitalWrite(LED_BUILTIN, HIGH);
    }
}

void loop()
{
    if (wifiError || sensorError || influxError)
    {
        digitalWrite(LED_BUILTIN, LOW);
        return;
    }

    uint16_t error;
    char errorMessage[256];

    // Clear fields for reusing the point. Tags will remain the same as set above.
    influxsensor.clearFields();

    // Read Measurement
    uint16_t co2 = 0;
    float temperature = 0.0f;
    float humidity = 0.0f;
    bool isDataReady = false;
    error = scd4x.getDataReadyFlag(isDataReady);
    if (error)
    {
        Serial.print("Error trying to execute getDataReadyFlag(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        sensorError = 1;
        return;
    }
    else
    {
        sensorError = 0;
    }
    if (!isDataReady)
    {
        return;
    }
    error = scd4x.readMeasurement(co2, temperature, humidity);
    if (error)
    {
        Serial.print("Error trying to execute readMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        sensorError = 1;
    }
    else
    {
        sensorError = 0;
        Serial.print("Co2:");
        Serial.print(co2);
        Serial.print("\t");
        Serial.print("Temperature:");
        Serial.print(temperature);
        Serial.print("\t");
        Serial.print("Humidity:");
        Serial.println(humidity);
    }

    // Store measured value into point
    influxsensor.addField("co2", co2);
    influxsensor.addField("temperature", temperature);
    influxsensor.addField("humidity", humidity);

    // Print what are we exactly writing
    Serial.print("Writing: ");
    Serial.println(influxsensor.toLineProtocol());

    // Check WiFi connection and reconnect if needed
    if (wifiMulti.run() != WL_CONNECTED)
    {
        Serial.println("Wifi connection lost");
        wifiError = 1;
    }
    else
    {
        wifiError = 0;
    }

    // Write point
    if (!client.writePoint(influxsensor))
    {
        Serial.print("InfluxDB write failed: ");
        Serial.println(client.getLastErrorMessage());
        influxError = 1;
    }
    else
    {
        Serial.println("InfluxDB write success");
        influxError = 0;
    }

    delay(100);
}

void printUint16Hex(uint16_t value)
{
    Serial.print(value < 4096 ? "0" : "");
    Serial.print(value < 256 ? "0" : "");
    Serial.print(value < 16 ? "0" : "");
    Serial.print(value, HEX);
}

void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2)
{
    Serial.print("Serial: 0x");
    printUint16Hex(serial0);
    printUint16Hex(serial1);
    printUint16Hex(serial2);
    Serial.println();
}