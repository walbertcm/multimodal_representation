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
// CONFIGURACAO DOS MOTORES
// Ajuste os pinos conforme a sua montagem real.
// =============================
const bool MOTOR_ACTIVE_HIGH = true;
const uint8_t NUM_MOTORS = 8;
const int MOTOR_PINS[NUM_MOTORS] = {14, 27, 26, 25, 33, 32, 4, 16};

// =============================
// ESTADO DA VIBRACAO (NAO BLOQUEANTE)
// Um estado por motor
// =============================
bool isPlaying[NUM_MOTORS] = {false};
unsigned long vibrationStart[NUM_MOTORS] = {0};
unsigned long vibrationDuration[NUM_MOTORS] = {0};
String currentDim[NUM_MOTORS];
int currentValue[NUM_MOTORS] = {0};
int currentLevel[NUM_MOTORS] = {0};
String currentPoint[NUM_MOTORS];
float currentRaw[NUM_MOTORS] = {0.0f};

// =============================
// UTILITARIOS DOS MOTORES
// =============================
bool isValidMotorIndex(int motorIndex) {
  return motorIndex >= 0 && motorIndex < NUM_MOTORS;
}

void motorOn(int motorIndex) {
  if (!isValidMotorIndex(motorIndex)) return;
  digitalWrite(MOTOR_PINS[motorIndex], MOTOR_ACTIVE_HIGH ? HIGH : LOW);
}

void motorOff(int motorIndex) {
  if (!isValidMotorIndex(motorIndex)) return;
  digitalWrite(MOTOR_PINS[motorIndex], MOTOR_ACTIVE_HIGH ? LOW : HIGH);
}

void clearMotorMetadata(int motorIndex) {
  if (!isValidMotorIndex(motorIndex)) return;
  currentValue[motorIndex] = 0;
  currentLevel[motorIndex] = 0;
  currentPoint[motorIndex] = "";
  currentRaw[motorIndex] = 0.0f;
  currentDim[motorIndex] = "duration";
}

void stopVibration(int motorIndex, bool clearMetadata = false) {
  if (!isValidMotorIndex(motorIndex)) return;
  motorOff(motorIndex);
  isPlaying[motorIndex] = false;
  vibrationStart[motorIndex] = 0;
  vibrationDuration[motorIndex] = 0;
  if (clearMetadata) {
    clearMotorMetadata(motorIndex);
  }
}

void stopAllVibrations(bool clearMetadata = false) {
  for (int i = 0; i < NUM_MOTORS; i++) {
    stopVibration(i, clearMetadata);
  }
}

void startDurationVibration(int motorIndex, unsigned long durationMs) {
  if (!isValidMotorIndex(motorIndex)) return;
  stopVibration(motorIndex, false);
  motorOn(motorIndex);
  vibrationStart[motorIndex] = millis();
  vibrationDuration[motorIndex] = durationMs;
  isPlaying[motorIndex] = true;
}

void updateVibration() {
  unsigned long now = millis();
  for (int i = 0; i < NUM_MOTORS; i++) {
    if (isPlaying[i] && (now - vibrationStart[i] >= vibrationDuration[i])) {
      stopVibration(i, false);
    }
  }
}

