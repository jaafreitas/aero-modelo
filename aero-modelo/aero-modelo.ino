/*
  
  nRF24L01: http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
  VCC 3.3V !!! NOT 5V

*/

#include <RF24.h>
#include <Servo.h>
#include <NeoSWSerial.h>
#include <TimeLib.h>
#include "src/TinyGPS/TinyGPS.h"

#define ESC_PIN 4                // joystick esquerdo, cima / baixo
#define SERVO_LEME_PIN 5         // joystick esquerda, direita/esquerda
#define SERVO_CALDA_PIN 6        // joystick direita, cima/baixo
#define SERVO_ASA_ESQUERDA_PIN 7 // joystick direito, esquerda/direita
#define SERVO_ASA_DIREITA_PIN 8  // joystick direito, !esquerda/direita

#define RF_CE_PIN     9
#define RF_CSN_PIN   10
#define RF_MOSI_PIN  11
#define RF_MISO_PIN  12
#define RF_SCK_PIN   13

#define GPS_TX_PIN A0
#define GPS_RX_PIN A1

const int ESC_MIN_SIGNAL = 700;
const int ESC_START_SIGNAL = 745;
const int ESC_MAX_SIGNAL = 2000;
const int ESC_MAX_SIGNAL_ALLOWED = ESC_MAX_SIGNAL;
//const int ESC_MAX_SIGNAL_ALLOWED = (ESC_START_SIGNAL + 0.10 * (ESC_MAX_SIGNAL - ESC_START_SIGNAL));

const uint64_t RFControle = 0xF0F0F0F0CCLL;
const uint64_t RFAeromodelo = 0xF0F0F0F0AALL;

unsigned long ultimo_dado_controle_recebido;
// Tempo em milisegundos para considerarmos que perdemos a comunicação com o controle.
const unsigned int timeout_dado_controle = 1000;

RF24 radio(RF_CE_PIN, RF_CSN_PIN);
Servo esc;
Servo servo_leme;
Servo servo_calda;
Servo servo_asa_esquerda;
Servo servo_asa_direita;
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

void debug(bool motorOn, int mappedValue, int signal) {
  Serial.print(" motor: "); Serial.print(motorOn ? "on" : "off");
  Serial.print(" | mapped Value: "); Serial.print(mappedValue);
  Serial.print(" | signal: "); Serial.print(signal);

  int potencia = max((float)(signal - ESC_START_SIGNAL) / (ESC_MAX_SIGNAL - ESC_START_SIGNAL) * 100, 0);
  Serial.print(" | potência: "); Serial.print(potencia);  Serial.print("\t");
}

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

  esc.attach(ESC_PIN);
  servo_leme.attach(SERVO_LEME_PIN);
  servo_calda.attach(SERVO_CALDA_PIN);
  servo_asa_esquerda.attach(SERVO_ASA_ESQUERDA_PIN);
  servo_asa_direita.attach(SERVO_ASA_DIREITA_PIN);

  ultimo_dado_controle_recebido = millis() - timeout_dado_controle;

  // Rotina de calibração do motor. Habilitar somente quando for necessário
  // uma nova calibração.
//  if (digitalRead(3)) {
//    Serial.println("Ajustando o sinal máximo...");
//    debug(false, 0, ESC_MAX_SIGNAL);
//    esc.writeMicroseconds(ESC_MAX_SIGNAL);
//    delay(5000);
//
//    Serial.println("Ajustando o sinal mínimo...");
//    debug(false, 0, ESC_MIN_SIGNAL);
//    esc.writeMicroseconds(ESC_MIN_SIGNAL);
//    delay(2000);
//  }
}

void loop() {
  static bool motorOn = false;

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
    ultimo_dado_controle_recebido = millis();
    radio.read(&dado_controle, sizeof(dado_controle));

    int mappedValue = map(dado_controle.Y1, 0, 1023, ESC_MIN_SIGNAL, ESC_MAX_SIGNAL_ALLOWED);
    int signal;

    // Para ligar o motor é preciso segurar o botão 1 e 2 apertado ao mesmo tempo.
    if (dado_controle.botao1 && dado_controle.botao2) {
      motorOn = true;
      // Iniciando com 10% da potência.
      signal = ESC_START_SIGNAL + 0.05 * (ESC_MAX_SIGNAL - ESC_START_SIGNAL);
    }
    // Só permite controlar o motor após o acionamento.
    else if (((mappedValue < ESC_START_SIGNAL) && motorOn) || !motorOn)  {
      motorOn = false;
      signal = ESC_MIN_SIGNAL;
    }
    else {
      signal = mappedValue;
    }
    
    debug(motorOn, mappedValue, signal);
    esc.writeMicroseconds(signal);    
    
    servo_leme.write(map(dado_controle.X1, 0, 1023, 120, 60));
    servo_calda.write(map(dado_controle.Y2, 0, 1023, 130, 50));
    servo_asa_esquerda.write(map(dado_controle.X2, 0, 1023, 140, 60));
    servo_asa_direita.write(map(dado_controle.X2, 0, 1023, 140, 60));    
  }
  
  // Perdemos a comunicação com o radio?
  if (millis() - ultimo_dado_controle_recebido > timeout_dado_controle) {
    esc.writeMicroseconds(ESC_MIN_SIGNAL);    
    motorOn = false;
    Serial.print(F("SEM COMUNICAÇÃO\t"));
    debug(motorOn, ESC_MIN_SIGNAL, ESC_MIN_SIGNAL);
  }
  
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

