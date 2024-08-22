/*********
  08/22/24  Latest Software on Github : https://github.com/Jpipe001/LEDpicker    <<   Check for Updates
  Concepts used from Rui Santos and David Bird
*********/

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>      // MDNS_Generic  by Khoi Hoang
#include <WiFiMulti.h>    // WifiMulti_Generic  by Khoi Hoang
WiFiMulti wifiMulti;
#include <HTTPClient.h>   // HttpClient  by Adrian McEwen
#include <FastLED.h>      // FastLED  by Daniel Garcia
WiFiServer server(80);    // Set web server port number to 80

//  ################   CONFIGURE YOUR SETTINGS HERE  ################

const char*        ssid = "Network SSID";       // your Network SSID
const char*    password = "Network Password";       // your Network Password

/* Please Modify the "NUM_LEDS and LED_TYPE and COLOR_ORDER" To suit your Project
  Note: LED_TYPE and COLOR_ORDER are Important. If LED is RED when it should be GREEN,
  Then Change the LED_TYPE to WS2811 and COLOR_ORDER to RGB.*/

// Set Up LEDS   ***   Connect LEDS to Ground & 5 Volts on ESP32 pins (There are Varients, so Check)

#define NUM_LEDS             10      // Number of LEDS displaying color
#define LED_TYPE         WS2812      // WS2811 or WS2812 or NEOPIXEL
#define COLOR_ORDER         GRB      // WS2811 are RGB or WS2812 are GRB or NEOPIXEL are CRGB
#define DATA_PIN              5      // Connect LED Data Line to pin D5/P5/GPIO5  *** With CURRENT LIMITING 330 Ohm Resistor in Line ***
#define BRIGHTNESS           16      // Master LED Brightness (<12=Dim 16~20=ok >20=Too Bright) Set as required
CRGB leds[NUM_LEDS];                 // Color Order for LEDs ~ CRGB for Displaying Gradients
#define FRAMES_PER_SECOND   120

//  ################   END OF SETTINGS  ################


String header;                  // Variable to store the HTTP request
unsigned long currentTime = millis();// Current time
unsigned long previousTime = 0; // Previous time
const long timeoutTime = 2000;  // Define timeout time in milliseconds (example: 2000ms = 2s)
String HW_addr;                 // WiFi local hardware Address
String SW_addr;                 // WiFi local software Address
// HTML Web Page with a Stable LOGICAL Address, because hardware address may change.
const char* ServerName = "LEDpicker";


void setup() {
  Serial.begin(115200);
  delay(4000);         // Time to press "Clear output" in Serial Monitor

  String ShortFileName;         // Shortened File Name
  ShortFileName = String(__FILE__).substring(String(__FILE__).lastIndexOf("\\") + 1);       // Shortened File Name
  if (ShortFileName == __FILE__) ShortFileName = String(__FILE__).substring(String(__FILE__).lastIndexOf("/") + 1);  // Shortened File Name Using Raspberry Pi
  Serial.printf("Long File Name   ~ %s\n", __FILE__);
  Serial.printf("Short File Name  ~ %s\n", ShortFileName.c_str());
  Serial.printf("Date Compiled    ~ %s\n\n", __DATE__);

  // Initialize the WiFi network settings.
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);

  // CONNECT to WiFi network:
  Serial.printf("WiFi Connecting to %s  ", ssid);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED)   {
    delay(300);    // Wait a little bit
    Serial.printf(".");
    count++;
    if (count > 80) break;    // Loop for 80 tries
  }
  if (count > 80)   Serial.printf("\n ~ Can't Connect to Network\n\n"); else  Serial.printf(" ~ Connected Successfully\n");
  if (!MDNS.begin(ServerName) || count > 80) {     // Start mDNS with ServerName
    Serial.printf("\nSOMETHING WENT WRONG\nError setting up MDNS responder !\nProgram Halted  ~  Check Network Settings !!\n");
    while (1) {
      delay(1000);                   // Stay here
    }
  }
  // Print the Signal Strength:
  long rssi = WiFi.RSSI() + 100;
  Serial.printf("Signal Strength = %ld", rssi);
  if (rssi > 50)  Serial.printf(" (>50 - Good)\n");  else   Serial.printf(" (Could be Better)\n");

  Init_LEDS();         // Initialize LEDs

  Serial.printf("*******************************************\n");

  wifiMulti.addAP(ssid, password);         // Initialize the WiFi settings to Get data from Server.

  Serial.printf("Connected to the %s WiFi Network.", ssid);

  HW_addr = "http://" + WiFi.localIP().toString();      // IP Address
  Serial.printf("  IP Address : %s\n", HW_addr.c_str());

  SW_addr = "http://" + String(ServerName) + ".local";  // Logical Address (ServerName)
  Serial.printf("MDNS started ~ Use THIS ADDRESS in your Browser : %s\n", SW_addr.c_str());

  server.begin();

  Serial.printf("*******************************************\n");
}