int countPlayingMotors() {
  int total = 0;
  for (int i = 0; i < NUM_MOTORS; i++) {
    if (isPlaying[i]) total++;
  }
  return total;
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

void appendMotorStatus(JsonArray motorsArray) {
  for (int i = 0; i < NUM_MOTORS; i++) {
    JsonObject motor = motorsArray.createNestedObject();
    motor["motor"] = i;
    motor["pin"] = MOTOR_PINS[i];
    motor["isPlaying"] = isPlaying[i];
    motor["dim"] = currentDim[i];
    motor["value"] = currentValue[i];
    motor["level"] = currentLevel[i];
    motor["point"] = currentPoint[i];
    motor["raw"] = currentRaw[i];
  }
}

void sendStatus(uint8_t clientNum) {
  StaticJsonDocument<1024> doc;
  doc["status"] = "ok";
  doc["cmd"] = "status";
  doc["ip"] = WiFi.localIP().toString();
  doc["ws_port"] = 81;
  doc["activeMotors"] = countPlayingMotors();
  JsonArray motors = doc.createNestedArray("motors");
  appendMotorStatus(motors);
  sendJson(clientNum, doc);
}

void sendAck(uint8_t clientNum, const char* cmdName, int motorIndex = -1) {
  StaticJsonDocument<512> doc;
  doc["status"] = "ok";
  doc["cmd"] = cmdName;
  doc["activeMotors"] = countPlayingMotors();

  if (isValidMotorIndex(motorIndex)) {
    doc["motor"] = motorIndex;
    doc["pin"] = MOTOR_PINS[motorIndex];
    doc["isPlaying"] = isPlaying[motorIndex];
    doc["dim"] = currentDim[motorIndex];
    doc["value"] = currentValue[motorIndex];
    doc["level"] = currentLevel[motorIndex];
    doc["point"] = currentPoint[motorIndex];
    doc["raw"] = currentRaw[motorIndex];
  }

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
// JSON aceito para play:
// {"cmd":"play","motor":0,"dim":"duration","value":300,"level":2,"point":"PA","raw":78.5}
// JSON aceito para stop:
// {"cmd":"stop","motor":0}  -> para um motor
// {"cmd":"stop","motor":-1} -> para todos
// {"cmd":"stop"}             -> para todos
// =============================
void handlePlayCommand(uint8_t clientNum, JsonDocument& doc) {
  const char* dim = doc["dim"] | "duration";
  int motorIndex = doc["motor"] | 0;
  int value = doc["value"] | 0;
  int level = doc["level"] | 0;
  const char* point = doc["point"] | "";
  float raw = doc["raw"] | 0.0;

  if (!isValidMotorIndex(motorIndex)) {
    sendError(clientNum, "Invalid 'motor'. Expected integer between 0 and 7.");
    return;
  }

  if (strcmp(dim, "duration") != 0) {
    sendError(clientNum, "Only 'duration' is supported in this Sprint 4 renderer.");
    return;
  }

  if (value <= 0) {
    sendError(clientNum, "Invalid 'value'. Expected duration in milliseconds > 0.");
    return;
  }

  if (value > 10000) {
    sendError(clientNum, "Value too large. Max allowed: 10000 ms.");
    return;
  }

  currentDim[motorIndex] = dim;
  currentValue[motorIndex] = value;
  currentLevel[motorIndex] = level;
  currentPoint[motorIndex] = point;
  currentRaw[motorIndex] = raw;

  startDurationVibration(motorIndex, (unsigned long)value);
  sendAck(clientNum, "play", motorIndex);

  Serial.print("[PLAY] motor=");
  Serial.print(motorIndex);
  Serial.print(" pin=");
  Serial.print(MOTOR_PINS[motorIndex]);
  Serial.print(" dim=");
  Serial.print(currentDim[motorIndex]);
  Serial.print(" value=");
  Serial.print(currentValue[motorIndex]);
  Serial.print(" level=");
  Serial.print(currentLevel[motorIndex]);
  Serial.print(" point=");
  Serial.print(currentPoint[motorIndex]);
  Serial.print(" raw=");
  Serial.println(currentRaw[motorIndex], 3);
}

void handleStopCommand(uint8_t clientNum, JsonDocument& doc) {
  if (!doc.containsKey("motor")) {
    stopAllVibrations(true);
    sendAck(clientNum, "stop");
    Serial.println("[STOP] all motors");
    return;
  }

  int motorIndex = doc["motor"] | -1;
  if (motorIndex == -1) {
    stopAllVibrations(true);
    sendAck(clientNum, "stop");
    Serial.println("[STOP] all motors");
    return;
  }

  if (!isValidMotorIndex(motorIndex)) {
    sendError(clientNum, "Invalid 'motor'. Expected integer between 0 and 7, or -1 for all.");
    return;
  }

  stopVibration(motorIndex, true);
  sendAck(clientNum, "stop", motorIndex);
  Serial.print("[STOP] motor=");
  Serial.println(motorIndex);
}

void handleMessage(uint8_t clientNum, uint8_t* payload, size_t length) {
  StaticJsonDocument<768> doc;
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
    handleStopCommand(clientNum, doc);
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
      StaticJsonDocument<256> hello;
      hello["status"] = "ok";
      hello["cmd"] = "hello";
      hello["message"] = "ESP32 WebSocket ready";
      hello["ip"] = WiFi.localIP().toString();
      hello["motors"] = NUM_MOTORS;
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

  for (int i = 0; i < NUM_MOTORS; i++) {
    pinMode(MOTOR_PINS[i], OUTPUT);
    motorOff(i);
    clearMotorMetadata(i);
  }

  Serial.println("\n=== ESP32 Haptic Sprint 4 - Generic Renderer (Multi-Motor) ===");
  Serial.println("Motores configurados:");
  for (int i = 0; i < NUM_MOTORS; i++) {
    Serial.print("  motor ");
    Serial.print(i);
    Serial.print(" -> GPIO ");
    Serial.println(MOTOR_PINS[i]);
  }

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
