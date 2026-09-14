/************************************************************
   ESP32 SMART HOME - HTTP + OLED CLOCK VERSION

   Relay 1 -> GPIO 2
   Relay 2 -> GPIO 4
   Relay 3 -> GPIO 15

   OLED:
   SDA -> GPIO 21
   SCL -> GPIO 22
   VCC -> 3.3V
   GND -> GND

   Communication:
   Website <-> Wi-Fi <-> ESP32 HTTP Server

   Features:
   - Wi-Fi connection
   - HTTP relay control
   - Individual relay control
   - All ON / All OFF
   - Relay status
   - OLED digital clock
   - Date
   - Day
   - Smooth/flicker-free 1-second clock update
************************************************************/

#include <WiFi.h>
#include <WebServer.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include <DHT.h>


// ============================================================
// 1. WIFI SETTINGS
// ============================================================

// CHANGE THESE

const char* WIFI_SSID = "Siva's_wifi_2G";
const char* WIFI_PASSWORD = "Hellosiva";


// ============================================================
// 2. RELAY PINS
// ============================================================

#define RELAY1 2
#define RELAY2 4
#define RELAY3 27


// ============================================================
// 3. RELAY LOGIC
// ============================================================

// Most relay modules are ACTIVE LOW

#define RELAY_ON  LOW
#define RELAY_OFF HIGH


// ============================================================
// 4. HTTP SERVER
// ============================================================

WebServer server(80);


// ============================================================
// 5. RELAY STATES
// ============================================================

bool relay1State = false;
bool relay2State = false;
bool relay3State = false;


// ============================================================
// 6. OLED SETTINGS
// ============================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define OLED_ADDRESS 0x3C


Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);


// ============================================================
// 7. CLOCK VARIABLES
// ============================================================

unsigned long lastClockUpdate = 0;

int lastSecond = -1;
int lastMinute = -1;
int lastHour   = -1;
int lastDay    = -1;


// ============================================================
// 8. CLOCK ENABLE
// ============================================================


bool clockEnabled = true;


// ============================================================
// 8A. DHT11 TEMPERATURE SENSOR
// ============================================================
// DHT11 DATA -> GPIO 18
// DHT11 VCC  -> 5V
// DHT11 GND  -> GND

#define DHTPIN 18
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

float temperatureC = 0.0;
bool temperatureValid = false;

unsigned long lastDTHRead = 0;
const unsigned long DTH_INTERVAL = 2500;

