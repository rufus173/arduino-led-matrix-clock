
#include <SPI.h>
#include <I2C_RTC.h>

#define CS 7
//CS pin 7
//DIN pin 11
//CLK pin 13

//control registers addresses
#define DECODE_MODE 9
#define INTENSITY 10
#define SCAN_LIMIT 11
#define SHUTDOWN 12
#define DISPLAY_TEST 16

#define R_EMPTY 0x00
#define R_FULL 0xff
#define R_1 0x01
#define R_2 0x02
#define R_3 0x04
#define R_4 0x08
#define R_5 0x10
#define R_6 0x20
#define R_7 0x40
#define R_8 0x80

class LedMatrix {
  private:
    uint16_t cs = 7;
    uint8_t matrix_index = 0;
  public:
    void send_data(uint8_t address, uint8_t value){
      digitalWrite(cs, HIGH);//tell chip data is transfering
      //data cascades through 16 bits at a time
      //when more then 16 bits are sent, the first bits start being pushed out to the next chip
      // c2       c1
      // 00000000 01010001 << 10000001
      // 01010001 10000001
      SPI.transfer(address);
      SPI.transfer(value);
      for (int i = 0;i < matrix_index;i++){ //matricies indexed from 0
        // "push" data through to correct chip
        SPI.transfer(0x00);
        SPI.transfer(0x00);
      }
      //latch data
      digitalWrite(cs, LOW);
      digitalWrite(cs,HIGH);
      //delay(2);
    }
    //constructor
    LedMatrix(uint16_t cs_pin, uint8_t p_matrix_index){
      cs = cs_pin;
      matrix_index = p_matrix_index;
      //digitalWrite(cs,HIGH);

    }
    void setup(){
      send_data(SCAN_LIMIT,0x07); //display all dots
      send_data(DECODE_MODE,0x00); //directly address pixels
      on();
    }
    void off(){
      send_data(SHUTDOWN,0x00);
    }
    void on(){
      send_data(SHUTDOWN,0x01);
    }
    void fill_display(){
      for (uint8_t i = 1; i < 9; i++){
        send_data(i,0xff);
      }
    }
    void empty_display(){
      for (uint8_t i = 1; i < 9; i++){
        send_data(i,0x00);
      }
    }
    void set_column(uint8_t column,uint8_t bitmask){ //or together R_* to make a column
      send_data(column+1,bitmask);      
    }
    void display_number(){
      led_matrix_1.set_column(0,R_FULL);
      led_matrix_1.set_column(1,R_FULL);
      led_matrix_1.set_column(2,R_1 | R_2 | R_5 | R_6);
      led_matrix_1.set_column(3,R_1 | R_2 | R_5 | R_6);
      led_matrix_1.set_column(4,R_3 | R_4 | R_7 | R_8);
      led_matrix_1.set_column(5,R_3 | R_4 | R_7 | R_8);
      led_matrix_1.set_column(6,R_EMPTY);
      led_matrix_1.set_column(7,R_EMPTY);
    }
  };
static LedMatrix led_matrix_1(7,1);
static LedMatrix led_matrix_2(7,1);
static LedMatrix led_matrix_3(7,2);
static LedMatrix led_matrix_4(7,3);
static DS1307 RTC;
void setup() {
  //================== init the led matrixes ============
  pinMode(CS,OUTPUT);
  SPI.setBitOrder(MSBFIRST); //big endian
  SPI.begin();
  Serial.begin(9600);
  Serial.println("Starting");
  delay(100);
  led_matrix_1.on();
  led_matrix_1.setup();
  led_matrix_1.send_data(INTENSITY,0x0f);//brightness
  led_matrix_1.fill_display();
  //================== init the rtc module ===============
  Serial.println("starting rtc module...");
  if (RTC.begin() == false){
    Serial.println("RTC module not found");
  }
  if (!RTC.isRunning()){
    Serial.println("starting rtc module clock");
    RTC.startClock();
  }
  Serial.println("starting displays...");
  //=============== display the letter "R" ================
  delay(500);
}

void loop() {
  delay(100);
  //=================== code for rtc module ==================
  Serial.print(RTC.getHours());
  Serial.print(" : ");
  Serial.println(RTC.getMinutes());
  // set the time from a unix timestamp over serial
  if (Serial.available() > 0){
    String buffer = Serial.readString();
    time_t timestamp = buffer.toInt();
    Serial.print("got timestamp: ");
    Serial.println(timestamp);
    RTC.setEpoch(timestamp);
  }
  //=================== code for led matrix ==================
}
