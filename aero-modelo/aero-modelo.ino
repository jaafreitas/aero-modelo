/*
  
  nRF24L01: http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
  VCC 3.3V !!! NOT 5V

*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

#define CE_PIN   9
#define CSN_PIN 10
#define SCK_PIN 13
#define MOSI_PIN 11
#define MISO_PIN 12

const uint64_t pipe = 0xAABBCCDDEELL;

RF24 radio(CE_PIN, CSN_PIN);
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

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
  Serial.println();
  Serial.print("Iniciando radio...");
  radio.begin();
  radio.setAutoAck(false);
  radio.setChannel(108);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);  
  radio.openReadingPipe(1, pipe);
  radio.startListening();
  Serial.println(" Ok!");

  servo1.attach(4);
  servo2.attach(5);
  servo3.attach(6);
  servo4.attach(7);
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

    servo1.write(map(dado_controle.X1, 0, 1023, 0, 179));   
    servo2.write(map(dado_controle.Y1, 0, 1023, 0, 179));   
    servo3.write(map(dado_controle.X2, 0, 1023, 0, 179));   
    servo4.write(map(dado_controle.Y2, 0, 1023, 0, 179));   
  }
}

