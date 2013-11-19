#include <XBee.h>

#define S1 1
//#define S2 1

// portability macro
#ifdef S1
#define STATUS_RESPONSE TX_STATUS_RESPONSE
#define request(addr64, payload, sizeofPayload) Tx64Request tx = Tx64Request(addr64, payload, sizeofPayload)
#define getStatus() txStatus.getStatus()
#define TXStatusResponse(txStatus) xbee.getResponse().getTxStatusResponse(txStatus)
#endif

#ifdef S2
#define STATUS_RESPONSE ZB_TX_STATUS_RESPONSE
#define request(addr64, payload, sizeofPayload) ZBTxRequest tx = ZBTxRequest(addr64, payload, sizeofPayload)
#define getStatus() txStatus.getDeliveryStatus()
#define TXStatusResponse(txStatus)  xbee.getResponse().getZBTxStatusResponse(txStatus)
#endif

volatile int inputstate = LOW;
//volatile boolean updated = false; 
int lastState = LOW;

// create the XBee object
XBee xbee = XBee();

uint8_t payload[] = { 0 };

// address of the door opener
// SH + SL Address of receiving XBee
XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x406A3A0A);


#ifdef S1
TxStatusResponse txStatus = TxStatusResponse();
#endif
#ifdef S2
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
#endif

// Create a Remote AT response object
//RemoteAtCommandResponse remoteAtResponse = RemoteAtCommandResponse();

int statusLed = 11;
int errorLed = 12;

void setup(){
  Serial.begin(9600);
  //xbee.begin(Serial);
  //delay(5000);
  pinMode(13, OUTPUT);
  pinMode(2, INPUT_PULLUP);
  //digitalWrite(2, HIGH);
  attachInterrupt(0, stateChange, CHANGE);
}

void loop() {
  //int state = digitalRead(2);
  //sendRemoteAtCommand();

    //Serial.print("Status is: ");
    //Serial.print((inputstate==LOW)?"LOW": "HIGH");
    //Serial.println(inputstate);
    //delay(100);


  if(lastState != inputstate){
    lastState = inputstate;
    
    payload[0] = inputstate & 0xff;
    sendPayload(payload, 1);
    //digitalWrite(13, inputstate);
  }
  


  // we're done
  //while (1) {};
}





void sendPayload(uint8_t *payload, int sizeofPayload){
  
  request(addr64, payload, sizeofPayload);
  
  xbee.send(tx);

  // flash TX indicator
  flashLed(statusLed, 1, 100);

  // after sending a tx request, we expect a status response
  // wait up to half second for the status response
  if (xbee.readPacket(500)) {
    // got a response!

    // should be a znet tx status    	
    if (xbee.getResponse().getApiId() == STATUS_RESPONSE) {
      TXStatusResponse(txStatus);
    
      // get the delivery status, the fifth byte

      if (getStatus() == SUCCESS) {
        Serial.println("Success");
        // success.  time to celebrate
        flashLed(13, 5, 50);
      } else {
        // the remote XBee did not receive our packet. is it powered on?
        //flashLed(errorLed, 3, 500);
      }
    }
  } else if (xbee.getResponse().isError()) {
    //nss.print("Error reading packet.  Error code: ");  
    //nss.println(xbee.getResponse().getErrorCode());
  } else {
    // local XBee did not provide a timely TX Status Response -- should not happen
    flashLed(errorLed, 2, 50);
  }

  delay(1000);
}

void flashLed(int pin, int times, int wait) {
    
    for (int i = 0; i < times; i++) {
      digitalWrite(pin, HIGH);
      delay(wait);
      digitalWrite(pin, LOW);
      
      if (i + 1 < times) {
        delay(wait);
      }
    }
}

void stateChange(){
  //Serial.println("Intterupted");
  inputstate = !inputstate;
} 

