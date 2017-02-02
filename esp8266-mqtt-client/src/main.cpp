#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DNSServer.h>
#include <WiFiManager.h>

#define LISTEN_TOPIC "displayMsg"
#define OLED_RESET 0

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;
IPAddress server;
uint16_t port;

Adafruit_SSD1306 display(OLED_RESET);
char hostString[16] = {0};

void displayMessage(const char* message)
{
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.clearDisplay();
    display.println(message);
    display.display();
}

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.clearDisplay();
    for (int i=0;i<length;i++) {
        display.print((char)payload[i]);
    }

    display.display();

}

void mqttReconnect()
{
    while (!client.connected()) {
        displayMessage("Connecting\nto MQTT\nbroker");
        if (client.connect("ESP8266Client")) {
            displayMessage("Connected\n\nWaiting\nfor first\nmessage...");
            client.subscribe(LISTEN_TOPIC);
        } else {
            delay(5000);
        }
    }
}

void configModeCallback (WiFiManager *myWiFiManager)
{
    displayMessage("Connect to:");

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.clearDisplay();
    display.println(myWiFiManager->getConfigPortalSSID());
    display.display();
}


void setup()
{
    Serial.begin(9600);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.display();

    displayMessage("Connecting\nto WIFI...");

    WiFiManager wifiManager;
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.autoConnect();

    sprintf(hostString, "ESP_%06X", ESP.getChipId());
    if (!MDNS.begin(hostString)) {
        displayMessage("ERROR: MDNS");
        while(true) {};
    }

    displayMessage("Searching\nfor MQTT\nservice...");

    bool foundService = false;
    while (!foundService) {
        int n = MDNS.queryService("mqtt", "tcp");
        if (n == 0) {
            delay(5000);
            continue;
        }

        foundService = true;
        server = MDNS.IP(0);
        port = MDNS.port(0);
    }

    displayMessage("Connecting...");

    client.setServer(server, port);
    client.setCallback(mqttCallback);

}

void loop()
{
    if (!client.connected()) {
        mqttReconnect();
    }

    client.loop();
}
