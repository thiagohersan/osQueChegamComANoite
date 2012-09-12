//receiver

#include <VirtualWire.h>  // you must download and install the VirtualWire.h to your hardware/libraries folder
#undef int
#undef abs
#undef double
#undef float
#undef round

/////
#define MY_ID 0x0

#define STATE_RECEIVE 0x0
#define STATE_RANDOM 0x1

#define COMMAND_RECEIVE 0x25  // b 0010_0101
#define COMMAND_RANDOM 0xAB   // b 1010_1011

unsigned int currentState;
unsigned long lastUpdated;

int go;
int current;
int relay_a = 9;

void setup() {
  Serial.begin(9600);    
  pinMode(relay_a, OUTPUT);

  // Initialise the IO and ISR
  vw_set_ptt_inverted(true);    // Required for RX Link Module
  vw_setup(1200);               // Bits per sec
  vw_set_rx_pin(11);            // We will be receiving on pin 11 (ie the RX pin from the module connects to this pin)
  vw_rx_start();                // Start the receiver 

  current = go = 0;
  currentState = STATE_RECEIVE;
  lastUpdated = millis();
}

// count number of 1 bits in a char
unsigned char numOnes(unsigned char c){
  unsigned char result = 0;
  for(unsigned char i = 0; i<8; i++){
    result += (c>>i)&0x1;
  }
  return result;
}

void loop(){
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;


  // check to see if anything has been received
  if ((currentState == STATE_RECEIVE) && (vw_get_message(buf, &buflen))) {
    Serial.print("Received something, buflen = ");
    Serial.println(buflen);
    if(buflen >= 4){
      unsigned char cmd = (buf[0]>>1)&0x7F;
      unsigned char onOff = (buf[0])&0x1;
      unsigned char id = (buf[1])&0xFF;
      unsigned char checksum = (buf[2])&0xFF;
      unsigned char sum = numOnes(buf[0]) + numOnes(buf[1]);

      // Message with a good checksum received
      if(sum == checksum){
        Serial.print("comamnd: ");
        Serial.print(cmd);
        Serial.print(" on/off: ");
        Serial.print(onOff);
        Serial.print(" ID: ");
        Serial.print(id);
        Serial.print(" checksum: ");
        Serial.print(checksum);
        Serial.print("\n");

        // check for command to enable random mode
        if(buf[0] == COMMAND_RANDOM){
          currentState = STATE_RANDOM;
        }
        // receive command
        else if((cmd == COMMAND_RECEIVE) && (id == MY_ID)){
          // set signal for on/off
          go = onOff;
        }
      }
    }
    //
    lastUpdated = millis();
  }
  // if in random mode, 
  //    pick a random value for on/off variable about every 5 seconds
  else if(currentState == STATE_RANDOM){
    if((millis() - lastUpdated) > random(4000,8000)){
      go = (int)(random(0,2));
      lastUpdated = millis();
    }
  }
  // if it's been a long time since we've seen an update (5 minutes)
  //    we're in RECEIVE mode, but not receiving
  else if((millis() - lastUpdated) > 300000){
    currentState = STATE_RANDOM;
    go = (int)(random(0,2));
    lastUpdated = millis();
  }

  // always do this. even if in random mode. 
  if(go != current){
    Serial.println("go != current");
    if(go == 0){
      Serial.println("turn relay OFF");
      digitalWrite(relay_a, LOW);   // relay coil off    
      current = 0;
    }
    else if(go == 1){
      Serial.println("turn relay ON");
      digitalWrite(relay_a, HIGH);   // relay coil on       
      current = 1;
    }
  }

}











