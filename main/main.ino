#include "UbidotsEsp32Mqtt.h"
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 21
#define RST_PIN 22

const char *UBIDOTS_TOKEN = "";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "";      // Put here your Wi-Fi password
const char *DEVICE_LABEL = "";   // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL_ID = ""; // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL_AMOUNT = "";
const char *VARIABLE_LABEL_SUBSCRIBE = "";

const int limite_ocupacao = 20;
float amount = 0;
String received;

Ubidots ubidots(UBIDOTS_TOKEN);
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Cria o objeto MFRC522

void callback(char *topic, byte *payload, unsigned int length)
{
  received = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] = ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    received.concat((char)payload[i]);
  }
  Serial.println();

   if (!received.toInt()) {
    amount = 0;
    ubidots.add(VARIABLE_LABEL_AMOUNT, amount);
    ubidots.publish(DEVICE_LABEL);
  }
}

void setup()
{
  Serial.begin(115200);
  ubidots.setDebug(true);  // uncomment this to make debug messages available
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_SUBSCRIBE);

  SPI.begin();          // Inicia a comunicação SPI
  mfrc522.PCD_Init();   // Inicia o módulo MFRC522
  Serial.println("Aproxime uma tag RFID");
}

void loop()
{
  if (!ubidots.connected())
  {
    ubidots.reconnect();
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_SUBSCRIBE);
  }

  // Verifica se há uma tag presente
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() ) {
    if (amount < limite_ocupacao && received.toInt()) {
      // Lê o UID da tag
      String uid = "";
      amount++;
      uid.concat(String(mfrc522.uid.uidByte[0], DEC));
      Serial.println("Tag lida: " + uid);
      ubidots.add(VARIABLE_LABEL_ID, uid.toFloat());
      ubidots.add(VARIABLE_LABEL_AMOUNT, amount);
      ubidots.publish(DEVICE_LABEL);
      Serial.println("Publicando os dados para Ubidots Cloud");
      Serial.println("----------------------------------------------");
      delay(1000);
    }

    // Retorna um aviso se a sala estiver cheia
    if (amount >= limite_ocupacao) {
      Serial.println("A sala está lotada!");
      Serial.println("----------------------------------------------");
      delay(1000);
    }

    // Retorna um aviso se o sensor estiver desligado
    if (!received.toInt()) {
      Serial.println("O sensor está desligado!");
      Serial.println("----------------------------------------------");
      delay(1000);
    }
  }
  ubidots.loop();
}