// Read the DHT11 only when the minimum DHT11 interval has passed.
bool readDHT() {

  if (
     millis() - lastDTHRead < DTH_INTERVAL
  ) {
    return temperatureValid;
  }
  lastDTHRead = millis();
  float newTemperature = dht.readTemperature();

  if (!isnan(newTemperature)) {
    temperatureC = newTemperature;
    temperatureValid = true;
    Serial.println("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" C");
    return true;
  }

  temperatureValid = false;
  return false;
}


// ============================================================
// TEMPERATURE ON HTTP
// ============================================================

void Tempratureon() {

  bool ok = readDHT();

  enableCORS();

  if (!ok) {

    server.send(
      500,
      "application/json",
      "{\"error\":\"DHT11 temperature read failed\"}"
    );

    return;
  }

  String response = "{\"temperature\":";
  response += String(temperatureC, 1);
  response += "}";

  server.send(
    200,
    "application/json",
    response
  );
}


// ============================================================
// TEMPERATURE OFF HTTP
// ============================================================
// The DHT11 is a sensor, so there is no physical ON/OFF relay.
// OFF only tells the website to stop displaying the temperature.

void Tempratureoff() {

  enableCORS();

  server.send(
    200,
    "text/plain",
    "TEMPERATURE OFF"
  );
}


// ============================================================
// TEMPERATURE READ HTTP
// ============================================================
// Kept as a simple endpoint so the website can directly request
// the current DHT11 reading.

void getTemperature() {

  bool ok = readDHT();

  enableCORS();

  if (!ok) {

    server.send(
      500,
      "application/json",
      "{\"error\":\"DHT11 temperature read failed\"}"
    );

    return;
  }

  String response = "{\"temperature\":";
  response += String(temperatureC, 1);
  response += "}";

  server.send(
    200,
    "application/json",
    response
  );
}


// ============================================================
// 9. CORS
// ============================================================

void enableCORS() {

  server.sendHeader(
    "Access-Control-Allow-Origin",
    "*"
  );

  server.sendHeader(
    "Access-Control-Allow-Methods",
    "GET,POST,OPTIONS"
  );

  server.sendHeader(
    "Access-Control-Allow-Headers",
    "Content-Type"
  );
}


// ============================================================
// 10. SET RELAY
// ============================================================

void setRelay(
  int relayPin,
  bool state
) {

  if (state) {

    digitalWrite(
      relayPin,
      RELAY_ON
    );

  } else {

    digitalWrite(
      relayPin,
      RELAY_OFF
    );
  }
}


// ============================================================
// 11. ALL RELAYS ON
// ============================================================

void allRelaysOn() {

  relay1State = true;
  relay2State = true;
  relay3State = true;


  setRelay(
    RELAY1,
    true
  );

  setRelay(
    RELAY2,
    true
  );

  setRelay(
    RELAY3,
    true
  );


  enableCORS();

  server.send(
    200,
    "text/plain",
    "ALL RELAYS ON"
  );


  Serial.println("ALL RELAYS ON");
}


// ============================================================
// 12. ALL RELAYS OFF
// ============================================================

void allRelaysOff() {

  relay1State = false;
  relay2State = false;
  relay3State = false;


  setRelay(
    RELAY1,
    false
  );

  setRelay(
    RELAY2,
    false
  );

  setRelay(
    RELAY3,
    false
  );


  enableCORS();

  server.send(
    200,
    "text/plain",
    "ALL RELAYS OFF"
  );


  Serial.println("ALL RELAYS OFF");
}


// ============================================================
// 13. RELAY 1 ON
// ============================================================

void relay1On() {

  relay1State = true;

  setRelay(
    RELAY1,
    true
  );


  enableCORS();

  server.send(
    200,
    "text/plain",
    "RELAY 1 ON"
  );


  Serial.println("Relay 1 ON");
}


// ============================================================
// 14. RELAY 1 OFF
// ============================================================

void relay1Off() {

  relay1State = false;

  setRelay(
    RELAY1,
    false
  );


  enableCORS();

  server.send(
    200,
    "text/plain",
    "RELAY 1 OFF"
  );


  Serial.println("Relay 1 OFF");
}


// ============================================================
// 15. RELAY 2 ON
// ============================================================

void relay2On() {

  relay2State = true;

  setRelay(
    RELAY2,
    true
  );


  enableCORS();

  server.send(
    200,
    "text/plain",
    "RELAY 2 ON"
  );


  Serial.println("Relay 2 ON");
}


// ============================================================
// 16. RELAY 2 OFF
// ============================================================

void relay2Off() {

  relay2State = false;

  setRelay(
    RELAY2,
    false
  );


  enableCORS();

  server.send(
    200,
    "text/plain",
    "RELAY 2 OFF"
  );


  Serial.println("Relay 2 OFF");
}


// ============================================================
// 17. RELAY 3 ON
// ============================================================

void relay3On() {

  relay3State = true;

  setRelay(
    RELAY3,
    true
  );


  enableCORS();

  server.send(
    200,
    "text/plain",
    "RELAY 3 ON"
  );


  Serial.println("Relay 3 ON");
}


// ============================================================
// 18. RELAY 3 OFF
// ============================================================

void relay3Off() {

  relay3State = false;

  setRelay(
    RELAY3,
    false
  );


  enableCORS();

  server.send(
    200,
    "text/plain",
    "RELAY 3 OFF"
  );


  Serial.println("Relay 3 OFF");
}


// ============================================================
// 19. GET RELAY STATUS
// ============================================================

void getStatus() {

  String response = "{";


  response += "\"relay1\":";
  response += relay1State ? "true" : "false";


  response += ",";


  response += "\"relay2\":";
  response += relay2State ? "true" : "false";


  response += ",";


  response += "\"relay3\":";
  response += relay3State ? "true" : "false";


  response += "}";


  enableCORS();


  server.send(
    200,
    "application/json",
    response
  );
}


// ============================================================
// 20. ROOT PAGE
// ============================================================

void handleRoot() {

  enableCORS();


  String message = "";


  message += "ESP32 SMART HOME\n";
  message += "HTTP SERVER RUNNING\n\n";


  message += "Relay 1: ";
  message += relay1State ? "ON\n" : "OFF\n";


  message += "Relay 2: ";
  message += relay2State ? "ON\n" : "OFF\n";


  message += "Relay 3: ";
  message += relay3State ? "ON\n" : "OFF\n";


  server.send(
    200,
    "text/plain",
    message
  );
}


// ============================================================
// 21. OLED CLOCK DRAW
// ============================================================

void updateOLEDClock() {

  // Update exactly once per second

  if (
    millis() - lastClockUpdate < 1000
  ) {

    return;
  }


  lastClockUpdate = millis();


  // ----------------------------------------------------------
  // CHECK CLOCK ENABLE
  // ----------------------------------------------------------

  if (!clockEnabled) {

    return;
  }


  // ----------------------------------------------------------
  // GET TIME
  // ----------------------------------------------------------

  struct tm timeinfo;


  if (
    !getLocalTime(
      &timeinfo
    )
  ) {

    Serial.println(
      "Unable to get time"
    );

    return;
  }


  // ----------------------------------------------------------
  // CURRENT VALUES
  // ----------------------------------------------------------

  int currentSecond = timeinfo.tm_sec;

  int currentMinute = timeinfo.tm_min;

  int currentHour = timeinfo.tm_hour;

  int currentDay = timeinfo.tm_mday;


  // ----------------------------------------------------------
  // DON'T REDRAW IF NOTHING CHANGED
  // ----------------------------------------------------------

  if (
    currentSecond == lastSecond &&
    currentMinute == lastMinute &&
    currentHour == lastHour &&
    currentDay == lastDay
  ) {

    return;
  }


  lastSecond = currentSecond;

  lastMinute = currentMinute;

  lastHour = currentHour;

  lastDay = currentDay;


  // ----------------------------------------------------------
  // FORMAT TIME
  // ----------------------------------------------------------

  char timeString[20];


  strftime(
    timeString,
    sizeof(timeString),
    "%I:%M:%S %p",
    &timeinfo
  );


  // ----------------------------------------------------------
  // FORMAT DATE
  // ----------------------------------------------------------

  char dateString[30];


  strftime(
    dateString,
    sizeof(dateString),
    "%d %b %Y",
    &timeinfo
  );


  // ----------------------------------------------------------
  // FORMAT DAY
  // ----------------------------------------------------------

  char dayString[20];


  strftime(
    dayString,
    sizeof(dayString),
    "%A",
    &timeinfo
  );


  // ==========================================================
  // DRAW OLED
  // ==========================================================

  display.clearDisplay();


  display.setTextColor(
    SSD1306_WHITE
  );


  // ----------------------------------------------------------
  // TITLE
  // ----------------------------------------------------------

  display.setTextSize(1);

  display.setCursor(
    42,
    0
  );

  display.print(
    "CLOCK"
  );


  // ----------------------------------------------------------
  // TIME
  // ----------------------------------------------------------

  display.setTextSize(2);

  display.setCursor(
    5,
    17
  );

  display.print(
    timeString
  );


  // ----------------------------------------------------------
  // DATE
  // ----------------------------------------------------------

  display.setTextSize(1);

  display.setCursor(
    30,
    40
  );

  display.print(
    dateString
  );


  // ----------------------------------------------------------
  // DAY
  // ----------------------------------------------------------

  display.setCursor(
    40,
    54
  );

  display.print(
    dayString
  );


  // ----------------------------------------------------------
  // SEND TO OLED
  // ----------------------------------------------------------

  display.display();
}


// ============================================================
// 22. OLED OFF
// ============================================================

void turnClockOff() {

  clockEnabled = false;


  display.clearDisplay();

  display.display();


  display.ssd1306_command(
    SSD1306_DISPLAYOFF
  );


  Serial.println(
    "OLED CLOCK OFF"
  );
}


// ============================================================
// 23. OLED ON
// ============================================================

void turnClockOn() {

  clockEnabled = true;


  display.ssd1306_command(
    SSD1306_DISPLAYON
  );


  // Force immediate refresh

  lastSecond = -1;

  lastMinute = -1;

  lastHour = -1;

  lastDay = -1;

  lastClockUpdate = 0;


  updateOLEDClock();


  Serial.println(
    "OLED CLOCK ON"
  );
}


// ============================================================
// 24. CLOCK ON HTTP
// ============================================================

void clockOn() {

  turnClockOn();


  enableCORS();


  server.send(
    200,
    "text/plain",
    "CLOCK ON"
  );
}


// ============================================================
// 25. CLOCK OFF HTTP
// ============================================================

void clockOff() {

  turnClockOff();


  enableCORS();


  server.send(
    200,
    "text/plain",
    "CLOCK OFF"
  );
}


// ============================================================
// 26. NOT FOUND
// ============================================================

void handleNotFound() {

  enableCORS();


  server.send(
    404,
    "text/plain",
    "Not Found"
  );
}


// ============================================================
// 27. SETUP
// ============================================================

void setup() {

  // ----------------------------------------------------------
  // SERIAL
  // ----------------------------------------------------------

  Serial.begin(
    115200
  );


  delay(1000);


  Serial.println();

  Serial.println(
    "===================================="
  );

  Serial.println(
    "       ESP32 SMART HOME"
  );

  Serial.println(
    "       HTTP + OLED CLOCK"
  );

  Serial.println(
    "===================================="
  );


  // ==========================================================
  // RELAYS
  // ==========================================================

  pinMode(
    RELAY1,
    OUTPUT
  );


  pinMode(
    RELAY2,
    OUTPUT
  );


  pinMode(
    RELAY3,
    OUTPUT
  );


  // ----------------------------------------------------------
  // ALL RELAYS OFF
  // ----------------------------------------------------------

  digitalWrite(
    RELAY1,
    RELAY_OFF
  );


  digitalWrite(
    RELAY2,
    RELAY_OFF
  );


  digitalWrite(
    RELAY3,
    RELAY_OFF
  );


  relay1State = false;

  relay2State = false;

  relay3State = false;


  Serial.println(
    "Relays initialized"
  );

  Serial.println(
    "All relays OFF"
  );


  // ==========================================================
  // DHT11 TEMPERATURE SENSOR
  // ==========================================================

  dht.begin();

  Serial.println(
    "DHT11 initialized on GPIO 18"
  );


  // ==========================================================
  // OLED
  // ==========================================================

  Wire.begin(
    21,
    22
  );


  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC,
      OLED_ADDRESS
    )
  ) {

    Serial.println(
      "OLED initialization FAILED!"
    );

  } else {

    Serial.println(
      "OLED initialized!"
    );


    display.clearDisplay();


    display.setTextColor(
      SSD1306_WHITE
    );


    display.setTextSize(1);


    display.setCursor(
      35,
      20
    );

    display.println(
      "SMART HOME"
    );


    display.setCursor(
      40,
      35
    );

    display.println(
      "Starting..."
    );


    display.display();


    delay(1500);
  }


  // ==========================================================
  // WIFI
  // ==========================================================

  WiFi.mode(
    WIFI_STA
  );


  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );


  Serial.println();

  Serial.print(
    "Connecting to Wi-Fi"
  );


  int attempts = 0;


  while (
    WiFi.status() != WL_CONNECTED &&
    attempts < 30
  ) {

    delay(500);

    Serial.print(".");

    attempts++;
  }


  Serial.println();


  // ==========================================================
  // WIFI RESULT
  // ==========================================================

  if (
    WiFi.status() == WL_CONNECTED
  ) {

    Serial.println();

    Serial.println(
      "===================================="
    );

    Serial.println(
      "       WIFI CONNECTED!"
    );

    Serial.println(
      "===================================="
    );


    Serial.print(
      "Wi-Fi Name: "
    );

    Serial.println(
      WiFi.SSID()
    );


    Serial.print(
      "ESP32 IP Address: "
    );

    Serial.println(
      WiFi.localIP()
    );


    Serial.print(
      "Signal Strength: "
    );

    Serial.print(
      WiFi.RSSI()
    );

    Serial.println(
      " dBm"
    );

  }

  else {

    Serial.println();

    Serial.println(
      "===================================="
    );

    Serial.println(
      "       WIFI CONNECTION FAILED"
    );

    Serial.println(
      "===================================="
    );


    Serial.println(
      "Check Wi-Fi name and password."
    );


    return;
  }


  // ==========================================================
  // NTP TIME
  // ==========================================================

  // India = UTC + 5:30

  configTime(
    19800,
    0,
    "pool.ntp.org",
    "time.nist.gov"
  );


  Serial.println(
    "NTP clock configured"
  );


  // ==========================================================
  // HTTP ROUTES
  // ==========================================================

  // ROOT

  server.on(
    "/",
    HTTP_GET,
    handleRoot
  );


  // ----------------------------------------------------------
  // ALL RELAYS
  // ----------------------------------------------------------

  server.on(
    "/relay/on",
    HTTP_GET,
    allRelaysOn
  );


  server.on(
    "/relay/off",
    HTTP_GET,
    allRelaysOff
  );


  // ----------------------------------------------------------
  // RELAY 1
  // ----------------------------------------------------------

  server.on(
    "/relay1/on",
    HTTP_GET,
    relay1On
  );


  server.on(
    "/relay1/off",
    HTTP_GET,
    relay1Off
  );


  // ----------------------------------------------------------
  // RELAY 2
  // ----------------------------------------------------------

  server.on(
    "/relay2/on",
    HTTP_GET,
    relay2On
  );


  server.on(
    "/relay2/off",
    HTTP_GET,
    relay2Off
  );


  // ----------------------------------------------------------
  // RELAY 3
  // ----------------------------------------------------------

  server.on(
    "/relay3/on",
    HTTP_GET,
    relay3On
  );


  server.on(
    "/relay3/off",
    HTTP_GET,
    relay3Off
  );


  // ----------------------------------------------------------
  // STATUS
  // ----------------------------------------------------------

  server.on(
    "/status",
    HTTP_GET,
    getStatus
  );


  // ----------------------------------------------------------
  // CLOCK
  // ----------------------------------------------------------

  server.on(
    "/clock/on",
    HTTP_GET,
    clockOn
  );


  server.on(
    "/clock/off",
    HTTP_GET,
    clockOff
  );

