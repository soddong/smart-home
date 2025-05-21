/* Include for IR */
#include <IRremote.h>
#include <IRremote.hpp>
#include "RemoteSignal.h"

/* Include for Temp-Hum */
#include <AM2302-Sensor.h>

/* Pin Info */
#define P_IR_Receiver   8
#define P_IR_Sender     9
#define P_TempHum       10
#define P_IR_Sender_SW  11

/* Define Macros */
#define BUF_SIZE        200
#define IR_DECODE_MODE  false

/* Global Variables */
AM2302::AM2302_Sensor TempHum{P_TempHum};
volatile int consol_debug_cnt;
volatile float tmp;
volatile float hum;

/* Function Prototypes */
void IR_Receiver_Main(void);
void IR_Sender_Main(const uint16_t[]);
void TempHum_Main(int);
void WiFi_Main(void); /* TODO: Need to implement */

void print_decode_info(IRrecv);
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
}

void IR_Receiver_Main(){
  if (IrReceiver.decode()){
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
      handleOverflow();
    }
    else{
      /* No overflow here */
      if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
        /* Print decoded signals */
        Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
        print_decode_info(IrReceiver);
        print_raw_data(IrReceiver);
      }
      /* Prepare for next decoding */
      IrReceiver.resume();
    }
  }
}

void IR_Sender_Main(const uint16_t ir_signal[]){
  /* Send Signal with 38KHz */
  Serial.println("Sending Carrier IR signal...");
  IrSender.sendRaw(ir_signal, sizeof(ir_signal) / sizeof(ir_signal[0]), NEC_KHZ);
}

void TempHum_Main(int sampling_period){
  int8_t status = TempHum.read();
  if(status == AM2302::AM2302_READ_OK){
    tmp = TempHum.get_Temperature();
    hum = TempHum.get_Humidity();

    Serial.print("Status: ");
    Serial.println(AM2302::AM2302_Sensor::get_sensorState(status));
    Serial.print("Temp: ");
    Serial.print(tmp);
    Serial.print(", Hum: ");
    Serial.println(hum);
    delay(sampling_period);
  }
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
  Serial.println(F("Overflow detected..."));
  /* Do something... */
}