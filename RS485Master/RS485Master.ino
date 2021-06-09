HardwareSerial Serial2(USART2);//PA3,PA2(rx,tx)
HardwareSerial Serial3(USART3);//PB11,PB10(rx,tx)

#define PIN_DE (PB0)
#define PIN_RE (PB1)
#define RS485Transmit (HIGH)
#define RS485Receive (LOW)
#define RS485Serial (Serial3)
void setRS485Mode(int8_t rs485Mode) {
  digitalWrite(PIN_DE, rs485Mode);
  digitalWrite(PIN_RE, rs485Mode);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_DE, OUTPUT);
  pinMode(PIN_RE, OUTPUT);
  Serial.begin(9600);
  RS485Serial.begin(9600);
  Serial.println("rs485 master begin");
}

void loop() {
  setRS485Mode(RS485Transmit);
  while(Serial.available() > 0) {
    char c = Serial.read();
    RS485Serial.write(c);
  }
}
