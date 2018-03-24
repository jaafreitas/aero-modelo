/*

  nRF24L01: http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
  VCC 3.3V !!! NOT 5V

 Comandos:
  joystick esquerdo:
    frente/trás: 
    esquerda/direita:
    botão:
  joystick direito:
    frente/trás: 
    esquerda/direita:
    botão:

*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN   9

// Arduino MEGA
#if defined(__AVR_ATmega2560__)
#define CSN_PIN 53
#define SCK_PIN 52
#define MOSI_PIN 51
#define MISO_PIN 50
#endif

// Arduino Uno, Nano e Duemilanove
#if defined(__AVR_ATmega328P__)
#define CSN_PIN 10
#define SCK_PIN 13
#define MOSI_PIN 11
#define MISO_PIN 12
#endif

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
  radio.setChannel(108); //108 - 2.508 Ghz //0-124 2.4gHz-2.5gHz
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.openWritingPipe(pipe);
  Serial.println(" Ok!");

  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
}

void loop() {
  dado_controle.X1 = analogRead(A0);
  dado_controle.Y1 = analogRead(A1);
  dado_controle.botao1 = !digitalRead(2);

  dado_controle.X2 = analogRead(A2);
  dado_controle.Y2 = analogRead(A3);
  dado_controle.botao2 = !digitalRead(3);

  Serial.print(millis()); Serial.print(" ms: ");
   
  Serial.print("X1: "); Serial.print(dado_controle.X1); Serial.print("\t");
  Serial.print("Y1: "); Serial.print(dado_controle.Y1); Serial.print("\t");
  Serial.print("botao1: "); Serial.print(dado_controle.botao1); Serial.print("\t");
   
  Serial.print("X2: "); Serial.print(dado_controle.X2); Serial.print("\t");
  Serial.print("Y2: "); Serial.print(dado_controle.Y2); Serial.print("\t");
  Serial.print("botao2: "); Serial.print(dado_controle.botao2); Serial.print("\t");  

  Serial.println();
  
  radio.write(&dado_controle, sizeof(dado_controle));
}

