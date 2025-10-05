/* Include for IR */
#include <IRremote.h>
#include <IRremote.hpp>
#include "RemoteSignal.h"

/* Include for Temp-Hum */
#include <AM2302-Sensor.h>

/* Include for WiFi */
#include <WiFi.h>
#include <PubSubClient.h>
#include "WiFiSSLClient.h"
#include "Secret.h"

/* Pin Info */
#define P_IR_Receiver   8
#define P_IR_Sender     9
#define P_TempHum       10
#define P_IR_Sender_SW  11

/* Define Macros */
#define MQTT_PORT       8883
#define MQTT_BUF_SIZE   50
#define IR_DECODE_MODE  false

/* Global Variables */
WiFiSSLClient espClient;
PubSubClient mqttClient(espClient);
AM2302::AM2302_Sensor TempHum{P_TempHum};

String tmp_s;
String hum_s;
volatile int tmp;
volatile int hum;
volatile bool isAcOn = false;

const char* ssid = SECRET_SSID;
const char* pass = SECRET_PASS;
const char* mqtt_server = SECRET_SERV;

char mqtt_msg[MQTT_BUF_SIZE];
volatile long lastMsg_MQTT;
volatile long lastMsg_TMPHUM;
volatile long lastMsg_IRSignal;

/* Function Prototypes */
void IR_Receiver_Main(void);
void IR_Sender_On(void);
void IR_Sender_Off(void);
void IR_Sender_Main(void);
void TempHum_Main(int);
void MQTT_Init(void);
void MQTT_Main(void);
void MQTT_Reconnect(void);
void MQTT_Receive(char*, byte*, unsigned int);
void print_raw_data(IRrecv);
void handleOverflow();

void setup() {
  /* Setup Pin Mode */
  pinMode(P_IR_Sender_SW, INPUT_PULLUP);

  /* Setup Serial Baud Rate */
  Serial.begin(9600);
  while(!Serial);

  /* Enable sensors with pin info */
  TempHum.begin();
  IrSender.begin(P_IR_Sender);
  IrReceiver.begin(P_IR_Receiver);

  /* Setup MQTT */
  MQTT_Init();

  /* Print IR Info */
  printActiveIRProtocols(&Serial);
  Serial.println("");
}

void loop() {
  /* Main function for IR Receiver(Decoder) */
  #if IR_DECODE_MODE
  IR_Receiver_Main();
  #else
  /* Main function for IR Sender */
  if(digitalRead(P_IR_Sender_SW) == LOW){
    IR_Sender_Main();
  }
  /* Main function for Temp-Hum Sensor */
  TempHum_Main(2000);
  /* Main function for MQTT */
  MQTT_Main();
  #endif
}

void IR_Receiver_Main(void){
  if (IrReceiver.decode()){
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
      handleOverflow();
    }
    else{
      /* No overflow here */
      if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
        /* Print decoded signals */
        Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
        print_raw_data(IrReceiver);
      }
      /* Prepare for next decoding */
      IrReceiver.resume();
    }
  }
}

void IR_Sender_On(void){
  Serial.println(F("Send Turn On Signal"));
  IrSender.sendRaw(decoded_on_data_17_raw2, 
                   sizeof(decoded_on_data_17_raw2) / sizeof(decoded_on_data_17_raw2[0]), 
                   NEC_KHZ);
  isAcOn = true;
}

void IR_Sender_Off(void){
  Serial.println(F("Send Turn Off Signal"));
  IrSender.sendRaw(decoded_off_data_17_raw,
                   sizeof(decoded_off_data_17_raw) / sizeof(decoded_off_data_17_raw[0]), 
                   NEC_KHZ);
  isAcOn = false;
}

void IR_Sender_Main(void){
  unsigned long now = millis();
  if (now - lastMsg_IRSignal > 2000){
    lastMsg_IRSignal = now;
    if(isAcOn == false){
      IR_Sender_On();
    }else{
      IR_Sender_Off();
    }
  } 
}

void TempHum_Main(int sampling_period){
  unsigned long now = millis();
  if (now - lastMsg_TMPHUM > sampling_period) {
    lastMsg_TMPHUM = now;
    int8_t status = TempHum.read();
    if(status == AM2302::AM2302_READ_OK){
      tmp = TempHum.get_Temperature();
      hum = TempHum.get_Humidity();

      Serial.print(F("Status: "));
      Serial.println(AM2302::AM2302_Sensor::get_sensorState(status));
      Serial.print(F("Temp: "));
      Serial.print(tmp);
      Serial.print(F(", Hum: "));
      Serial.println(hum);
    }
  }
}

void MQTT_Init(void){
  /* Print Setup Info */
  Serial.print(F("Connecting to "));
  Serial.println(F(ssid));

  /* Try WiFi connection */
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }

  /* Setup Certificate for TLS */
  espClient.setCACert(ca_cert);

  /* Get seed from current time */
  randomSeed(micros());

  /* Print WiFi Info */
  Serial.println(F("WiFi connected"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());

  /* Setup MQTT as Client */
  mqttClient.setServer(mqtt_server, MQTT_PORT);
  mqttClient.setCallback(MQTT_Receive);
}

void MQTT_Main(void){
  /* If disconnected */
  if (!mqttClient.connected()) {
    MQTT_Reconnect();
  }
  mqttClient.loop();
  

  /* Publish Topics */
  volatile unsigned long now = millis();
  if (now - lastMsg_MQTT > 2000) {
    lastMsg_MQTT = now;
    //snprintf (mqtt_msg, MQTT_BUF_SIZE, "Current Temp Hum : #%d, #%d", tmp, hum);
    tmp_s = String(tmp);
    hum_s = String(hum);

    Serial.print(F("Publish message: "));
    Serial.println(F(mqtt_msg));
    mqttClient.publish("smarthome/tmp", tmp_s.c_str());
    mqttClient.publish("smarthome/hum", hum_s.c_str());
  }
}

void MQTT_Receive(char* topic, byte* payload, unsigned int length){
  /* Print received MQTT message */
  Serial.print(F("Message arrived ["));
  Serial.print(F(topic));
  Serial.print(F("] "));
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");

  /* Handle received message */
  if ((char)payload[0] == '1') {
    IR_Sender_On();
  } else {
    IR_Sender_Off();
  }
}

void MQTT_Reconnect(void) {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Create Random Client Account
    String clientName = "Client_";
    clientName += String(random(0xffff), HEX);

    const char* clientId = MQTT_CLIENT_ID;
    const char* clientPw = MQTT_CLIENT_PW;

    if (mqttClient.connect(clientName.c_str(), clientId, clientPw)) {
      Serial.println(F("connected"));
      // Once connected, publish an announcement...
      mqttClient.publish("Starry/Pub", "Reconnect");
      // ... and re-subscribe
      mqttClient.subscribe("Starry/Sub");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(F(", try again"));
    }
  }
}

void print_raw_data(IRrecv irrecv){
  irrecv.printIRResultRawFormatted(&Serial, true);
}

void handleOverflow() {
  Serial.println(F("Overflow detected..."));
  /* Do something... */
}