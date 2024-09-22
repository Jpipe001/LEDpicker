/*********
  08/24/24  Latest Software on Github : https://github.com/Jpipe001/LEDpicker    <<   Check for Updates
  Concepts used from Rui Santos and David Bird
*********/

#include <Arduino.h>
#include <ESPmDNS.h>      // MDNS_Generic  by Khoi Hoang
#include <WiFiMulti.h>    // WifiMulti_Generic  by Khoi Hoang
WiFiMulti wifiMulti;
#include <HTTPClient.h>   // HttpClient  by Adrian McEwen
#include <FastLED.h>      // FastLED  by Daniel Garcia
WiFiServer server(80);    // Set web server port number to 80

//  ################   CONFIGURE YOUR SETTINGS HERE  ################

const char*        ssid = "SSID";       // your Network SSID
const char*    password = "Password";       // your Network Password

/* Please Modify the "NUM_LEDS and LED_TYPE and COLOR_ORDER" To suit your Project
  Note: LED_TYPE and COLOR_ORDER are Important. If LED is RED when it should be GREEN,
  Then Change the LED_TYPE to WS2811 and COLOR_ORDER to RGB.*/

// Set Up LEDS   ***   Connect LEDS to Ground & 5 Volts on ESP32 pins (There are Varients, so Check)

#define NUM_LEDS             10      // Number of LEDS displaying color
#define LED_TYPE         WS2812      // WS2811 or WS2812 or NEOPIXEL
#define COLOR_ORDER         GRB      // WS2811 are RGB or WS2812 are GRB or NEOPIXEL are CRGB
#define DATA_PIN              5      // Connect LED Data Line to pin D5/P5/GPIO5  *** With CURRENT LIMITING 330 Ohm Resistor in Line ***
#define BRIGHTNESS           16      // Master LED Brightness (<12=Dim 16~20=ok >20=Too Bright) Set as required
CRGB leds[NUM_LEDS];                 // Color Order for LEDs ~ CRGB Colors
#define FRAMES_PER_SECOND   120

//  ################   END OF SETTINGS  ################


String HW_addr;                 // WiFi local hardware Address
String SW_addr;                 // WiFi local software Address
// HTML Web Page with a Stable LOGICAL Address, because hardware address may change.
const char* ServerName = "ledcolor";
// Test Link: http://ledcolor.local


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
  FastLED.setBrightness(BRIGHTNESS);           // Set Master Brightness
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip); // For WS2812
  //FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS); // For NEOPIXEL
  Serial.printf("LEDs Initialized for No_Leds = %d\n", NUM_LEDS);
}


// *********** Set LEDS to Color
void Display_Color_LEDS(int Red, int Green, int Blue) {
  Serial.printf("Color Selected = Red: %d \tGreen: %d\tBlue: %d\n", Red, Green, Blue);
  fill_solid(leds, NUM_LEDS, CRGB::Black);       // Set All leds to Black
  FastLED.show();
  delay(300);    // Wait a bit
  fill_solid(leds, NUM_LEDS, CRGB(Red, Green, Blue));  // Set LEDS to Color
  FastLED.show();
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
  html_code += "<table border= '1'><tr><td>";
  html_code += "Connected to : </td><td>" + String(ssid) + "</td></tr>";
  html_code += "<tr><td>IP Address : </td><td>" + HW_addr + "</td></tr>";
  html_code += "<tr><td>Url Address : </td><td>" +  SW_addr + "</td></tr></table><p><p>";

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


// *********** Main Loop - concept from Rui Santos
void loop() {
  String header;                  // Variable to store the HTTP request
  const long timeoutTime = 2000;  // Define timeout time in milliseconds (example: 2000ms = 2s)
  unsigned long currentTime = millis();
  unsigned long previousTime = currentTime;
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
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

            search0 = header.indexOf("GET /updateColor?");  //Get LED Color
            if (search0 >= 0)  {
              search0 = header.indexOf("Red=", search0 + 1) + 4;
              int search1 = header.indexOf("&", search0 + 1);
              String Red = header.substring(search0, search1);

              search0 = header.indexOf("Grn=") + 4;
              search1 = header.indexOf("&", search0 + 1);
              String Grn = header.substring(search0, search1);

              search0 = header.indexOf("Blu=") + 4;
              search1 = header.indexOf(" HTTP", search0 + 1);
              String Blu = header.substring(search0, search1);

              //Serial.printf("Color Selected = Red: .%s. \tGreen: .%s.\tBlue: .%s.\n", Red.c_str(), Grn.c_str(), Blu.c_str());

              if (Red.length() > 0 && Grn.length() > 0 && Blu.length() > 0)
                if (Red.length() < 4 && Grn.length() < 4 && Blu.length() < 4)  {
                  int R = Red.toInt();
                  int G = Grn.toInt();
                  int B = Blu.toInt();
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
