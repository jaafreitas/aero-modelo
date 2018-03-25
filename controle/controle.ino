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

#include <RF24.h>
#include <TimeLib.h>

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

const uint64_t RFControle = 0xF0F0F0F0CCLL;
const uint64_t RFAeromodelo = 0xF0F0F0F0AALL;

RF24 radio(CE_PIN, CSN_PIN);

struct dado_controle {
  unsigned long id;
  int X1;
  int Y1;
  bool botao1;
  int X2;
  int Y2;
  bool botao2;
} dado_controle;

struct dado_aeromodelo {
  unsigned long id;
  time_t horario;
  float latitude;
  float longitude;
  float altitude;
  float velocidade;
  int satelites;
  
} dado_aeromodelo;

void setup() {
  Serial.begin(115200);
  Serial.println();  
  Serial.print("Iniciando radio...");
  radio.begin();
  radio.setAutoAck(false);
  radio.setChannel(108);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.openWritingPipe(RFControle);
  radio.openReadingPipe(1, RFAeromodelo);  
  Serial.println(" Ok!");

  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
}

void loop() {
  dado_controle.id++;
  dado_controle.X1 = analogRead(A0);
  dado_controle.Y1 = analogRead(A1);
  dado_controle.botao1 = !digitalRead(2);

  dado_controle.X2 = analogRead(A2);
  dado_controle.Y2 = analogRead(A3);
  dado_controle.botao2 = !digitalRead(3);
  
  radio.stopListening();
  radio.write(&dado_controle, sizeof(dado_controle));
  
  radio.startListening();
  if (radio.available()) {
    radio.read(&dado_aeromodelo, sizeof(dado_aeromodelo));
    setTime(dado_aeromodelo.horario);
  }
  
  time_t t = now();
  static char isotime[30];
  sprintf(isotime, "%4d-%02d-%02dT%02d:%02d:%02d+00:00", year(t), month(t), day(t), hour(t), minute(t), second(t));
  Serial.print(isotime); Serial.print(" ");

  Serial.print("aeromodelo id: "); Serial.print(dado_aeromodelo.id); Serial.print("\t");
  
  Serial.print("pos: ");
  Serial.print(dado_aeromodelo.latitude, 6);
  Serial.print(", ");
  Serial.print(dado_aeromodelo.longitude, 6);
  Serial.print("\t");        

  Serial.print("altitude: "); Serial.print(dado_aeromodelo.altitude); Serial.print("\t");
  Serial.print("velocidade: "); Serial.print(dado_aeromodelo.velocidade); Serial.print("\t");
  Serial.print("satelites: "); Serial.print(dado_aeromodelo.satelites); Serial.print("\t");
 
  Serial.print("controel id: "); Serial.print(dado_controle.id); Serial.print("\t");

  Serial.print("X1: "); Serial.print(dado_controle.X1); Serial.print("\t");
  Serial.print("Y1: "); Serial.print(dado_controle.Y1); Serial.print("\t");
  Serial.print("botao1: "); Serial.print(dado_controle.botao1); Serial.print("\t");
 
  Serial.print("X2: "); Serial.print(dado_controle.X2); Serial.print("\t");
  Serial.print("Y2: "); Serial.print(dado_controle.Y2); Serial.print("\t");
  Serial.print("botao2: "); Serial.print(dado_controle.botao2); Serial.print("\t");

  Serial.println();  
}

