typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

#define ACK 0x00
#define NACK 0x01
#define CH_MAX 5

struct str_i2c {
  uchar scl;
  uchar sda;
  uchar addr;
};

struct str_i2c i2c[CH_MAX] = {
    {2,3,0x44}, // ch0
    {4,5,0x44}, // ch1
    {6,7,0x44}, // ch2
    {10,11,0x44}, // ch3
    {8,9,0x44} // ch4
  };

#define INTERVAL uint(1000) // 1kHz

static uchar data[6] = {0};


void setup() {
  uint cnt = 0;

  Serial.begin(9600);

  for(cnt = 0; cnt < CH_MAX; cnt++){
    digitalWrite(i2c[cnt].scl, HIGH);
    pinMode(i2c[cnt].scl, OUTPUT);
    digitalWrite(i2c[cnt].sda, HIGH);
    pinMode(i2c[cnt].sda, OUTPUT);
    ss_write(cnt, 0x30A2); // soft reset
  }
//  ss_write(i2c[0].addr, 0xF32D);
//  ss_read(i2c[0].addr, 3, data); // read status registaer 
  delay(1000);

}

void loop() {
  uint cnt,j = 0;

  for(cnt = 0; cnt < CH_MAX; cnt++){
    for(j = 0; j < 6; j++)data[j] = 0;
    ss_write(cnt, 0x2400); // read data single shot
    delay(20);
    ss_read(cnt, 6, data);
    Serial.print(cnt);
    Serial.print(",");
//    Serial.print(float( ( ( ( data[0] << 8 ) | data[1] ) * 175.0 ) / ( 0xFFFF - 1 ) - 45) );
    Serial.print(ushort( ( data[0] << 8 ) | data[1] ));
    Serial.print(",");
//    Serial.println(float( ( ( ( data[3] << 8 ) | data[4] ) * 100.0 ) / ( 0xFFFF - 1 ) ));
    Serial.println(ushort( ( data[3] << 8 ) | data[4] ));
  }

  delay(5000);
}

void ss_write(uchar ch, uint data){
  uchar cnt;
  uchar addr;

  addr = i2c[ch].addr << 1; // LSB means write

  ss_startbit(ch);
  ss_send(ch, addr);
  ss_send(ch, uchar( data >> 8 ) );
  ss_send(ch, uchar( data ) );
  ss_stopbit(ch);

}

// retun 0:true,1:ack error
uchar ss_send(uchar ch, uchar data){
  uchar cnt = 0;
  uchar result = 0;

  for(cnt = 0; cnt <8; cnt++){
    digitalWrite(i2c[ch].sda, ( data >> 7 ) );
    data <<= 1;
    ss_oneclock(ch);
  }
  pinMode(i2c[ch].sda, INPUT);
  ss_oneclock(ch);
  result = digitalRead(i2c[ch].sda);
//  delay(1);
  pinMode(i2c[ch].sda, OUTPUT);

  return !result;
}

void ss_read(uchar ch, uchar num, uchar *data){
  uchar cnt;
  uint var = 0;
  uchar addr;

  addr = (i2c[ch].addr << 1) | 0x01; // LSB means read

  ss_startbit(ch);
  ss_send(ch, addr);
  for(cnt = 0; cnt < ( num - 1 ); cnt++){
    ss_read_byte(ch, ACK, &(data[cnt]));
  }
  ss_read_byte(ch, NACK, &data[cnt]);
  ss_stopbit(ch);

}

uchar ss_read_byte(uchar ch, uchar ack, uchar *data){
  uchar cnt = 0;
  uchar result = 0;
  uchar test = 0;

  pinMode(i2c[ch].sda, INPUT_PULLUP);
  for(cnt = 0; cnt <8; cnt++){
//    *data |= ss_oneclock();
//    *data <<= 1;
    *data |= ss_oneclock(ch) << (7 - cnt);
//    test |= ss_oneclock() << (7 - cnt);
//    Serial.println(ss_oneclock() );
//    test |= PIND & _BV(i2c[0].sda);
//    test <<= 1;
  }
//  Serial.println(test, HEX);
  test = 0;

  digitalWrite(i2c[ch].sda, ack);
  pinMode(i2c[ch].sda, OUTPUT);
  result = ss_oneclock(ch);
//  delay(1);

  return !result;
}

void ss_startbit(uchar ch){
  uint cnt = 0;

  digitalWrite(i2c[ch].sda, LOW);
  for(cnt = 0; cnt < INTERVAL; cnt++ ){
    asm("nop");
  }
  digitalWrite(i2c[ch].scl, LOW);
  for(cnt = 0; cnt < INTERVAL; cnt++ ){
    asm("nop");
  }
}

void ss_stopbit(uchar ch){
  uint cnt = 0;

  digitalWrite(i2c[ch].scl, HIGH);
  for(cnt = 0; cnt < INTERVAL; cnt++ ){
    asm("nop");
  }
  digitalWrite(i2c[ch].sda, HIGH);
  for(cnt = 0; cnt < INTERVAL; cnt++ ){
    asm("nop");
  }
}

uchar ss_oneclock(uchar ch) {
  uint cnt = 0;
  uchar var = 0;

  digitalWrite(i2c[ch].scl, HIGH);
  for(cnt = 0; cnt < INTERVAL; cnt++){
    asm("nop");
  }
  var = digitalRead(i2c[ch].sda);
  digitalWrite(i2c[ch].scl, LOW);
  for(cnt = 0; cnt < INTERVAL; cnt++){
    asm("nop");
  }

  return var;
}
