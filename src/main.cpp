#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>

const char *apSSID = "ESP32AP";
const char *apPassword = "password";
bool isInAPMode = false;
WebServer server(80);

String ssid = "";
String password = "";

#define AP_MODE_INDICATOR_PIN 2
#define CLIENT_MODE_INDICATOR_PIN 4
#define RESET_PIN 5

// Definir o endereço de memória EEPROM onde as credenciais serão armazenadas
#define EEPROM_SIZE 512
#define EEPROM_ADDR_MAGIC 0
#define EEPROM_MAGIC_VALUE 0xA5

void handleRoot()
{
  server.send(200, "text/html", "<h1>Configuração de Rede WiFi</h1><form action='/configure-wifi' method='post'>SSID: <input type='text' name='ssid'><br>Senha: <input type='password' name='password'><br><input type='submit' value='Enviar'></form>");
}

void handleConfigureWiFi()
{
  if (server.method() == HTTP_POST)
  {
    ssid = server.arg("ssid");
    password = server.arg("password");
    // Salvar as credenciais na EEPROM
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC_VALUE);
    EEPROM.put(EEPROM_ADDR_MAGIC + 1, ssid);
    EEPROM.put(EEPROM_ADDR_MAGIC + 1 + sizeof(ssid), password);
    EEPROM.commit();
    EEPROM.end();
    server.send(200, "text/plain", "Configuração bem-sucedida! Reiniciando o ESP32...");
    delay(2000);
    ESP.restart();
  }
  else
  {
    server.send(405, "text/plain", "Método não permitido.");
  }
}

bool hasStoredWiFiCredentials()
{
  EEPROM.begin(EEPROM_SIZE);
  bool hasCredentials = EEPROM.read(EEPROM_ADDR_MAGIC) == EEPROM_MAGIC_VALUE;
  EEPROM.end();
  return hasCredentials;
}

bool isConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

void setupWiFiClientMode()
{
  digitalWrite(AP_MODE_INDICATOR_PIN, LOW);
  WiFi.begin(ssid.c_str(), password.c_str());
  while (!isConnected())
  {
    digitalWrite(CLIENT_MODE_INDICATOR_PIN, LOW);
    delay(1000);
    digitalWrite(CLIENT_MODE_INDICATOR_PIN, HIGH);
    delay(1000);
    Serial.println("Tentando se conectar à rede WiFi...");
  }
  Serial.println("Conectado à rede WiFi");
  digitalWrite(CLIENT_MODE_INDICATOR_PIN, HIGH);
}

void setupWiFiAPMode()
{
  isInAPMode = true;
  digitalWrite(CLIENT_MODE_INDICATOR_PIN, LOW);
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP(apSSID, apPassword);
}

void setupAPRequestHandler()
{
  server.on("/", handleRoot);
  server.on("/configure-wifi", handleConfigureWiFi);
  server.begin();
  Serial.println("Acesse http://" + WiFi.softAPIP().toString() + " para configurar a rede WiFi");
}

void resetWiFiCredentials()
{
  Serial.println("Resetando as credenciais WiFi...");
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(EEPROM_ADDR_MAGIC, 0);
  EEPROM.commit();
  EEPROM.end();
  delay(2000);
  ESP.restart();
}

void setup()
{
  pinMode(AP_MODE_INDICATOR_PIN, OUTPUT);
  pinMode(CLIENT_MODE_INDICATOR_PIN, OUTPUT);
  pinMode(RESET_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  delay(1000);

  if (hasStoredWiFiCredentials())
  {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(EEPROM_ADDR_MAGIC + 1, ssid);
    EEPROM.get(EEPROM_ADDR_MAGIC + 1 + sizeof(ssid), password);
    EEPROM.end();

    setupWiFiClientMode();
  }
  else
  {
    setupWiFiAPMode();
    setupAPRequestHandler();
  }
}

void loop()
{
  server.handleClient();

  if (isInAPMode)
  {
    digitalWrite(AP_MODE_INDICATOR_PIN, LOW);
    delay(1000);
    digitalWrite(AP_MODE_INDICATOR_PIN, HIGH);
    delay(1000);
  }

  if (isConnected())
  {
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
    delay(5000); // Aguardar 5 segundos antes de mostrar o próximo endereço IP
  }
  else if (!isInAPMode)
  {
    setupWiFiClientMode();
  }

  if (digitalRead(RESET_PIN) == LOW)
  {
    resetWiFiCredentials();
  }
}
