#ifndef _wifi_setup_H
#define _wifi_setup_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>

// Replace with your primary network credentials
const char* ssid = "PrimarySSID";
const char* password = "PrimaryPassword";

// Replace with your secondary network credentials
const char* ssid2 = "SecondarySSID";
const char* password2 = "SecondaryPassword";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Web server html
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Graphic Equalizer</title>
  <style>
    html {font-family: Arial; display: inline-block}
    h2 {font-size: 3.5rem; text-align: center}
    p {font-size: 1.9rem;}
  table {width: 100%%}
  input {width: 80%%; border-radius: 10px}
  button {width: 100%%; height: 75px; font-size: 1.5rem; border-radius: 15px }
  button:hover{transition-duration: 0.5s; background-color: #555555; 
  color: white;}
  
    body {max-width: 500px; margin:0px auto; padding: 0px 7px; }
    .slider { -webkit-appearance: none; margin: 14px 0; width: 80%%; height: 12px; background: #af05e8; 
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 20px; height: 30px; background: #29c7cc; cursor: grabbing; border-radius: 10px;}
    .slider::-moz-range-thumb { width: 25px; height: 25px; background: #29c7cc; cursor: pointer; }
  .labelCol {width: 80px}
  .valCol {width: 40px; padding-left: 14px}
  </style>
</head>
<body>
  <h2>JD Equalizer</h2>
 <button type="button" id="nextBtn" onclick="sendData('n',1)" >Next pattern</button>

  </br></br>
  <table border="0">
  <tr>
</br>
  <label id="labelMessage" for="message">Message: </label>
  <input id="message" type="text" onchange="sendData('m',this.value)" value="%MESSAGE%">
  <br>
  <label id="labelSpeed" for="speedSlider">Speed: </label>
   <input type="range" id="speedSlider" onchange="sendData('s',this.value)" min="0" max="200" value="100" step="1" class="slider">
  </form><br>
    <label id="labelBrightness" for="brightnessSlider">Brightness: </label>
    <input type="range" id="brightnessSlider" onchange="sendData('t',this.value)" min="0" max="255" value="175" step="1" class="slider">
  <form action="/action_page.php">
  <label id= "labelColor" for="favcolor">Select color: </label>
  <input type="color" id="favcolor" value="%FAVCOLORVALUE%" onChange="sendData('c', this.value)">
  </form>

<script>
 var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  
  function onOpen(event) {
    console.log('Connection opened');
  }
  
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  
  function onMessage(event) {
    var dataType = event.data.charAt(0);
    var dataValue = event.data.substring(1);
    switch (dataType){
    case 's':
      document.getElementById('speedSlider').value = dataValue;
      break;
    case 't':
      document.getElementById('brightnessValue').innerHTML = dataValue;
      document.getElementById('brightnessSlider').value = dataValue;
      break;
    case 'c':
      document.getElementById('color').innerHTML = dataValue;
      break;
    case 'm':
      document.getElementById('message').innerHTML = dataValue;
      break;
    }
  }
  
  function onLoad(event) {
    initWebSocket();
  }
  function sendData(type, val) {
    console.log(type+val);
    websocket.send(type+val);
  }
</script>
</body>
</html>
)rawliteral";

uint8_t brightness = 175;
unsigned long red = 255;
unsigned long green= 255;
unsigned long blue = 255;
String col = "";
uint16_t scroll_speed = 100;
String text = "Josh Dolgin";
uint8_t pallet_count;
#define NUM_PALLETS  13

#define XRES 8
#define YRES 8
int x    = XRES;
int pass = 0;

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    Serial.println((char*)data);
    String message = String((char*)data);
    char dataType = message.charAt(0);
    String dataValue = message.substring(1);

    switch (dataType) {
      case 's':
        scroll_speed = dataValue.toInt();
        ws.textAll(message);
        break;
      case 't':
        brightness = dataValue.toInt();
        ws.textAll(message);
        break;
      case 'c':
      {
        col = dataValue + " ";
        String subRed = col.substring(1, 3);
        String subGreen = col.substring(3, 5);
        String subBlue = col.substring(5, 7);
        red = strtoul(subRed.c_str(), NULL, 16);
        green = strtoul(subGreen.c_str(), NULL, 16);
        blue = strtoul(subBlue.c_str(), NULL, 16);
        ws.textAll(message);
        break;
      }
      case 'm':
        text = dataValue;
        x    = XRES;
        pass = 0;
        ws.textAll(message);
        break;
      case 'n':
        pallet_count = (pallet_count + 1) % NUM_PALLETS;
        EEPROM.write(0, pallet_count);
        EEPROM.commit();
        break;
    }
  }
}


void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}


String processor(const String& var){
  if(var == "BRIGHTNESSVALUE"){
    return String(brightness);
  }
  if(var == "FAVCOLORVALUE"){
    return col;
  }
    if(var == "MESSAGE"){
    return text;
  }
    if(var == "SPEEDVALUE"){
    return String(scroll_speed);
  }
  return "";
}

void setupWebServer(){

  uint8_t connectionAttempts = 0;

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay (1000);
    Serial.println("Connecting to primary WiFi ...");
    connectionAttempts++;
    if (connectionAttempts > 20) break;    
  }

  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid2, password2);
    delay (1000);
    Serial.println("Connecting to secondary WiFi ...");
    connectionAttempts++;
    if (connectionAttempts > 10) break;    
  }

  // Print ESP Local IP Address
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.begin();
}

#endif
