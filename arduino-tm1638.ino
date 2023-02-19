#include "TM1630.hpp"
#include <ArduinoJson.h>

#define TM1638_STB 7
#define TM1638_CLK 9
#define TM1638_DIO 8

#define JSON_SIZE 256

// TM1638 state
TM1638 tm(TM1638_STB, TM1638_CLK, TM1638_DIO);
uint8_t rawButtonState;
uint8_t rawButtonStatePrevious;

void setup()
{
  // Initialize GPIO
  pinMode(TM1638_STB, OUTPUT);
  pinMode(TM1638_CLK, OUTPUT);
  pinMode(TM1638_DIO, OUTPUT);

  // Initialize and wait for serial
  Serial.begin(115200);
  while (!Serial)
  {
    continue;
  }

  // Initialize TM1638
  tm.init();
}

static inline bool buttonEdgeTrigger(uint8_t buttonState, uint8_t &buttonStatePrevious)
{
  if (buttonState != buttonStatePrevious)
  {
    buttonStatePrevious = buttonState;
    return true;
  }
  return false;
}

static inline void processLed(StaticJsonDocument<JSON_SIZE> &ledJson)
{
  if (ledJson["single"])
  {
    int position = ledJson["position"];
    bool state = ledJson["state"];
    tm.setLED(position, state);
  }
  else
  {
    for (int i = 0; i < 8; i++)
    {
      bool state = ledJson["state"][String(i + 1)];
      tm.setLED(i, state);
    }
  }
}

static inline void processSeg(StaticJsonDocument<JSON_SIZE> &segJson)
{
  if (segJson["single"])
  {
    int position = segJson["position"];
    String state = segJson["state"];
    tm.setChar(position, state[0]);
  }
  else
  {
    String text = segJson["state"];
    tm.setString(text);
  }
}

void processOutput()
{
  StaticJsonDocument<JSON_SIZE> deviceState;
  JsonObject buttons = deviceState.createNestedObject("buttons");

  rawButtonState = tm.readButtons();
  if (!buttonEdgeTrigger(rawButtonState, rawButtonStatePrevious))
  {
    return;
  }

  for (int i = 0; i < 8; i++)
  {
    buttons["s" + String(i + 1)] = rawButtonState & (1 << i) ? true : false;
  }
  serializeJson(deviceState, Serial);
  Serial.println();
  deviceState.clear();
}

void processInput()
{
  StaticJsonDocument<JSON_SIZE> interfaceState;

  if (Serial.available())
  {
    String serialBuffer = Serial.readStringUntil('\n');
    DeserializationError error = deserializeJson(interfaceState, serialBuffer);
    if (error)
    {
      return;
    }
  }

  String device = interfaceState["device"];
  if (device == "led")
  {
    processLed(interfaceState);
  }
  if (device == "seg")
  {
    processSeg(interfaceState);
  }

  interfaceState.clear();
}

void loop()
{
  processOutput();
  processInput();
}
