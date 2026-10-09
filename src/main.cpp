#include <Arduino.h>
#include "lora_if.h"

constexpr int LORA_RX = 16;
constexpr int LORA_TX = 17;
constexpr int LORA_M0 = 5;
constexpr int LORA_M1 = 18;
constexpr uint8_t LORA_UART = 1;

constexpr size_t MAX_PACKET_SIZE = 64;
constexpr uint8_t WRITE_CONFIG_COMMAND[] = {0xC0, 0x00, 0x06, 0x98, 0x48, 0x70, 0x01, 0x00, 0x80};

LoRa lora(LORA_UART, LORA_RX, LORA_TX, LORA_M0, LORA_M1);
uint32_t sendval = 0;
bool loraReady = false;

void printHexBytes(const uint8_t *bytes, size_t length)
{
    size_t index = 0;

    for (index = 0; index < length; ++index)
    {
        if (index > 0)
            Serial.print(' ');
        if (bytes[index] < 0x10)
            Serial.print('0');
        Serial.print(bytes[index], HEX);
    }
}

void setup()
{
    uint8_t responseBuffer[sizeof(WRITE_CONFIG_COMMAND)] = {};
    size_t responseLength = 0;

    Serial.begin(115200);

    if (!lora.begin())
    {
        Serial.println(F("LoRa initialization failed"));
        return;
    }

    if (!lora.writeConfig(WRITE_CONFIG_COMMAND, sizeof(WRITE_CONFIG_COMMAND)))
    {
        Serial.println(F("LoRa config write failed"));
        return;
    }

    Serial.println("Read config");
    responseLength = lora.receiveConfigResponse(responseBuffer, sizeof(responseBuffer));

    Serial.print(F("Response: "));
    printHexBytes(responseBuffer, responseLength);
    Serial.println();

    Serial.println("LoRa TX ready");
    loraReady = true;
}

void loop()
{
    uint8_t message[4] = {};
    uint8_t buffer[MAX_PACKET_SIZE] = {};
    size_t bytesRead = 0;

    if (!loraReady)
    {
        delay(1000);
        return;
    }

    message[0] = static_cast<uint8_t>(sendval >> 24);
    message[1] = static_cast<uint8_t>(sendval >> 16);
    message[2] = static_cast<uint8_t>(sendval >> 8);
    message[3] = static_cast<uint8_t>(sendval);

    if (!lora.sendMessage(message, sizeof(message)))
    {
        Serial.println(F("LoRa send failed"));
    }

    delay(2000);
    ++sendval;

    bytesRead = lora.receiveAvailable(buffer, sizeof(buffer));
    if (bytesRead > 0)
    {
        Serial.print(F("Received: "));
        printHexBytes(buffer, bytesRead);
        Serial.println();
    }
}