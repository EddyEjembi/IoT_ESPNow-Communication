
#include <ESP8266WiFi.h>
#include <espnow.h>
int button = 14;
int led = 12;
int button_state;
// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0X84,0XF3,0XEB,0XC9,0XAB,0X57};

// Define variables to store button readings to be sent

int button_state;
// Define variables to store incoming led state readings
int state;

// Updates DHT readings every 10 seconds
const long interval = 10000; 
unsigned long previousMillis = 0;    // will store last time DHT was updated 

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    int value;
} struct_message;

// Create a struct_message called buttonReadings to hold sensor readings
struct_message buttonReadings;

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

void get_buttonstate(){
  button_state = digitalRead(button);
}

void printIncomingReadings(){
  if (state==HIGH){
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
    get_buttonstate();

    //Set values to send
    buttonReadings.value = button_state;
   
    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &buttonReadings, sizeof(buttonReadings));

    // Print incoming readings
    printIncomingReadings();
}
