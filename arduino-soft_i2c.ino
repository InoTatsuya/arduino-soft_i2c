typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

#define ACK 0x00
#define NACK 0x01

struct str_i2c {
  uchar scl;
  uchar sda;
  uchar addr;
};

struct str_i2c i2c[5] = {
    {2,3,0x44}, // ch0
    {4,5,0x44}, // ch1
    {6,7,0x44}, // ch2
    {10,11,0x44}, // ch3
    {12,13,0x44} // ch4
  };

//#define INTERVAL uint(1572) // 1kHz
#define INTERVAL uint(2000) // 1kHz

static uchar data[6] = {0};


void setup() {
  uint cnt = 0;

  Serial.begin(9600);
  digitalWrite(i2c[0].sda, HIGH);
  pinMode(i2c[0].sda, OUTPUT);
  digitalWrite(i2c[0].scl, HIGH);
  pinMode(i2c[0].scl, OUTPUT);
  delay(1);
  ss_write(i2c[0].addr, 0x30A2); // soft reset
  delay(1);
//  ss_write(i2c[0].addr, 0xF32D); //
//  ss_read(i2c[0].addr, 3, data); // read status registaer 
  delay(5000);

}

void loop() {
  uint cnt = 0;

  for(cnt = 0; cnt < 6; cnt++)data[cnt] = 0;

  ss_write(i2c[0].addr, 0x2400); // read data single shot
  delay(20);
  ss_read(i2c[0].addr, 6, data);
  Serial.println(float( ( ( ( data[0] << 8 ) | data[1] ) * 175.0 ) / ( 0xFFFF - 1 ) - 45) );
  Serial.println(float( ( ( ( data[3] << 8 ) | data[4] ) * 100.0 ) / ( 0xFFFF - 1 ) ));

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
    digitalWrite(i2c[0].sda, ( data >> 7 ) );
    data <<= 1;
    ss_oneclock();
  }
  pinMode(i2c[0].sda, INPUT);
  ss_oneclock();
  result = digitalRead(i2c[0].sda);
//  delay(1);
  pinMode(i2c[0].sda, OUTPUT);

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
//  Serial.println(float( ( ( ( data[0] << 8 ) | data[1] ) * 175.0 ) / ( 0xFFFF - 1 ) - 45) );
//  Serial.println(float( ( ( ( data[3] << 8 ) | data[4] ) * 100.0 ) / ( 0xFFFF - 1 ) ));

  ss_stopbit();

}

uchar ss_read_byte(uchar ack, uchar *data){
  uchar cnt = 0;
  uchar result = 0;
  uchar test = 0;

  pinMode(i2c[0].sda, INPUT);
  for(cnt = 0; cnt <8; cnt++){
//    *data |= ss_oneclock();
//    *data <<= 1;
    *data |= ss_oneclock() << (7 - cnt);
//    test |= ss_oneclock() << (7 - cnt);
//    Serial.println(ss_oneclock() );
//    test |= PIND & _BV(i2c[0].sda);
//    test <<= 1;
  }
//  Serial.println(test, HEX);
  test = 0;

  digitalWrite(i2c[0].sda, ack);
  pinMode(i2c[0].sda, OUTPUT);
  result = ss_oneclock();
//  delay(1);

  return !result;
}

void ss_startbit(){
  uint cnt = 0;

  digitalWrite(i2c[0].sda, LOW);
  for(cnt = 0; cnt < INTERVAL; cnt++ ){
    asm("nop");
  }
  digitalWrite(i2c[0].scl, LOW);
  for(cnt = 0; cnt < INTERVAL; cnt++ ){
    asm("nop");
  }
}

void ss_stopbit(){
  uint cnt = 0;

  digitalWrite(i2c[0].scl, HIGH);
  for(cnt = 0; cnt < INTERVAL; cnt++ ){
    asm("nop");
  }
  digitalWrite(i2c[0].sda, HIGH);
  for(cnt = 0; cnt < INTERVAL; cnt++ ){
    asm("nop");
  }
}

uchar ss_oneclock() {
  uint cnt = 0;
  uchar var = 0;

  digitalWrite(i2c[0].scl, HIGH);
  for(cnt = 0; cnt < INTERVAL; cnt++){
    asm("nop");
  }
  var = digitalRead(i2c[0].sda);
  digitalWrite(i2c[0].scl, LOW);
  for(cnt = 0; cnt < INTERVAL; cnt++){
    asm("nop");
  }

  return var;
}
