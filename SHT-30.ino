typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

#define SCL 2
#define SDA 3
#define ADDR 0x44
#define ACK 0x00
#define NACK 0x01

//#define INTERVAL uint(1572) // 1kHz
#define INTERVAL uint(2000) // 1kHz

static uchar data[6] = {0};


void setup() {
  uint cnt = 0;

  Serial.begin(9600);
  digitalWrite(SDA, HIGH);
  pinMode(SDA, OUTPUT);
  digitalWrite(SCL, HIGH);
  pinMode(SCL, OUTPUT);
  delay(1);
  ss_write(ADDR, 0x30A2); // soft reset
  delay(1);
  ss_write(ADDR, 0xF32D); // read status registaer
  delay(1);
  ss_read(ADDR, 3, data);
  delay(5000);

}

void loop() {
  uint cnt = 0;

  for(cnt = 0; cnt < 6; cnt++)data[cnt] = 0;

  ss_write(ADDR, 0x2400); // read data single shot
  delay(20);
  ss_read(ADDR, 6, data);
  delay(1);

  delay(5000);
}

void ss_write(uchar addr, uint data){
  uchar cnt;

  addr = addr << 1; // LSB means write

  ss_startbit();
  ss_send(addr);
  ss_send( uchar( data >> 8 ) );
  ss_send( uchar( data ) );
  ss_stopbit();

}

// retun 0:true,1:ack error
uchar ss_send(uchar data){
  uchar cnt = 0;
  uchar result = 0;

  for(cnt = 0; cnt <8; cnt++){
    digitalWrite(SDA, ( data >> 7 ) );
    data <<= 1;
    ss_oneclock();
  }
  pinMode(SDA, INPUT);
  ss_oneclock();
  result = digitalRead(SDA);
//  delay(1);
  pinMode(SDA, OUTPUT);

  return !result;
}

void ss_read(uchar addr, uchar num, uchar *data){
  uchar cnt;
  uint var = 0;

  addr = (addr << 1) | 0x01; // LSB means read

  ss_startbit();
  ss_send(addr);
  for(cnt = 0; cnt < ( num - 1 ); cnt++){
    ss_read_byte(ACK, &(data[cnt]));
  }
  ss_read_byte(NACK, &data[cnt]);
  Serial.println(float( ( ( ( data[0] << 8 ) | data[1] ) * 175.0 ) / ( 0xFFFF - 1 ) - 45) );
  Serial.println(float( ( ( ( data[3] << 8 ) | data[4] ) * 100.0 ) / ( 0xFFFF - 1 ) ));

  ss_stopbit();

}

uchar ss_read_byte(uchar ack, uchar *data){
  uchar cnt = 0;
  uchar result = 0;
  uchar test = 0;

  pinMode(SDA, INPUT);
  for(cnt = 0; cnt <8; cnt++){
//    *data |= ss_oneclock();
//    *data <<= 1;
    *data |= ss_oneclock() << (7 - cnt);
//    test |= ss_oneclock() << (7 - cnt);
//    Serial.println(ss_oneclock() );
//    test |= PIND & _BV(SDA);
//    test <<= 1;
  }
//  Serial.println(test, HEX);
  test = 0;

  digitalWrite(SDA, ack);
  pinMode(SDA, OUTPUT);
  result = ss_oneclock();
//  delay(1);

  return !result;
}

void ss_startbit(){
  uint cnt = 0;

  digitalWrite(SDA, LOW);
  for(cnt = 0; cnt < INTERVAL; cnt++ ){
    asm("nop");
  }
  digitalWrite(SCL, LOW);
  for(cnt = 0; cnt < INTERVAL; cnt++ ){
    asm("nop");
  }
}

void ss_stopbit(){
  uint cnt = 0;

  digitalWrite(SCL, HIGH);
  for(cnt = 0; cnt < INTERVAL; cnt++ ){
    asm("nop");
  }
  digitalWrite(SDA, HIGH);
  for(cnt = 0; cnt < INTERVAL; cnt++ ){
    asm("nop");
  }
}

uchar ss_oneclock() {
  uint cnt = 0;
  uchar var = 0;

  digitalWrite(SCL, HIGH);
  for(cnt = 0; cnt < INTERVAL; cnt++){
    asm("nop");
  }
  var = digitalRead(SDA);
  digitalWrite(SCL, LOW);
  for(cnt = 0; cnt < INTERVAL; cnt++){
    asm("nop");
  }

  return var;
}