// *********** Initialize LEDs
void Init_LEDS() {
  FastLED.setBrightness(BRIGHTNESS);           // Master brightness control
  //FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS); // For NEOPIXEL
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip); // For WS2812
  Serial.printf("LEDs Initialized for No_Leds = %d\n", NUM_LEDS);
}

// *********** Reset LEDS to Black
void Display_Black_LEDS() {       
  fill_solid(leds, NUM_LEDS, CRGB::Black);  // Set All leds to Black
  FastLED.show();
  delay(50);    // Wait a smidgen
}

// *********** Set LEDS to Color
void Display_Color_LEDS(int Red, int Green, int Blue) {
  fill_solid(leds, NUM_LEDS, CRGB::Black);       // Set All leds to Black
  FastLED.show();
  delay(1000);    // Wait a second
  fill_solid(leds, NUM_LEDS, CRGB(Red, Green, Blue));  // Set LEDS to Color
  FastLED.show();
}

// *********** Web Page Code
String Web_Page() {         // 
  String html_code = "<!DOCTYPE html><html>";
  html_code += "<HEAD>";
  html_code += "<meta name=\'viewport\' content=\'width=device-width, initial-scale=1.0, user-scalable=no\'>";
  //html_code_code += "<meta http-equiv='refresh' content='' + String(refresh) + ''>";
  html_code += "<TITLE>" + String(ServerName) + "</TITLE>";

  //***********   STYLE   ***********
  html_code += "<STYLE type='text/css'>";
  html_code += "html_code { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: left;}";

  //***********   CSS   ***********
  html_code += ".sliderR {appearance: none; width: 300px; height: 25px; background: #FF0000 ; outline: none; border - radius: 12px;}";
  html_code += ".sliderG {appearance: none; width: 300px; height: 25px; background: #00FF00 ; outline: none; border - radius: 12px;}";
  html_code += ".sliderB {appearance: none; width: 300px; height: 25px; background: #0000FF ; outline: none; border - radius: 12px;}";
  html_code += ".output {font-size: 20px; margin-top: 10px;}";
  html_code += ".ColorBox {width: 200px; height: 200px; border: 2px solid black; margin - top: 20px;}";
  html_code += "</STYLE></HEAD>";

  //***********   BODY   ***********
  html_code += "<body><center><h1>LED Color Picker and HEX Converter</h1><P>";

  html_code += "Connected to : " + String(ssid) + "<BR>";
  long rssi = WiFi.RSSI() + 100;
  html_code += "Signal Strength : " + String(rssi) + "<BR>";
  html_code += "Local IP Address : " + HW_addr + "<BR>";
  html_code += "Use Url Address : " +  SW_addr + "<P><P>";

  //***********   CODE   ***********
  html_code += "<table BORDER = '2' CELLPADDING = '5'>";
  html_code += "<tr><td><label for = 'sliderR'>Red: </label><p>";
  html_code += "<label for = 'sliderG'>Green: </label><p>";
  html_code += "<label for = 'sliderB'>Blue: </label><p></td>";
  html_code += "<td><input type = 'range' id = 'sliderR' class = 'sliderR' min = '0' max = '255' value = '128'><span id = 'Rvalue'>128 </span><br>";
  html_code += "<input type = 'range' id = 'sliderG' class = 'sliderG' min = '0' max = '255' value = '128'><span id = 'Gvalue'>128 </span><br>";
  html_code += "<input type = 'range' id = 'sliderB' class = 'sliderB' min = '0' max = '255' value = '128'><span id = 'Bvalue'>128 </span><br></td>";
  html_code += "<td><div class = 'ColorBox' id = 'ColorBox'></div></td></tr>";
  html_code += "<td><td><div class = 'output'>HEX CODE: <span id = 'hexValue'>#808080</span></div></td>";
  html_code += "<td><center><button onclick='sendColorToServer()'>Send Decimal Color Codes</button></center></td></tr></table>";

  //***********   SCRIPT   ***********
  html_code += "<script>const sliderR = document.getElementById('sliderR'); const sliderG = document.getElementById('sliderG'); const sliderB = document.getElementById('sliderB'); ";
  html_code += "const Rvalue = document.getElementById('Rvalue'); const Gvalue = document.getElementById('Gvalue'); const Bvalue = document.getElementById('Bvalue'); ";
  html_code += "const hexValue = document.getElementById('hexValue'); const ColorBox = document.getElementById('ColorBox'); ";

  html_code += "function updateColor() { const r = parseInt(sliderR.value); const g = parseInt(sliderG.value); const b = parseInt(sliderB.value); ";
  html_code += "Rvalue.textContent = r; Gvalue.textContent = g; Bvalue.textContent = b; ";
  html_code += "const hex = rgbToHex(r, g, b); hexValue.textContent = hex; ";
  html_code += "ColorBox.style.backgroundColor = hex; ";
  html_code += "sendHexToServer(hex);}";

  html_code += "function rgbToHex(r, g, b) { return '#' + [r, g, b].map(x => { const hex = x.toString(16); return hex.length === 1 ? '0' + hex : hex; }).join('');}";

  html_code += "function sendColorToServer() {";
  html_code += "const r = document.getElementById('sliderR').value; const g = document.getElementById('sliderG').value; const b = document.getElementById('sliderB').value;";
  //const url = `http://your-esp32-ip-address/?r=${r}&g=${g}&b=${b}`;
  html_code += "const url = `" + SW_addr + "/?r=${r}&g=${g}&b=${b}`; fetch(url)";
  html_code += ".then(response => response.text()) .then(data => console.log(data)) .catch(error => console.error('Error:', error)); }";

  html_code += "sliderR.addEventListener('input', updateColor);";
  html_code += "sliderG.addEventListener('input', updateColor);";
  html_code += "sliderB.addEventListener('input', updateColor);";

  html_code += "</script></center></body></html>";
  return html_code;
}

