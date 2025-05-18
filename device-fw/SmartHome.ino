/* Include for IR */
#include <IRremote.h>
#include <IRremote.hpp>
#include "RemoteSignal.h"

/* Include for Temp-Hum */
#include <AM2302-Sensor.h>

/* Port Info */
#define P_IRsensor    8
#define P_TempHum     9
#define DEBUG_LED     10

/* Define Macros */
#define BUF_SIZE      200
#define DEBUG_DELAY   200

/* Global Variables */
int consol_debug_cnt;
int data_buffer[BUF_SIZE];
float tmp;
float hum;
AM2302::AM2302_Sensor am2302{P_TempHum};

/* Function Prototypes */
void print_debug();
void print_decode_info(IRrecv);
void print_raw_data(IRrecv);
void handleOverflow();

void setup() {
  /* Setup Pin Mode */
  pinMode(DEBUG_LED, OUTPUT);

  /* Setup Serial Baud Rate */
  Serial.begin(9600);
  while(!Serial);

  IrReceiver.begin(P_IRsensor);
  am2302.begin();

  printActiveIRProtocols(&Serial);
}

void loop() {
  /* Logic for IR decoder */
  if (IrReceiver.decode()){
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
      handleOverflow();
    }
    else{
      /* No overflow */
      if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
        Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
        print_decode_info(IrReceiver);
        print_raw_data(IrReceiver);
      }
      IrReceiver.resume();
    }
  }

  /* Logic for Temp-Hum Sensor */
  auto status = am2302.read();

  tmp = am2302.get_Temperature();
  hum = am2302.get_Humidity();

  Serial.print("Status: ");
  Serial.println(AM2302::AM2302_Sensor::get_sensorState(status));
  Serial.print("Temp: ");
  Serial.print(tmp);
  Serial.print(", Hum: ");
  Serial.println(hum);
  delay(2000);

}

void print_debug(){
  Serial.println("*********** Debug Code Activates ***********");
  int cnt = 0;
  do {
      digitalWrite(DEBUG_LED, HIGH);
      delay(DEBUG_DELAY);
      digitalWrite(DEBUG_LED, LOW);
      delay(DEBUG_DELAY);
      cnt++;
  } while(cnt <= 3);
}

void print_decode_info(IRrecv irrecv){
  consol_debug_cnt++;
  Serial.print("*********** Decode Info[");
  Serial.print(consol_debug_cnt);
  Serial.println("] ***********");

  /* decode_type_t protocol; */
  Serial.print("Protocol: ");
  Serial.println(IrReceiver.decodedIRData.protocol);
  
  /* uint16_t address; */
  Serial.print("Address: ");
  Serial.println(IrReceiver.decodedIRData.address, HEX);

  /* uint16_t command; */
  Serial.print("Command: ");
  Serial.println(IrReceiver.decodedIRData.command, HEX);

  /* uint16_t extra; */
  Serial.print("Extra: ");
  Serial.println(IrReceiver.decodedIRData.extra, HEX);

  /* IRRawDataType decodedRawData; */
  Serial.print("Decoded Raw Data: ");
  Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

  #if defined(DECODE_DISTANCE_WIDTH)
  /* DistanceWidthTimingInfoStruct DistanceWidthTimingInfo; */
  //Serial.print("Distance Width Timing Info: ");
  //Serial.println(IrReceiver.decodedIRData.DistanceWidthTimingInfo, HEX);

  /* IRRawDataType decodedRawDataArray[RAW_DATA_ARRAY_SIZE]; */
  int rawDataLength = 0;
  Serial.print("Decoded Raw Data Array: ");
  for(int idx = 0; idx < IrReceiver.decodedIRData.rawlen; idx++){
    Serial.print("[");
    Serial.print(IrReceiver.decodedIRData.decodedRawDataArray[idx], HEX);
    Serial.print("]");
    rawDataLength++;
  }
  Serial.print("\n[Check] Decoded Raw Data Array Length: ");
  Serial.println(rawDataLength);
  #endif

  /* uint16_t numberOfBits; */
  Serial.print("NumberOfBits: ");
  Serial.println(IrReceiver.decodedIRData.numberOfBits, HEX);

  /* uint8_t flags; */
  Serial.print("Flags: ");
  Serial.println(IrReceiver.decodedIRData.flags, HEX);

  /* IRRawlenType rawlen; */
  Serial.print("rawlen: ");
  Serial.println(IrReceiver.decodedIRData.rawlen, HEX);

  /* uint16_t initialGapTicks; */
  Serial.print("initialGapTicks: ");
  Serial.println(IrReceiver.decodedIRData.initialGapTicks, HEX);

  Serial.println("**************************************");
}

void print_raw_data(IRrecv irrecv){
  irrecv.printIRResultRawFormatted(&Serial, true);
}

void handleOverflow() {
  Serial.println(F("Overflow detected"));
  /* Do something... */
}