#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// =============================
// CONFIGURACAO WIFI
// =============================
const char* WIFI_SSID = "WalbertNet2.4";
const char* WIFI_PASSWORD = "12345678@";

// =============================
// CONFIGURACAO WEBSOCKET
// =============================
WebSocketsServer webSocket = WebSocketsServer(81);

// =============================
// PINO DO MOTOR
// =============================
const int MOTOR_PIN = 18;   // ajuste conforme sua montagem
const bool MOTOR_ACTIVE_HIGH = true;

// =============================
// ESTADO DA VIBRACAO (NAO BLOQUEANTE)
// =============================
bool isPlaying = false;
unsigned long vibrationStart = 0;
unsigned long vibrationDuration = 0;
String currentDim = "duration";
int currentValue = 0;
int currentLevel = 0;
String currentPoint = "";
float currentRaw = 0.0f;

// =============================
// UTILITARIOS DO MOTOR
// =============================
void motorOn() {
  digitalWrite(MOTOR_PIN, MOTOR_ACTIVE_HIGH ? HIGH : LOW);
}

void motorOff() {
  digitalWrite(MOTOR_PIN, MOTOR_ACTIVE_HIGH ? LOW : HIGH);
}

void stopVibration() {
  motorOff();
  isPlaying = false;
  vibrationStart = 0;
  vibrationDuration = 0;
}

void startDurationVibration(unsigned long durationMs) {
  stopVibration();
  motorOn();
  vibrationStart = millis();
  vibrationDuration = durationMs;
  isPlaying = true;
}

void updateVibration() {
  if (isPlaying && (millis() - vibrationStart >= vibrationDuration)) {
    stopVibration();
  }
}

// =============================
// JSON / RESPOSTAS
// =============================
template <size_t N>
void sendJson(uint8_t clientNum, StaticJsonDocument<N>& doc) {
  String out;
  serializeJson(doc, out);
  webSocket.sendTXT(clientNum, out);
}

void sendStatus(uint8_t clientNum) {
  StaticJsonDocument<256> doc;
  doc["status"] = "ok";
  doc["cmd"] = "status";
  doc["ip"] = WiFi.localIP().toString();
  doc["ws_port"] = 81;
  doc["isPlaying"] = isPlaying;
  doc["dim"] = currentDim;
  doc["value"] = currentValue;
  doc["level"] = currentLevel;
  doc["point"] = currentPoint;
  doc["raw"] = currentRaw;
  sendJson(clientNum, doc);
}

void sendAck(uint8_t clientNum, const char* cmdName) {
  StaticJsonDocument<256> doc;
  doc["status"] = "ok";
  doc["cmd"] = cmdName;
  doc["isPlaying"] = isPlaying;
  doc["dim"] = currentDim;
  doc["value"] = currentValue;
  doc["level"] = currentLevel;
  doc["point"] = currentPoint;
  doc["raw"] = currentRaw;
  sendJson(clientNum, doc);
}

void sendError(uint8_t clientNum, const char* message) {
  StaticJsonDocument<256> doc;
  doc["status"] = "error";
  doc["message"] = message;
  sendJson(clientNum, doc);
}

// =============================
// COMANDOS
// =============================
void handlePlayCommand(uint8_t clientNum, JsonDocument& doc) {
  const char* dim = doc["dim"] | "duration";
  int value = doc["value"] | 0;
  int level = doc["level"] | 0;
  const char* point = doc["point"] | "";
  float raw = doc["raw"] | 0.0;

  if (strcmp(dim, "duration") != 0) {
    sendError(clientNum, "Only 'duration' is supported in this Sprint 4 renderer.");
    return;
  }

  if (value <= 0) {
    sendError(clientNum, "Invalid 'value'. Expected duration in milliseconds > 0.");
    return;
  }

  // Limite de seguranca basico para evitar vibracoes longas demais por engano.
  if (value > 10000) {
    sendError(clientNum, "Value too large. Max allowed: 10000 ms.");
    return;
  }

  currentDim = dim;
  currentValue = value;
  currentLevel = level;
  currentPoint = point;
  currentRaw = raw;

  startDurationVibration((unsigned long)value);
  sendAck(clientNum, "play");

  Serial.print("[PLAY] dim=");
  Serial.print(currentDim);
  Serial.print(" value=");
  Serial.print(currentValue);
  Serial.print(" level=");
  Serial.print(currentLevel);
  Serial.print(" point=");
  Serial.print(currentPoint);
  Serial.print(" raw=");
  Serial.println(currentRaw, 3);
}

void handleMessage(uint8_t clientNum, uint8_t* payload, size_t length) {
  StaticJsonDocument<384> doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    sendError(clientNum, "Invalid JSON payload.");
    Serial.print("[JSON ERROR] ");
    Serial.println(error.c_str());
    return;
  }

  const char* cmd = doc["cmd"] | "";

  if (strcmp(cmd, "play") == 0) {
    handlePlayCommand(clientNum, doc);
  } else if (strcmp(cmd, "stop") == 0) {
    stopVibration();
    currentValue = 0;
    currentLevel = 0;
    currentPoint = "";
    currentRaw = 0.0f;
    sendAck(clientNum, "stop");
    Serial.println("[STOP]");
  } else if (strcmp(cmd, "ping") == 0) {
    StaticJsonDocument<128> pong;
    pong["status"] = "ok";
    pong["cmd"] = "pong";
    sendJson(clientNum, pong);
    Serial.println("[PING] pong enviado");
  } else if (strcmp(cmd, "status") == 0) {
    sendStatus(clientNum);
    Serial.println("[STATUS]");
  } else {
    sendError(clientNum, "Unknown command.");
  }
}

// =============================
// EVENTOS WEBSOCKET
// =============================
void onWebSocketEvent(uint8_t clientNum, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WS] Cliente %u desconectado\n", clientNum);
      break;

    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(clientNum);
      Serial.printf("[WS] Cliente %u conectado de %d.%d.%d.%d\n", clientNum, ip[0], ip[1], ip[2], ip[3]);
      StaticJsonDocument<192> hello;
      hello["status"] = "ok";
      hello["cmd"] = "hello";
      hello["message"] = "ESP32 WebSocket ready";
      hello["ip"] = WiFi.localIP().toString();
      sendJson(clientNum, hello);
      break;
    }

    case WStype_TEXT:
      handleMessage(clientNum, payload, length);
      break;

    default:
      break;
  }
}

// =============================
// WIFI
// =============================
void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Conectando ao Wi-Fi");
  unsigned long startAttempt = millis();
  const unsigned long timeoutMs = 20000;

  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < timeoutMs) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi conectado.");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Falha ao conectar no Wi-Fi.");
    Serial.println("Revise SSID/senha ou aproxime o ESP32 do roteador.");
  }
}

// =============================
// SETUP / LOOP
// =============================
void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(MOTOR_PIN, OUTPUT);
  motorOff();

  Serial.println("\n=== ESP32 Haptic Sprint 4 - Generic Renderer ===");
  connectToWiFi();

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  Serial.println("WebSocket iniciado na porta 81.");
  Serial.println("Pronto para receber JSON com cmd=play/stop/ping/status.");
}

void loop() {
  webSocket.loop();
  updateVibration();
}
