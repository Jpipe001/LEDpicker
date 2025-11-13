/********* LEDPicker_11_12_25 Modified to use Adafruit_NeoPixel *********/

#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#define WS2811 2811
#define WS2812 2812
WiFiServer server(80); // Set web server port number to 80

// ################ CONFIGURE YOUR SETTINGS HERE ################
// Please Modify the "Your Network Name" and "Your Network Password"

//const char*        ssid = "Your Network Name";       // Your Network SSID
//const char*    password = "Your Network Password";   // Your Network Password

// Set Up LEDS   ***   Connect LEDS to Ground & 5 Volts on ESP32 pins (There are Varients, so Check Connections)
#define LED_TYPE  WS2811    // Type of LED String (WS2811 = RGB) or (WS2812 = GRB)
#define NUM_LEDS      10    // Number of LEDs displaying color
#define LED_DATA_PIN   5    // GPIO pin connected to LED data line
#define BRIGHTNESS    16    // Master LED Brightness
// ################   END OF SETTINGS  ################

#if LED_TYPE == WS2811
Adafruit_NeoPixel strip(NUM_LEDS, LED_DATA_PIN, NEO_RGB + NEO_KHZ800); // (WS2811 = RGB)
#else
Adafruit_NeoPixel strip(NUM_LEDS, LED_DATA_PIN, NEO_GRB + NEO_KHZ800); // (WS2812 = GRB)
#endif
//Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, LED_PIXEL_TYPE);

String HW_addr; // WiFi local hardware Address
String SW_addr; // WiFi local software Address
const char* ServerName = "ledcolor"; // Test Link: http://ledcolor.local

// *********** Initialize LEDs
void Init_LEDS() {
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show(); // Initialize all pixels to 'off'
  Serial.printf("LEDs Initialized Using WS%d with %d LEDs.\n\n", LED_TYPE, NUM_LEDS);
}

// *********** Set LEDS to Color
void Display_Color_LEDS(int Red, int Green, int Blue) {
  Serial.printf("Color Selected = Red: %d \tGreen: %d\tBlue: %d\n", Red, Green, Blue);
  strip.clear();
  strip.show();
  delay(300);
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(Red, Green, Blue));
  }
  strip.show();
}

void setup() {
  Serial.begin(115200);
  delay(4000);

  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  Serial.printf("\nWiFi Connecting to %s Network", ssid);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.printf(".");
    count++;
    if (count > 80) break;
  }
  if (count > 80) Serial.printf("\n ~ Can't Connect to Network\n\n");
  else Serial.printf(" ~ Connected Successfully\n");

  if (!MDNS.begin(ServerName) || count > 80) {
    Serial.printf("\nSOMETHING WENT WRONG\nError setting up MDNS responder!\nProgram Halted ~ Check Network Settings !!\n");
    while (1) {
      delay(1000);
    }
  }

  Init_LEDS();
  wifiMulti.addAP(ssid, password);
  HW_addr = "http://" + WiFi.localIP().toString();
  SW_addr = "http://" + String(ServerName) + ".local";
  server.begin();
}


