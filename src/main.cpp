/*
 * 
 *  PotWatcher - v1.0
 *  Code by Gavin Tryzbiak
 *  https://github.com/GavinTryz
 *  
 *  Personal program built for ESP32 to make my Google Home tell me when my water on the stove is boiling.
 *  
 */

#include "credentials.h"
#include "OTA.h"
#include <Arduino.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <DHT.h>
#include <esp8266-google-home-notifier.h>
#include <EEPROM.h>

GoogleHomeNotifier ghn;
DHT dht(DHT_PIN, DHT_TYPE);

// MQTT Setup
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, ADAFRUITIO_SERVER, ADAFRUITIO_SERVERPORT, ADAFRUITIO_USER, ADAFRUITIO_KEY);
Adafruit_MQTT_Subscribe commandFeed = Adafruit_MQTT_Subscribe(&mqtt, ADAFRUITIO_USER ADAFRUITIO_FEED);
Adafruit_MQTT_Publish reportFeed = Adafruit_MQTT_Publish(&mqtt, ADAFRUITIO_USER ADAFRUITIO_FEED);

// Variables
boolean takeMeasurements = false;

void WiFi_connect();
void MQTT_connect();
void GHN_setup();

void setup()
{
  // Utilities setup
  Serial.begin(115200);
  EEPROM.begin(1); // The humidity threshold (*1* byte) will be stored in address 0 of EEPROM to avoid losing it during power loss
  dht.begin();
  ArduinoOTA.setHostname("PotWatcher"); // GoogleHomeNotifier seems to stop this from working, but there doesn't appear to be side effects. Kept in the off chance GHN is updated.
  WiFi_connect();
  GHN_setup();
  setupOTA();
}

void loop()
{
  // Utilities verification
  while(WiFi.status() != WL_CONNECTED)
  {
    WiFi.disconnect();
    WiFi_connect();
    if(WiFi.status() == WL_CONNECTED)
    {
      GHN_setup();
      mqtt.subscribe(&commandFeed);
    }
    else
      delay(5000);
  }

  MQTT_connect(); // Make sure ESP32 is connected to broker

  if(takeMeasurements)
  {
    byte humidity = (byte)dht.readHumidity();
    Serial.println("Measured humidity of " + String(humidity) + "%");
    if(humidity > EEPROM.read(0))
    {
      Serial.println("Water is boiling!");
      takeMeasurements = false;
      reportFeed.publish("potwatcher boiling");
      if (ghn.notify("The water is now boiling.") != true)
        Serial.println(ghn.getLastError());
    }
  }

  // Read commands
  Adafruit_MQTT_Subscribe *tempSubscription;
  while((tempSubscription = mqtt.readSubscription(4500))) // Listen for new message for X ms
  {
    // Verify subscription matches expected feed (not strictly necessary in a 1-feed program, but good practice)
    if(tempSubscription == &commandFeed)
    {
      // Execute commands (if intended for PotWatcher)
      String command = (char *)commandFeed.lastread;

      Serial.print("Heard command: ");
      Serial.println(command);

      if(command.equals("potwatcher watch")) // Watch
      {
        Serial.println("Now watching pot.");
        takeMeasurements = true;
      }
      else if(command.equals("potwatcher stop")) // Stop
      {
        Serial.println("No longer watching pot.");
        takeMeasurements = false;
      }
      else if(command.equals("potwatcher getHumidity")) // Get humidity
      {
        Serial.println("Saying the humidity.");
        String humidityPhrase = "The stove humidity sensor reads " + String((int)dht.readHumidity()) + " percent";
        if (ghn.notify(humidityPhrase.c_str()) != true)
          Serial.println(ghn.getLastError());
      }
      else if(command.equals("potwatcher updateThreshold")) // Update threshold (to current humidity value. Stored in EEPROM address 0 so it is not forgotten on restart)
      {
        Serial.println("Saving the current humidity as the threshold to EEPROM.");
        EEPROM.write(0, (byte)dht.readHumidity());
        EEPROM.commit();
      }
      else if(command.substring(0, 23).equals("potwatcher setThreshold")) // Set threshold ()
      {
        Serial.println("Saving " + command.substring(24) + " to EEPROM.");
        EEPROM.write(0, (byte)(command.substring(24).toInt()));
        EEPROM.commit();
      }
      else
      {
        Serial.println("Command not intended for PotWatcher.");
      }
    }
  }

  ArduinoOTA.handle(); // Handle OTA update system (~40us)
}

// MQTT connection helper
void MQTT_connect()
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0)
  { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000); // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

void WiFi_connect()
{
  Serial.print("Connecting to WiFi network");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(150);
    Serial.print(".");
  }
  Serial.print("\nConnected! IP: ");
  Serial.println(WiFi.localIP());
}

void GHN_setup()
{
  Serial.println("Connecting to Google Home...");
  if (ghn.device(GOOGLE_HOME_NAME, "en") != true)
  {
    Serial.println(ghn.getLastError());
    return;
  }
  Serial.print("Found Google Home (");
  Serial.print(ghn.getIPAddress());
  Serial.print(":");
  Serial.print(ghn.getPort());
  Serial.println(")");

  // Startup phrase
  String startupPhrase = "Pot Watcher available. The current threshold is set to " + String(EEPROM.read(0)) + " percent. You can change this by telling me to set the threshold.";
  if (ghn.notify(startupPhrase.c_str()) != true)
    Serial.println(ghn.getLastError());
}
