/*
- CONNECTIONS: nRF24L01 Modules See:

http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
  1 - GND
  2 - VCC 3.3V !!! NOT 5V
  3 - CE to Arduino pin 9
  4 - CSN to Arduino pin 10
  5 - SCK to Arduino pin 13p
  6 - MOSI to Arduino pin 11
  7 - MISO to Arduino pin 12
  8 - UNUSED

*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN   9
#define CSN_PIN 10

const uint64_t pipe = 0xE8E8F0F0E1LL;

RF24 radio(CE_PIN, CSN_PIN);

struct dado_controle {
  int X1;
  int Y1;
  bool botao1;
  int X2;
  int Y2;
  bool botao2;
} dado_controle;

void setup() {
  Serial.begin(115200);

  Serial.print("Iniciando radio...");
  radio.begin();
  radio.setAutoAck(false);
  radio.setChannel(108);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);  
  radio.openReadingPipe(1, pipe);
  radio.startListening();
  Serial.println(" Ok!");
}

void loop() {
 if (radio.available()) {
   radio.read(&dado_controle, sizeof(dado_controle));
   Serial.print(millis()); Serial.print(" ms: ");
   
   Serial.print("X1: "); Serial.print(dado_controle.X1); Serial.print("\t");
   Serial.print("Y1: "); Serial.print(dado_controle.Y1); Serial.print("\t");
   Serial.print("botao1: "); Serial.print(dado_controle.botao1); Serial.print("\t");
   
   Serial.print("X2: "); Serial.print(dado_controle.X2); Serial.print("\t");
   Serial.print("Y2: "); Serial.print(dado_controle.Y2); Serial.print("\t");
   Serial.print("botao2: "); Serial.print(dado_controle.botao2); Serial.print("\t");
   
   Serial.println();
 }
}