// *********** Web Page Code
String Web_Page() {
  String html_code = "<!DOCTYPE html><html>";
  html_code += "<HEAD>";
  html_code += "<meta name=\'viewport\' content=\'width=device-width, initial-scale=1.0, user-scalable=no\'>";
  //html_code_code += "<meta http-equiv='refresh' content='' + String(refresh) + ''>";
  html_code += "<TITLE>" + String(ServerName) + "</TITLE>";

  //***********   STYLE   ***********
  html_code += "<STYLE type='text/css'>";
  html_code += "html_code { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: left;}";

  //***********   CSS   ***********
  html_code += ".color-picker {display: flex; flex-direction: column; align-items: center;}";
  html_code += ".slider {width: 300px; -webkit-appearance: none; appearance: none; height: 25px; border-radius: 12px; outline: none; opacity: 0.7; transition: opacity 0.2s;}";
  html_code += ".slider:hover {opacity: 1;}";
  html_code += ".value-box {width: 40px; margin-left: 10px; font-size: 18px; text-align: center; border: none;}";
  html_code += "#redSlider {background-color: #f00;}";
  html_code += "#greenSlider {background-color: #0f0;}";
  html_code += "#blueSlider {background-color: #00f;}";
  html_code += ".color-display {width: 200px; height: 200px; border: 2px solid #000; display: flex; align-items: center; justify-content: center;}";
  html_code += ".hex-code {font-size: 24px; font-weight: bold; color: #000;}";
  html_code += ".send-button {padding: 10px 20px; background-color: #000; color: #fff; border: none; border-radius: 5px; cursor: pointer;}";
  html_code += ".send-button:hover {background-color: #333;}";
  
  html_code += "table {border-collapse: collapse; margin-top: 20px;}";
  html_code += "td, th {border: 3px solid #6060bd; padding: 10px; text-align: center;}";
  html_code += "</style></head>";

  //***********   BODY   ***********
  html_code += "<body><center><h1>LED Color Picker and HEX Converter</h1><P>";
  //  <table bordercolor= 'white' BORDER='3' CELLPADDING='5'>
  html_code += "<table border= '1'>";
  html_code += "<tr><td>Connected to : </td><td>" + String(ssid) + "</td></tr>";
  html_code += "<tr><td>IP Address : </td><td>" + HW_addr + "</td></tr>";
  html_code += "<tr><td>Url Address : </td><td>" +  SW_addr + "</td></tr>";
  html_code += "<tr><td>LED Configuration : </td><td>WS" + String(LED_TYPE) + " using " + String(NUM_LEDS) + " LEDS</td></tr>";
  html_code += "</table><p><p>";
  
  //***********   TABLE   ***********
  html_code += "<div class='color-picker'><table><tr><th>Color</th><th>Slider and Decimal Value</th><th>Color Box</th></tr>";
  
  html_code += "<tr><td>Red</td><td><input type='range' class='slider' id='redSlider' min='0' max='255' value='128'>";
  html_code += "<input type='text' class='value-box' id='redValue' value='128' readonly></td>";
  html_code += "<td rowspan='3'><div class='color-display' id='colorDisplay'></div></td></tr>";
  html_code += "<tr><td>Green</td><td><input type='range' class='slider' id='greenSlider' min='0' max='255' value='128'>";
  html_code += "<input type='text' class='value-box' id='greenValue' value='128' readonly></td></tr>";
  html_code += "<tr><td>Blue</td><td><input type='range' class='slider' id='blueSlider' min='0' max='255' value='128'>";
  html_code += "<input type='text' class='value-box' id='blueValue' value='128' readonly></td></tr>";
  html_code += "<tr><td></td><td>Hex Code: <span class='hex-code' id='hexCode'></span></td>";
  html_code += "<td><button class='send-button' id='sendButton'>Send Color Code</button></td></tr>";
  html_code += "</table></div>";
  
  //***********   SCRIPT   ***********
  html_code += "<script> const redSlider = document.getElementById('redSlider');";
  html_code += "const greenSlider = document.getElementById('greenSlider');";
  html_code += "const blueSlider = document.getElementById('blueSlider');";
  html_code += "const redValue = document.getElementById('redValue');";
  html_code += "const greenValue = document.getElementById('greenValue');";
  html_code += "const blueValue = document.getElementById('blueValue');";
  html_code += "const colorDisplay = document.getElementById('colorDisplay');";
  html_code += "const hexCode = document.getElementById('hexCode');";
  html_code += "const sendButton = document.getElementById('sendButton');";
  
  html_code += "function updateColor() {const red = redSlider.value; const green = greenSlider.value; const blue = blueSlider.value;";
  html_code += "const hexColor = `#${(red << 16 | green << 8 | blue).toString(16).padStart(6, '0')}`;";
  html_code += "colorDisplay.style.backgroundColor = hexColor; hexCode.innerText = hexColor;";
  html_code += "redValue.value = red; greenValue.value = green; blueValue.value = blue;}";
  
  html_code += "function sendColor() {const red = redSlider.value; const green = greenSlider.value; const blue = blueSlider.value;";
  html_code += "const url = `" + SW_addr + "/updateColor?Red=${red}&Grn=${green}&Blu=${blue}`; fetch(url)";
  html_code += ".then(response => response.text()) .then(data => console.log(data)) .catch(error => console.error('Error:', error));}";
  
  html_code += "redSlider.addEventListener('input', updateColor);";
  html_code += "greenSlider.addEventListener('input', updateColor);";
  html_code += "blueSlider.addEventListener('input', updateColor);";
  html_code += "sendButton.addEventListener('click', sendColor);";
  
  html_code += "updateColor(); </script></body></html>";
  
  //const url = `http://your-esp32-ip-address/?r=${r}&g=${g}&b=${b}`;
  //html_code += "const url = `" + SW_addr + "/?Red=${r}&Grn=${g}&Blu=${b}`; fetch(url)";

  return html_code;
}


// *********** Main Loop
void loop() {
  String header;
  const long timeoutTime = 2000;
  unsigned long currentTime = millis();
  unsigned long previousTime = currentTime;
  WiFiClient client = server.available();

  if (client) {
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            int search0 = header.indexOf("GET /updateColor?");
            if (search0 >= 0) {
              int rStart = header.indexOf("Red=") + 4;
              int rEnd = header.indexOf("&", rStart);
              int gStart = header.indexOf("Grn=") + 4;
              int gEnd = header.indexOf("&", gStart);
              int bStart = header.indexOf("Blu=") + 4;
              int bEnd = header.indexOf(" HTTP", bStart);

              int R = header.substring(rStart, rEnd).toInt();
              int G = header.substring(gStart, gEnd).toInt();
              int B = header.substring(bStart, bEnd).toInt();

              Display_Color_LEDS(R, G, B);
            } else {
              client.println(Web_Page());
            }
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
  }
}
