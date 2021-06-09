#include<QueueList.h>

#define SLAVE_ID (1)

HardwareSerial Serial2 (USART2);
HardwareSerial RS485(USART3);

//RS485 related
#define SEND_MODE (0)
#define READ_MODE (1)
#define RE (PB1)
#define DE (PB0)
#define BAUD_RATE (9600)
#define inputSerial (RS485)
#define LIGHT_UP (LOW)
#define LED (PB14)
//#define inputSerial (Serial2)
//#define inputSerial (Serial) //RS232

enum State {
  Idle = 0,
  FlashLED = 1,
};

typedef struct Command {
  char mode = 0;
  int32_t params[4] = {0};
} Cmd;

QueueList <Cmd> cmdQueue;
State curState;
char buf[512];
int dataIndex = 0;
uint32_t idleTimer;
uint32_t ledTimer;
uint8_t ledState = 1 - LIGHT_UP;

int32_t numFlash = 0;
int32_t flashInterval = 0;

void setRS485Mode(int8_t mode) {
  if(mode == SEND_MODE) {
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
  }
  else if(mode == READ_MODE) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
  }
}

void setState(State state) {
  if (state == Idle) {
    idleTimer = millis();
  }
  curState = state;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUD_RATE);
  Serial2.begin(BAUD_RATE);
  RS485.begin(BAUD_RATE);
  pinMode(LED, OUTPUT);
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  digitalWrite(LED, ledState);
  
  setRS485Mode(READ_MODE);
  setState(Idle);
}

void executeCmd(Cmd cmd) {
  if(cmd.mode == 'l') { //l,slaveID,numFlash,flashInterval(millis)
    numFlash = cmd.params[0];
    flashInterval = cmd.params[1];
    ledTimer = 0;
    setState(FlashLED);
  }
}

void parseBuffer() {
  Cmd cmd;
  const char* token = strtok(buf, ",");
  int index = 0;

  while(token != NULL) {
    //Serial.println(token);    
    if(index == 0) {
      cmd.mode = token[0];
    }
    else if(index == 1) {
      int slaveID = atoi(token);
      if(slaveID != SLAVE_ID) {
        //the command is not for this slave
        cmd.mode = 0;
        break;
      }
    }
    else if(index >= 2 && index <= 5) {
      cmd.params[index - 2] = atoi(token);
    }
    else {
      //out of params capacity
      break;
    }

    index++;
    token = strtok(NULL, ",");
  }

  if(cmd.mode != 0) {
    if(curState == Idle) {
       executeCmd(cmd);
    }
    else {
       cmdQueue.push(cmd); 
    }
  }
}

void emptyCmdQueue() {
  while (!cmdQueue.isEmpty ())
    cmdQueue.pop();
}

bool canPutIntoBuffer(char c) {
  return (dataIndex < sizeof(buf) - 1) &&
  (c >= 'a' && c <= 'z') ||
  (c >= 'A' && c <= 'Z') ||
  (c >= '0' && c <= '9') ||
  c == ',' ||
  c == '-';
}

void loop() {
  //parse input serial data
  if(inputSerial.available() > 0) {
    char c = inputSerial.read();
    if(c == '\n') {
      buf[dataIndex] = 0;
      parseBuffer();
      dataIndex = 0;
    }
    else if(canPutIntoBuffer(c)){
      buf[dataIndex] = c;
      dataIndex++;
    }
  }

  if(curState == Idle) {
    if(!cmdQueue.isEmpty()) {
      executeCmd(cmdQueue.pop());
    }
  }
  else if(curState == FlashLED) {
    if(ledTimer == 0) {
      if(numFlash > 0) {
        ledState = LIGHT_UP;
        digitalWrite(LED, ledState);
        ledTimer = millis();
        numFlash--;
      }
      else {
        setState(Idle);
      }
    }
    else if(millis() - ledTimer >= flashInterval){
      if(ledState == LIGHT_UP) {
        ledState = 1 - LIGHT_UP;
        digitalWrite(LED, ledState);
        ledTimer = millis();
      }
      else {
        ledTimer = 0;
      }
    }
  }
}
