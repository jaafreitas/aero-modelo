/*
  
  nRF24L01: http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
  VCC 3.3V !!! NOT 5V

*/

#include <RF24.h>
#include <Servo.h>
#include <NeoSWSerial.h>
#include <TimeLib.h>
#include "src/TinyGPS/TinyGPS.h"

#define SERVO1_PIN 5
#define SERVO2_PIN 6
#define SERVO3_PIN 7
#define SERVO4_PIN 8

#define CE_PIN     9
#define CSN_PIN   10
#define MOSI_PIN  11
#define MISO_PIN  12
#define SCK_PIN   13

#define GPS_TX_PIN A0
#define GPS_RX_PIN A1

const uint64_t RFControle = 0xF0F0F0F0CCLL;
const uint64_t RFAeromodelo = 0xF0F0F0F0AALL;

RF24 radio(CE_PIN, CSN_PIN);
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
NeoSWSerial gps_serial(GPS_TX_PIN, GPS_RX_PIN);
TinyGPS gps;

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
  radio.openReadingPipe(1, RFControle);
  radio.openWritingPipe(RFAeromodelo);  
  Serial.println(" Ok!");

  gps_serial.begin(9600);

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo3.attach(SERVO3_PIN);
  servo4.attach(SERVO4_PIN);
}

void loop() {
  while (gps_serial.available()) {
    if (gps.encode(gps_serial.read())) {
      dado_aeromodelo.id++;
      
      unsigned long age;
      gps.f_get_position(&dado_aeromodelo.latitude, &dado_aeromodelo.longitude, &age);
      dado_aeromodelo.latitude = dado_aeromodelo.latitude == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : dado_aeromodelo.latitude; 
      dado_aeromodelo.longitude = dado_aeromodelo.longitude == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : dado_aeromodelo.longitude;

      int year;
      byte month, day, hour, minute, second, hundredths;
      gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
      setTime(hour, minute, second, day, month, year);
      dado_aeromodelo.horario = now();

      dado_aeromodelo.altitude = gps.f_altitude() == 1000000 ? 0 : gps.f_altitude();
      dado_aeromodelo.velocidade = gps.f_speed_kmph() == TinyGPS::GPS_INVALID_SPEED ? 0 : gps.f_speed_kmph();
      dado_aeromodelo.satelites = gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites();

      radio.stopListening();
      radio.write(&dado_aeromodelo, sizeof(dado_aeromodelo));
    }
  }

  radio.startListening();
  if (radio.available()) {
    radio.read(&dado_controle, sizeof(dado_controle));
    
    servo1.write(map(dado_controle.X1, 0, 1023, 0, 179));
    servo2.write(map(dado_controle.Y1, 0, 1023, 0, 179));
    servo3.write(map(dado_controle.X2, 0, 1023, 0, 179));
    servo4.write(map(dado_controle.Y2, 0, 1023, 0, 179));    
  }

  time_t t = now();
  char isotime[30];
  sprintf(isotime, "%4d-%02d-%02dT%02d:%02d:%02d+00:00", year(t), month(t), day(t), hour(t), minute(t), second(t));
  Serial.print(isotime); Serial.print(" ");

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

