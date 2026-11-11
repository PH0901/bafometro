// Bafômetro Inteligente - ESP32 + MQ-3 (sensor de gás do Wokwi) + LED + buzzer + MQTT
#include <WiFi.h>
#include <PubSubClient.h>

// ---------- Pinos ----------
const int PIN_MQ3    = 34;  // AOUT do sensor (ADC1)
const int PIN_LED    = 25;  // LED vermelho (via resistor de 220 ohms)
const int PIN_BUZZER = 26;  // Buzzer

// ---------- Parâmetros ----------
const int           LIMIAR        = 2000;   // 0-4095 (ADC 12 bits). Ajuste na calibração.
const int           N_AMOSTRAS    = 8;      // média para reduzir ruído
const unsigned long PREAQUECER_MS = 5000;   // simulação; no sensor real, alguns minutos
const unsigned long INTERVALO_MS  = 1000;   // período de leitura e publicação

// ---------- Wi-Fi / MQTT ----------
const char* WIFI_SSID = "Wokwi-GUEST";       // rede de simulação do Wokwi (sem senha)
const char* WIFI_PASS = "";
const char* MQTT_HOST = "broker.hivemq.com";
const int   MQTT_PORT = 1883;
const char* MQTT_TOPIC = "transito/bafometro/leitura";
const char* DEVICE_ID  = "bafometro01";

WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long ultimaLeitura = 0;

void conectarWiFi() {
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(250); Serial.print("."); }
  Serial.println(" conectado");
}

void conectarMQTT() {
  if (mqtt.connected()) return;
  String clientId = String(DEVICE_ID) + "-" + String(random(0xffff), HEX);
  if (mqtt.connect(clientId.c_str())) Serial.println("MQTT conectado");
  else { Serial.print("Falha MQTT, rc="); Serial.println(mqtt.state()); }
}

int lerSensor() {
  long soma = 0;
  for (int i = 0; i < N_AMOSTRAS; i++) { soma += analogRead(PIN_MQ3); delay(5); }
  return soma / N_AMOSTRAS;
}

void atuadores(bool alerta) {
  digitalWrite(PIN_LED, alerta ? HIGH : LOW);
  // Buzzer ativo físico: digitalWrite(PIN_BUZZER, HIGH). No Wokwi o buzzer
  // só soa com tom, por isso usamos tone() (~2,5 kHz, como o KY-012).
  if (alerta) tone(PIN_BUZZER, 2500); else noTone(PIN_BUZZER);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  analogReadResolution(12);
  atuadores(false);

  conectarWiFi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);

  Serial.println("Pré-aquecendo o sensor...");
  delay(PREAQUECER_MS);
  Serial.println("Pronto.");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) conectarWiFi();
  conectarMQTT();
  mqtt.loop();

  if (millis() - ultimaLeitura >= INTERVALO_MS) {
    ultimaLeitura = millis();

    int valor = lerSensor();
    bool alerta = valor > LIMIAR;
    atuadores(alerta);

    char payload[96];
    snprintf(payload, sizeof(payload),
             "{\"id\":\"%s\",\"valor\":%d,\"alerta\":%s}",
             DEVICE_ID, valor, alerta ? "true" : "false");

    if (mqtt.connected()) mqtt.publish(MQTT_TOPIC, payload);
    Serial.println(payload);
  }
}
