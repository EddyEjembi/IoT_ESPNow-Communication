
#include <ESP8266WiFi.h>
#include <espnow.h>
int button = 14;
int led = 4;
int button_state;
String led_state;
// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0XFC,0XF5,0XC4,0X96,0XEC,0X2B};

// Define variables to store current led state to be sent

String led_current_state;
// Define variables to store incoming button state readings
String button_incoming_state;

// Updates DHT readings every 10 seconds
const long interval = 10000; 
unsigned long previousMillis = 0;    // will store last time DHT was updated 

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    String value;
} struct_message;

// Create a struct_message called led_currentstate_Readings to hold led state readings to be sent
struct_message led_currentstate_Readings;

// Create a struct_message to hold incoming button readings
struct_message incoming_button_Readings;

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
  memcpy(&incoming_button_Readings, incomingData, sizeof(incoming_button_Readings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  led_state = incoming_button_Readings.value;
}

void get_led_currentstate(){
  led_current_state = digitalRead(led);
}

void execute_incoming_button_Readings(){
  if (led_state=="1"){
    digitalWrite(led,HIGH);
  }
  else{
    digitalWrite(led,LOW);
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  pinMode(led,OUTPUT);
  pinMode(button,INPUT);

 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

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
 
void loop() {
    
    //Get butto    nstate readings
   get_led_currentstate();
    //Set values to send
    led_currentstate_Readings.value = led_current_state;
   
    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &led_currentstate_Readings, sizeof(led_currentstate_Readings));

    // Print incoming readings
    execute_incoming_button_Readings();
}
