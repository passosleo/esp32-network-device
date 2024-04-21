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
#define TIME_TO_RESET 10000

#define WATER_FLOW_PIN 32
#define ENABLE_WATER_FLOW_PIN 18

#define EEPROM_SIZE 512
#define EEPROM_ADDR_MAGIC 0
#define EEPROM_MAGIC_VALUE 0xA5

volatile bool timerRunning = false;
unsigned long startTime = 0;
unsigned long elapsedTime = 0;

void startTimer()
{
  startTime = millis();
  timerRunning = true;
}

void stopTimer()
{
  elapsedTime = millis() - startTime;
  timerRunning = false;
}

unsigned long getElapsedTime()
{
  if (timerRunning)
  {
    return millis() - startTime;
  }
  else
  {
    return elapsedTime;
  }
}

int interval = 1000;
long currentMillis = 0;
long previousMillis = 0;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

void blinkLED(int pin, int delayTime = 1000)
{
  digitalWrite(pin, LOW);
  delay(delayTime);
  digitalWrite(pin, HIGH);
  delay(delayTime);
}

bool isWiFiConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

bool isButtonPressed(int pin)
{
  return digitalRead(pin) == LOW;
}

bool areWiFiCredentialsValid(String ssid, String password)
{
  WiFi.begin(ssid.c_str(), password.c_str());
  int attempts = 0;

  while (!isWiFiConnected() && attempts < 3)
  {
    delay(1000);
    attempts++;
  }

  if (isWiFiConnected())
  {
    WiFi.disconnect();
    return true;
  }

  return false;
}

void storeWiFiCredentials(String ssid, String password)
{
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC_VALUE);
  EEPROM.put(EEPROM_ADDR_MAGIC + 1, ssid);
  EEPROM.put(EEPROM_ADDR_MAGIC + 1 + sizeof(ssid), password);
  EEPROM.commit();
  EEPROM.end();
}

void handleRoot()
{
  server.send(200, "text/html", "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Configurar Wi-Fi</title></head><body><h1>Configuração de Rede Wi-Fi</h1><form action='/configure-wifi' method='post'><label for='ssid'>SSID:</label><input type='text' id='ssid' name='ssid'><br><br><label for='password'>Senha:</label><input type='password' id='password' name='password'><br><br><input type='submit' value='Enviar'></form><script>const params = new URLSearchParams(window.location.search);if (params.get('error') === '1') {alert('Credenciais inválidas!');}</script></body></html>");
}

void handleConfigureWiFi()
{
  if (server.method() == HTTP_POST)
  {
    ssid = server.arg("ssid");
    password = server.arg("password");

    if (areWiFiCredentialsValid(ssid, password))
    {
      storeWiFiCredentials(ssid, password);
      server.send(200, "text/html", "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Configurar Wi-Fi</title></head><body><p>Configuração salva com sucesso!</p></body></html>");
      delay(2000);
      ESP.restart();
    }
    else
    {
      server.sendHeader("Location", "/?error=1", true);
      server.send(302, "text/plain", "");
      // server.send(200, "text/html", "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Configurar Wi-Fi</title></head><body><p>Credenciais inválidas. Por favor, verifique e tente novamente.</p><a href='/'>Voltar</a></body></html>");
    }
  }
  else
  {
    server.send(405, "text/plain", "Not Allowed");
  }
}

bool hasStoredWiFiCredentials()
{
  EEPROM.begin(EEPROM_SIZE);
  bool hasCredentials = EEPROM.read(EEPROM_ADDR_MAGIC) == EEPROM_MAGIC_VALUE;
  EEPROM.end();
  return hasCredentials;
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
  delay(5000);
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
  pinMode(WATER_FLOW_PIN, INPUT_PULLUP);
  pinMode(ENABLE_WATER_FLOW_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  delay(1000);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;

  attachInterrupt(digitalPinToInterrupt(WATER_FLOW_PIN), pulseCounter, FALLING);

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

  if (!isInAPMode)
  {
    if (isWiFiConnected())
    {
      if (isButtonPressed(ENABLE_WATER_FLOW_PIN))
      {
        currentMillis = millis();
        if (currentMillis - previousMillis > interval)
        {

          pulse1Sec = pulseCount;
          pulseCount = 0;

          // Because this loop may not complete in exactly 1 second intervals we calculate
          // the number of milliseconds that have passed since the last execution and use
          // that to scale the output. We also apply the calibrationFactor to scale the output
          // based on the number of pulses per second per units of measure (litres/minute in
          // this case) coming from the sensor.
          flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
          previousMillis = millis();

          // Divide the flow rate in litres/minute by 60 to determine how many litres have
          // passed through the sensor in this 1 second interval, then multiply by 1000 to
          // convert to millilitres.
          flowMilliLitres = (flowRate / 60) * 1000;

          // Add the millilitres passed in this second to the cumulative total
          totalMilliLitres += flowMilliLitres;

          // Print the flow rate for this second in litres / minute
          Serial.print("Flow rate: ");
          Serial.print(int(flowRate)); // Print the integer part of the variable
          Serial.print("L/min");
          Serial.print("\t"); // Print tab space

          // Print the cumulative total of litres flowed since starting
          Serial.print("Output Liquid Quantity: ");
          Serial.print(totalMilliLitres);
          Serial.print("mL / ");
          Serial.print(totalMilliLitres / 1000);
          Serial.println("L");
        }
      }
      else
      {
        totalMilliLitres = 0;
        flowRate = 0;
      }
    }
    else
    {
      setupWiFiClientMode();
    }

    if (isButtonPressed(RESET_PIN))
    {
      if (!timerRunning)
      {
        startTimer();
      }
      else
      {
        if (getElapsedTime() >= 3000)
        {
          blinkLED(CLIENT_MODE_LED_PIN, 500);
        }

        if (getElapsedTime() >= TIME_TO_RESET)
        {
          stopTimer();
          Serial.println("Botão pressionado por 10 segundos. Resetando as credenciais WiFi...");
          digitalWrite(CLIENT_MODE_LED_PIN, LOW);
          resetWiFiCredentials();
        }
      }
    }
    else
    {
      digitalWrite(CLIENT_MODE_LED_PIN, HIGH);
      stopTimer();
    }
  }
  else
  {
    blinkLED(AP_MODE_LED_PIN);
  }
}