// *********** Main Loop - concept from Rui Santos
void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    //Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        // Serial.write(c);                    // print it out the serial monitor
        header += c;

        if (header == "GET /favicon.ico HTTP/1.1")  break;
        // if the header (from a Computer) then ignore "GET /favicon.ico HTTP/1.1"

        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            //Serial.printf("header = \n%s\n", header.c_str());

            int search0 = header.indexOf("GET / HTTP/1.1");  //Get Web Page
            if (search0 >= 0)  {
              // Display the HTML web page
              String html_code = Web_Page();
              client.println(html_code);
            }

            search0 = header.indexOf("GET /?r=");  //Get LED Color
            if (search0 >= 0)  {
              search0 = header.indexOf("?r=", search0 + 1) + 3;
              int search1 = header.indexOf("&", search0 + 1);
              String r = header.substring(search0, search1);

              search0 = header.indexOf("&g=") + 3;
              search1 = header.indexOf("&", search0 + 1);
              String g = header.substring(search0, search1);

              search0 = header.indexOf("&b=") + 3;
              search1 = header.indexOf(" HTTP", search0 + 1);
              String b = header.substring(search0, search1);

              Serial.printf("Color Selected = Red: .%s.\tGreen: .%s.\tBlue: .%s.\n", r.c_str(), g.c_str(), b.c_str());

              if (r.length() > 0 && g.length() > 0 && b.length() > 0)
              if (r.length() < 4 && g.length() < 4 && b.length() < 4)  {
                int R = r.toInt();
                int G = g.toInt();
                int B = b.toInt();
                Display_Color_LEDS(R, G, B);
              }  else  
                Serial.printf("Something Went Wrong.\n");
              }

            client.println();     // The HTTP response ends with another blank line
            break;                // Break out of the while loop
          } else {                // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {   // if you got anything else but a carriage return character,
          currentLine += c;       // add it to the end of the currentLine
        }
      }             //  if (client.available())
    }               //  while (client.connected()
    header = "";    // Clear the header variable

    client.stop();  // Close the connection
    //Serial.println("Client disconnected.\n");
  }                 // if (client)
}
