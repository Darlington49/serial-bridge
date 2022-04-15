//******************************************************************************
// WiFiUartTransparentBridge
// Version 1.1.0
// Note
// This sketch is based on "WiFiTelnetToSerial - Example Transparent UART to Telnet Server for esp8266"
//******************************************************************************

#include <WiFi.h>
#include "config.h"

//#define STATIC_IP_ADDR
HardwareSerial Serial_one(1);
HardwareSerial Serial_two(2);
HardwareSerial *COM[NUM_COM] = {&Serial, &Serial_one, &Serial_two};

WiFiServer server_0(SERIAL0_TCP_PORT);
WiFiServer server_1(SERIAL1_TCP_PORT);
WiFiServer server_2(SERIAL2_TCP_PORT);
WiFiServer *server[NUM_COM] = {&server_0, &server_1, &server_2};
WiFiClient localClient[NUM_COM];

#ifdef STATIC_IP_ADDR
IPAddress staticIP(192, 168, 0, 25);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
#endif

void setup()
{
  COM[0]->begin(UART_BAUD0);
  COM[1]->begin(UART_BAUD1, SERIAL_PARAM1, SERIAL1_RXPIN, SERIAL1_TXPIN);
  COM[2]->begin(UART_BAUD2, SERIAL_PARAM2, SERIAL2_RXPIN, SERIAL2_TXPIN);

  // COM[0].begin(baudrate);
  WiFi.begin(ssid, password);
#ifdef STATIC_IP_ADDR
  WiFi.config(staticIP, gateway, subnet);
#endif
  COM[0]->print("\nConnecting to ");
  COM[0]->println(ssid);

  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20)
    delay(500);
  if (i == 21)
  {
    COM[0]->print("Could not connect to");
    COM[0]->println(ssid);
    while (1)
      delay(500);
  }

  // start UART and the server
  server[0]->begin();
  server[0]->setNoDelay(true);

  server[1]->begin();
  server[1]->setNoDelay(true);

  server[2]->begin();
  server[2]->setNoDelay(true);

  COM[0]->print("Ready! Use 'Uart-WiFi Bridge ");
  COM[0]->print(WiFi.localIP());
  COM[0]->println(" to connect");
}

void loop()
{
  for (int num = 0; num < NUM_COM; num++)
  {
    // check if there are any new clients
    if (server[num]->hasClient())
    {
      if (!localClient[num].connected())
      {
        if (localClient[num])
          localClient[num].stop();
        localClient[num] = server[num]->available();
      }
    }

    // check a client for data
    if (localClient[num] && localClient[num].connected())
    {
      if (localClient[num].available())
      {
        size_t len = localClient[num].available();
        uint8_t sbuf[len];
        localClient[num].readBytes(sbuf, len);
        COM[num]->write(sbuf, len);
        // read from wich port
        COM[0]->write(sbuf, len);

      }
    }

    // check UART for data
    if (COM[num]->available())
    {
      size_t len = COM[num]->available();
      uint8_t sbuf[len];
      COM[num]->readBytes(sbuf, len);
      if (localClient[num] && localClient[num].connected())
      {
        // read from  which com
        COM[0]->write(sbuf, len);
        localClient[num].write(sbuf, len);
      }
    }
  }
}
