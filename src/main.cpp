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
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R2, D1, D2);

// alternative:

// SSD1306 Controller, Mode 1 = Page Buffer Mode, u8g2(orientation, clock, data)
//U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, D1, D2);

// SSD1306 Controller, 8x8 Mode
//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(D1, D2);

#define LCDWidth u8g2.getDisplayWidth()
#define ALIGN_CENTER(t) ((LCDWidth - (u8g2.getUTF8Width(t))) / 2)
#define ALIGN_RIGHT(t) (LCDWidth - u8g2.getUTF8Width(t))
#define ALIGN_LEFT 0

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void print_menuline(const char *text)
{
  u8g2.setFont(u8g2_font_tenthinguys_tf);
  u8g2.setCursor(ALIGN_CENTER(text), 63);
  u8g2.print(text);
}

void print_status(const char *text)
{
  u8g2.setFont(u8g2_font_bitcasual_t_all);
  u8g2.setCursor(ALIGN_CENTER(text), 32);
  u8g2.print(text);
}

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  u8g2.clearBuffer();
  print_menuline("WiFi-Setup");
  print_status("Connecting...");
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
  print_menuline("WiFi-Setup");
  print_status("Success!");
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

  u8g2.clearBuffer();
  print_menuline(appname);
  u8g2.setFont(u8g2_font_logisoso32_tf);
  u8g2.setCursor(ALIGN_CENTER(output), 42);
  u8g2.print(output);
  u8g2.sendBuffer();
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

    u8g2.clearBuffer();
    print_menuline("MQTT-Setup");
    print_status("Connecting...");
    u8g2.sendBuffer();

    // Create a random client ID
    String clientId = "ESP-PowerMeter-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass))
    {
      Serial.println(" connected!");

      u8g2.clearBuffer();
      print_menuline("MQTT-Setup");
      print_status("Success!");
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

      u8g2.clearBuffer();
      print_menuline("MQTT-Setup");
      u8g2.setFont(u8g2_font_bitcasual_t_all);
      u8g2.setCursor(ALIGN_CENTER("Error!"), 22);
      u8g2.print("Error!");
      u8g2.setCursor(ALIGN_CENTER("Retry in 5s..."), 34);
      u8g2.print("Retry in 5s...");
      u8g2.sendBuffer();

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
  u8g2.enableUTF8Print();

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
