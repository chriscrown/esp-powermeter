#include <Arduino.h>
#include <U8g2lib.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "settings.h"

// Wemos D1 mini pro
// VCC -> 3,3V
// GND -> GND
// D1  -> SCL
// D2  -> SDA

const char *headline = "POWERMETER 1.0";

// SSD1306 Controller, Mode F = Full screen buffer mode, u8g2(orientation, clock, data)
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, D1, D2);

// alternative:

// SSD1306 Controller, Mode 1 = Page Buffer Mode, u8g2(orientation, clock, data)
//U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, D1, D2);

// SSD1306 Controller, 8x8 Mode
//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(D1, D2);

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_bitcasual_t_all);
  u8g2.drawStr(49, 8, "WiFi");
  u8g2.setFont(u8g2_font_tenthinguys_tf);
  u8g2.drawStr(20, 30, "WiFi-Setup...");
  u8g2.sendBuffer();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_bitcasual_t_all);
  u8g2.drawStr(49, 8, "WiFi");
  u8g2.setFont(u8g2_font_tenthinguys_tf);
  u8g2.drawStr(20, 30, "Connected!");
  u8g2.sendBuffer();
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char output[length + 2];
  for (int i = 0; i < (int)length; i++)
  {
    Serial.print((char)payload[i]);
    output[i] = (char)payload[i];
  }
  output[length] = 'W';
  output[length + 1] = 0;
  Serial.println();

  int fill = 5 - (int)length; // max 5 digits

  // payload detected
  if ((char)payload[0])
  {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_bitcasual_t_all);
    u8g2.drawStr(10, 8, headline);
    u8g2.setFont(u8g2_font_logisoso32_tf);
    u8g2.drawStr(fill * 13, 55, output);
    u8g2.sendBuffer();
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_bitcasual_t_all);
    u8g2.drawStr(47, 8, "MQTT");
    u8g2.setFont(u8g2_font_tenthinguys_tf);
    u8g2.drawStr(20, 30, "MQTT-Setup...");
    u8g2.sendBuffer();

    // Create a random client ID
    String clientId = "ESP-PowerMeter-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass))
    {
      Serial.println(" connected!");

      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_bitcasual_t_all);
      u8g2.drawStr(47, 8, "MQTT");
      u8g2.setFont(u8g2_font_tenthinguys_tf);
      u8g2.drawStr(20, 30, "Connected!");
      u8g2.sendBuffer();

      // Once connected, publish an announcement...
      client.publish(outTopic, "Hello from ESP-PowerMeter");
      // ... and resubscribe
      client.subscribe(inTopic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(9600);
  delay(200);
  Serial.print("\n\n");
  u8g2.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}