// ----------------------------------------------------------
  // Temprature
  // ----------------------------------------------------------

  server.on(
    "/Temprature/on",
    HTTP_GET,
    Tempratureon
  );


  server.on(
    "/Temprature/off",
    HTTP_GET,
    Tempratureoff
  );


  // ----------------------------------------------------------
  // DHT11 TEMPERATURE VALUE
  // ----------------------------------------------------------

  server.on(
    "/temperature",
    HTTP_GET,
    getTemperature
  );

  // Also accept /temperature/ in case the browser sends a trailing slash.
  server.on(
    "/temperature/",
    HTTP_GET,
    getTemperature
  );
  // ----------------------------------------------------------
  // 404
  // ----------------------------------------------------------

  server.onNotFound(
    handleNotFound
  );


  // ==========================================================
  // START SERVER
  // ==========================================================

  server.begin();


  Serial.println();

  Serial.println(
    "===================================="
  );

  Serial.println(
    "       HTTP SERVER STARTED"
  );

  Serial.println(
    "===================================="
  );


  Serial.print(
    "Open in browser: http://"
  );

  Serial.println(
    WiFi.localIP()
  );


  Serial.println();

  Serial.println(
    "Clock starting..."
  );
}


// ============================================================
// 28. LOOP
// ============================================================

void loop() {

  // ----------------------------------------------------------
  // WIFI RECONNECT
  // ----------------------------------------------------------

  if (
    WiFi.status() != WL_CONNECTED
  ) {

    Serial.println(
      "Wi-Fi disconnected. Reconnecting..."
    );


    WiFi.disconnect();


    WiFi.begin(
      WIFI_SSID,
      WIFI_PASSWORD
    );


    delay(1000);


    return;
  }


  // ----------------------------------------------------------
  // HANDLE WEBSITE REQUESTS
  // ----------------------------------------------------------

  server.handleClient();


  // ----------------------------------------------------------
  // UPDATE OLED CLOCK
  // ----------------------------------------------------------

  updateOLEDClock();


  // ----------------------------------------------------------
  // SMALL DELAY
  // ----------------------------------------------------------

  delay(2);
}