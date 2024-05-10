
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <espnow.h>
int button = 14;
int led = 12;
int button_state;
String app_button;
String state;
// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0X84,0XF3,0XEB,0XC9,0XAB,0X57};
String header;
// Variable to store if sending data was successful
String success;
WiFiServer server(80);
// Assign output variables to GPIO pins

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    String value;
} struct_message;

// Create a struct_message called buttonReadings to hold sensor readings
struct_message mobileapp_button;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  state = incomingReadings.value;
}

void setup(){
  // Set device as a Wi-Fi Station and access
  WiFi.mode(WIFI_AP_STA);
  // Init Serial Monitor
  Serial.begin(115200); 
  // Wifi access point credentials
  WiFi.softAP("ESPWebServe", "12345678");
  // Get IP address
  IPAddress IP = WiFi.softAPIP();
  WiFi.begin("ESPWebServe", "12345678");  
  while (WiFi.status() != WL_CONNECTED) {    
    delay(500);    
    Serial.print(".");  }  
    Serial.println("");
  Serial.print("AP IP address: ");
  // Print IP address
  Serial.println(IP);
  server.begin();
  
  pinMode(led,OUTPUT);
  pinMode(button,INPUT);

   // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
  }

void AP(){
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
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
            
            // turns the GPIOs on and off
            if (header.indexOf("/turnon") >= 0) {
              Serial.println("load on");           
              app_button="1";
            } else if (header.indexOf("turnoff") >= 0) {
              Serial.println("/load off");
             app_button="0";
            }
            
            /*** Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println("#headar{margin:1px auto 0 auto; min-width: 100px; max-width: 400px; min-height: 60px; max-height: 70px; background:#1E90FF;padding: 7px 3px; color:white;}");
            client.println("h1 { top: 40px; font-size: 200%; color: #F0FFFF; margin: auto 0 auto 20px; width: 487px;}");
            client.println(".h2{ top:30px; font-size:150%; color:#2F4F4F;font-family: sans-serif;margin: auto 0 auto 20px;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1 div id = \"headar\" >INTERNET OF THINGS</h1>");
            client.println("<body><h2 class=\"h2\"> CONTROL DASH BOARD</h2>");
            
            // Display current state, and ON/OFF buttons for GPIO 26  
            client.println("<p>Load - State " + state + "</p>");
            // If the output26State is off, it displays the ON button       
            if (state=="0") {
              
              client.println("<p><a href=\"/3/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/3/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            client.println("</body></html>"); ***/
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
  
  void loop(){
   AP();  
     //Set values to send
    mobileapp_button.value = app_button;    
    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &mobileapp_button, sizeof(mobileapp_button));
    // Print incoming readings
   
  }


