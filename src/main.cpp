#include <ThingerESP32.h>
#include <WiFi.h>
#include <Arduino.h>
#include <DHT.h>

#define potenciometro 34    // Pino do potenciômetro
#define pinoDHT 17          // Pino do sensor DHT22
#define tipoDHT DHT22

DHT dht(pinoDHT, tipoDHT);

// Configurações do Thinger.io
#define USERNAME "Kaique"          // Substitua pelo seu nome de usuário no Thinger.io
#define DEVICE_ID "esp32-device"        // Device ID configurado no Thinger.io
#define DEVICE_CREDENTIAL "esp32password" // Credencial configurada no Thinger.io

// Configurações do Wi-Fi
#define SSID "Wokwi-GUEST"              // Rede Wi-Fi 
#define SSID_PASSWORD ""

float consumoEnergia = 0;
unsigned long ultimoTempo = 0;
float consumoTaxa;           
const float limiteEnergia = 200.0;
float temp, umid;

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

void setup() {
  dht.begin();

  Serial.begin(115200);
  thing.add_wifi(SSID, SSID_PASSWORD);

  pinMode(potenciometro, INPUT);
  pinMode(pinoDHT, OUTPUT);
  digitalWrite(pinoDHT, LOW);
  ultimoTempo = millis();

  // Conexão Wi-Fi
  Serial.print("Conectando à rede Wi-Fi");
  WiFi.begin(SSID, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado à rede Wi-Fi!");

  // Thinger.io configuração
  thing["Consumo de Energia"] >> outputValue(consumoEnergia);
  thing["Temperatura"] >> outputValue(temp);
  thing["Umidade"] >> outputValue(umid);
}

void leituraConsumo() {
  int valorPotenciometro = analogRead(potenciometro);

  if (valorPotenciometro == 0) {
    Serial.println("Erro: leitura do potenciômetro é zero. Verifique as conexões.");
    return;
  }

  float maxValorPot = 4095.0;
  float maxConsumo = 4095.0;
  float percentual = valorPotenciometro / maxValorPot; 
  consumoTaxa = percentual * maxConsumo;

  unsigned long tempoAtual = millis();
  float tempoDecorrido = (tempoAtual - ultimoTempo) / 1000.0;  

  consumoEnergia += consumoTaxa * tempoDecorrido;

  ultimoTempo = tempoAtual;

  Serial.print("Valor do Potenciômetro: ");
  Serial.print(valorPotenciometro);
  Serial.print(" | Taxa de Consumo: ");
  Serial.print(consumoTaxa, 1);
  Serial.print(" kWh/s | Energia Consumida: ");
  Serial.print(consumoEnergia, 1); 
  Serial.println(" kWh");

  thing["Consumo de Energia"] >> outputValue(consumoEnergia);
}

void leituraSensor() {
  temp = dht.readTemperature();
  umid = dht.readHumidity();

  Serial.print("Temperatura: ");
  Serial.print(temp);
  Serial.print(" graus - Umidade: ");
  Serial.print(umid);
  Serial.println(" %");

  // Atualizar Thinger.io
  thing["Temperatura"] >> outputValue(temp);
  thing["Umidade"] >> outputValue(umid);
}

void loop() {
  leituraConsumo();
  leituraSensor();
  thing.handle();
}
