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

#define AP_MODE_LED_PIN 2
#define CLIENT_MODE_LED_PIN 4
#define RESET_PIN 5

#define EEPROM_SIZE 512
#define EEPROM_ADDR_MAGIC 0
#define EEPROM_MAGIC_VALUE 0xA5

void blinkLED(int pin, int delayTime = 1000)
{
  digitalWrite(pin, LOW);
  delay(delayTime);
  digitalWrite(pin, HIGH);
  delay(delayTime);
}

void handleRoot()
{
  server.send(200, "text/html", "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Configurar Wi-Fi</title></head><body><h1>Configuração de Rede Wi-Fi</h1><form action='/configure-wifi' method='post'><label for='ssid'>SSID:</label><input type='text' id='ssid' name='ssid'><br><br><label for='password'>Senha:</label><input type='password' id='password' name='password'><br><br><input type='submit' value='Enviar'></form></body></html>");
}

void handleConfigureWiFi()
{
  if (server.method() == HTTP_POST)
  {
    ssid = server.arg("ssid");
    password = server.arg("password");

    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC_VALUE);
    EEPROM.put(EEPROM_ADDR_MAGIC + 1, ssid);
    EEPROM.put(EEPROM_ADDR_MAGIC + 1 + sizeof(ssid), password);
    EEPROM.commit();
    EEPROM.end();
    server.send(200, "text/html", "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Configurar Wi-Fi</title></head><body><p>Configuração salva com sucesso!</p></body></html>");
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

bool isWiFiConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

void setupWiFiClientMode()
{
  digitalWrite(AP_MODE_LED_PIN, LOW);
  WiFi.begin(ssid.c_str(), password.c_str());

  while (!isWiFiConnected())
  {
    blinkLED(CLIENT_MODE_LED_PIN);
    Serial.println("Tentando se conectar à rede WiFi...");
  }

  Serial.println("Conectado à rede WiFi");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  digitalWrite(CLIENT_MODE_LED_PIN, HIGH);
}

void setupWiFiAPMode()
{
  isInAPMode = true;
  digitalWrite(CLIENT_MODE_LED_PIN, LOW);
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

void getStoredWiFiCredentials()
{
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(EEPROM_ADDR_MAGIC + 1, ssid);
  EEPROM.get(EEPROM_ADDR_MAGIC + 1 + sizeof(ssid), password);
  EEPROM.end();
}

void setup()
{
  pinMode(AP_MODE_LED_PIN, OUTPUT);
  pinMode(CLIENT_MODE_LED_PIN, OUTPUT);
  pinMode(RESET_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  delay(1000);

  if (hasStoredWiFiCredentials())
  {
    getStoredWiFiCredentials();
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
    blinkLED(AP_MODE_LED_PIN);
  }

  if (!isWiFiConnected() && !isInAPMode)
  {
    setupWiFiClientMode();
  }

  if (digitalRead(RESET_PIN) == LOW)
  {
    resetWiFiCredentials();
  }
}
