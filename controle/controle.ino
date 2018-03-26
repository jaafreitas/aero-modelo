/*

  nRF24L01: http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
  VCC 3.3V !!! NOT 5V

*/

#include <RF24.h>
#include <TimeLib.h>

//#define DISPLAY_SSD1306
//#define DISPLAY_LCD16x2

#define CE_PIN 9

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
#define MOSI_PIN 11
#define MISO_PIN 12
#define SCK_PIN 13
#endif

#define JOYX1_PIN A0
#define JOYY1_PIN A1
#define JOYBUTTON1_PIN 2
#define JOYX2_PIN A2
#define JOYY2_PIN A3
#define JOYBUTTON2_PIN 3

const uint64_t RFControle = 0xF0F0F0F0CCLL;
const uint64_t RFAeromodelo = 0xF0F0F0F0AALL;

#ifdef DISPLAY_SSD1306
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#endif

#ifdef DISPLAY_LCD16x2
#include <Adafruit_LiquidCrystal.h>
Adafruit_LiquidCrystal display(0x20);
#endif

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
  Serial.print(F("Iniciando radio..."));
  radio.begin();
  radio.setAutoAck(false);
  radio.setChannel(108);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.openWritingPipe(RFControle);
  radio.openReadingPipe(1, RFAeromodelo);  
  Serial.println(F(" Ok!"));

  pinMode(JOYBUTTON1_PIN, INPUT_PULLUP);
  pinMode(JOYBUTTON2_PIN, INPUT_PULLUP);

#ifdef DISPLAY_SSD1306
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
#endif
#ifdef DISPLAY_LCD16x2
  display.begin (16, 2);
#endif
}

void displayLCD() {  
  static unsigned long tempoAnterior = 0;
  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnterior >= 1000) {

#ifdef DISPLAY_LCD16x2
    display.clear();
    display.setCursor(0,0);
    display.print(dado_aeromodelo.latitude, 6); display.print(" ");
    char displayalt[5];
    sprintf(displayalt, "%5d", (int)dado_aeromodelo.altitude);
    display.print(displayalt);

    display.setCursor(0,1);
    display.print(dado_aeromodelo.longitude, 6); display.print(" ");
    char displayvel[5];
    sprintf(displayvel, "%5d", (int)dado_aeromodelo.velocidade);
    display.print(displayvel);
#endif
    
#ifdef DISPLAY_SSD1306
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0,0);
  
    display.setTextSize(1);
    char displaytime[24];
    time_t t = now();
    sprintf(displaytime, "%02d/%02d/%4d %02d:%02d:%02d", day(t), month(t), year(t), hour(t), minute(t), second(t));  
    display.println(displaytime);
    
    display.setTextSize(2);
    display.print(F("alt ")); display.println(dado_aeromodelo.altitude);
    display.print(F("vel ")); display.println(dado_aeromodelo.velocidade);
  
    display.setTextSize(1);
    display.print(dado_aeromodelo.latitude, 6);
    display.print(F(","));
    display.print(dado_aeromodelo.longitude, 6);
  
    display.print(F("id ")); display.print(dado_aeromodelo.id); 
    display.print(F(" | sat ")); display.println(dado_aeromodelo.satelites);
    
    display.display();
#endif
    
    tempoAnterior = tempoAtual;
  }  
}

void loop() {
  dado_controle.id++;
  dado_controle.X1 = analogRead(JOYX1_PIN);
  dado_controle.Y1 = analogRead(JOYY1_PIN);
  dado_controle.botao1 = !digitalRead(JOYBUTTON1_PIN);

  dado_controle.X2 = analogRead(JOYX2_PIN);
  dado_controle.Y2 = analogRead(JOYY2_PIN);
  dado_controle.botao2 = !digitalRead(JOYBUTTON2_PIN);
  
  radio.stopListening();
  radio.write(&dado_controle, sizeof(dado_controle));
  
  radio.startListening();
  if (radio.available()) {
    radio.read(&dado_aeromodelo, sizeof(dado_aeromodelo));
    setTime(dado_aeromodelo.horario);
  }

  displayLCD();
  
  Serial.print(millis()); Serial.print(" ");

  Serial.print(F("aeromodelo id: ")); Serial.print(dado_aeromodelo.id); Serial.print("\t");
  
  Serial.print(F("pos: "));
  Serial.print(dado_aeromodelo.latitude, 6);
  Serial.print(F(", "));
  Serial.print(dado_aeromodelo.longitude, 6);
  Serial.print("\t");

  Serial.print(F("altitude: ")); Serial.print(dado_aeromodelo.altitude); Serial.print("\t");
  Serial.print(F("velocidade: ")); Serial.print(dado_aeromodelo.velocidade); Serial.print("\t");
  Serial.print(F("satelites: ")); Serial.print(dado_aeromodelo.satelites); Serial.print("\t");
 
  Serial.print(F("controle id: ")); Serial.print(dado_controle.id); Serial.print("\t");

  Serial.print(F("X1: ")); Serial.print(dado_controle.X1); Serial.print("\t");
  Serial.print(F("Y1: ")); Serial.print(dado_controle.Y1); Serial.print("\t");
  Serial.print(F("botao1: ")); Serial.print(dado_controle.botao1); Serial.print("\t");
 
  Serial.print(F("X2: ")); Serial.print(dado_controle.X2); Serial.print("\t");
  Serial.print(F("Y2: ")); Serial.print(dado_controle.Y2); Serial.print("\t");
  Serial.print(F("botao2: ")); Serial.print(dado_controle.botao2); Serial.print("\t");

  Serial.println();
}

