/* Include for IR */
#include <IRremote.h>
#include <IRremote.hpp>
#include "RemoteSignal.h"

/* Include for Temp-Hum */
#include <AM2302-Sensor.h>

/* Include for WiFi */
#include <WiFi.h>
#include <PubSubClient.h>

/* Pin Info */
#define P_IR_Receiver   8
#define P_IR_Sender     9
#define P_TempHum       10
#define P_IR_Sender_SW  11

/* Define Macros */
#define IR_DECODE_MODE  false
#define MQTT_PORT       1883
#define MQTT_BUF_SIZE   50

/* Global Variables */
AM2302::AM2302_Sensor TempHum{P_TempHum};
WiFiClient espClient;
PubSubClient mqttClient(espClient);

volatile int consol_debug_cnt;
volatile int tmp;
volatile int hum;

const char* ssid = "soddong2";
const char* password = "tvxq1226";
const char* mqtt_server = "broker.mqtt-dashboard.com";

volatile int value;
volatile long lastMsg_MQTT;
volatile long lastMsg_TMPHUM;
char msg[MQTT_BUF_SIZE];

/* Function Prototypes */
void IR_Receiver_Main(void);
void IR_Sender_Main(const uint16_t[]);
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
  IrSender.begin(P_IR_Sender);
  IrReceiver.begin(P_IR_Receiver);
  TempHum.begin();

  /* Setup MQTT */
  MQTT_Init();

  /* Print IR Info */
  printActiveIRProtocols(&Serial);
}

void loop() {
  /* Main function for IR Receiver(Decoder) */
  #if IR_DECODE_MODE
  IR_Receiver_Main();
  #endif

  /* Main function for Temp-Hum Sensor*/
  TempHum_Main(2000);

  /* Main function for IR Sender */
  if(digitalRead(P_IR_Sender_SW) == LOW){
    IR_Sender_Main(decoded_on_data_17_raw);
  }

  /* Main function for MQTT */
  MQTT_Main();
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

void IR_Sender_Main(const uint16_t ir_signal[]){
  /* Send Signal with 38KHz */
  Serial.println(F("Sending Carrier IR signal..."));
  IrSender.sendRaw(ir_signal, sizeof(ir_signal) / sizeof(ir_signal[0]), NEC_KHZ);
}

void TempHum_Main(int sampling_period){
  unsigned long now = millis();
  if (now - lastMsg_TMPHUM > sampling_period) {
    lastMsg_TMPHUM = now;
    int8_t status = TempHum.read();
    if(status == AM2302::AM2302_READ_OK){
      tmp = (TempHum.get_Temperature());
      hum = (TempHum.get_Humidity());

      Serial.print(F("Status: "));
      Serial.println(F(AM2302::AM2302_Sensor::get_sensorState(status)));
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
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }

  /* Get seed from current time */
  randomSeed(micros());

  /* Print WiFi Info */
  Serial.println("");
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
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
    ++value;
    snprintf (msg, MQTT_BUF_SIZE, "Current Temp Hum : #%d, #%d", tmp, hum);
    Serial.print(F("Publish message: "));
    Serial.println(F(msg));
    mqttClient.publish("Starry/Test/Pub", msg);
  }
}

void MQTT_Receive(char* topic, byte* payload, unsigned int length){
  Serial.print(F("Message arrived ["));
  Serial.print(F(topic));
  Serial.print(F("] "));
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");

  if ((char)payload[0] == '1') {
    /* Do something... */
  } else {
    /* Do something... */
  }
}

void MQTT_Reconnect(void) {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Create a random client ID
    String clientId = "Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println(F("connected"));
      // Once connected, publish an announcement...
      mqttClient.publish("Starry/Test/Pub", "Reconnect");
      // ... and resubscribe
      mqttClient.subscribe("Starry/Test/Sub");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(F("try again"));
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