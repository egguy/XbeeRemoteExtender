#include <XBee.h>

//#define S1 1
#define S2 1
#define IN1 2
#define IN2 3

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

//volatile 
int inputstate = LOW;
//volatile boolean updated = false; 
int lastState = LOW;

// create the XBee object
XBee xbee = XBee();

uint8_t payload[] = { 0 };

// address of the door opener
// SH + SL Address of receiving XBee
XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x40A0A3F3);


// Set DIO0 (pin 20) to Analog Input
//uint8_t d0Cmd[] = { 'D', '0' };
//uint8_t d0Value[] = { 0x4 };

// Create a remote AT request with the IR command
//RemoteAtCommandRequest remoteAtRequest = RemoteAtCommandRequest(remoteAddress, d0Cmd, d0Value, sizeof(d0Value));



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
int b1_state = HIGH;
int b2_state = HIGH;

void setup(){
  Serial.begin(9600);
  //xbee.begin(Serial);
  //delay(5000);
  pinMode(13, OUTPUT);
  pinMode(IN1, INPUT_PULLUP);
  pinMode(IN2, INPUT_PULLUP);
  //digitalWrite(2, HIGH);
  //attachInterrupt(0, stateChange, CHANGE);
}

void loop() {
  //int state = digitalRead(2);
  //sendRemoteAtCommand();

    //Serial.print("Status is: ");
    //Serial.print((inputstate==LOW)?"LOW": "HIGH");
    //Serial.println(inputstate);
    //delay(100);

  inputstate = digitalRead(IN1);
  if(inputstate == LOW && b1_state == HIGH){
    payload[0] = 0x01;
    sendPayload(payload, 1);
    b1_state = LOW;
  } else if(inputstate == HIGH  && b1_state == LOW){ 
    b1_state = HIGH; 
  }

  inputstate = digitalRead(IN2);
  if(inputstate == LOW && b2_state == HIGH){
    payload[0] =  0x02;
    sendPayload(payload, 1);
    b2_state = LOW;
  } else if(inputstate == HIGH  && b2_state == LOW){ 
    b2_state = HIGH; 
  }

  /*if(lastState != inputstate){
    lastState = inputstate;
    
    //d0Value[0] = (inputstate == LOW? 0x04: 0x05);
    
    //sendATCommand(d0Cmd, d0Value, sizeof(d0Value));
    
    payload[0] = inputstate & 0xff;
    sendPayload(payload, 1);
    //digitalWrite(13, inputstate);
  }*/
  


  // we're done
  //while (1) {};
}


void sendATCommand(uint8_t *cmd, uint8_t *payload, int sizeOfPayload){

  // Create a remote AT request with the IR command
  RemoteAtCommandRequest remoteAtRequest = RemoteAtCommandRequest(addr64, cmd, payload, sizeOfPayload);
  // Create a Remote AT response object
  RemoteAtCommandResponse remoteAtResponse = RemoteAtCommandResponse();
  
  xbee.send(remoteAtRequest);
  
   // wait up to 5 seconds for the status response
  if (xbee.readPacket(5000)) {
    // got a response!

    // should be an AT command response
    if (xbee.getResponse().getApiId() == REMOTE_AT_COMMAND_RESPONSE) {
      xbee.getResponse().getRemoteAtCommandResponse(remoteAtResponse);

      if (remoteAtResponse.isOk()) {
        flashLed(13, 5, 50);
        if (remoteAtResponse.getValueLength() > 0) {
          
          for (int i = 0; i < remoteAtResponse.getValueLength(); i++) {
            // Read responses
          }
        }
      } else {
      }
    } else {
    }    
  } else if (xbee.getResponse().isError()) {
    //nss.print("Error reading packet.  Error code: ");  
    //nss.println(xbee.getResponse().getErrorCode());
  } else {
    //nss.print("No response from radio");  
  }
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

