
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

      //data cascades through 16 bits at a time
      //when more then 16 bits are sent, the first bits start being pushed out to the next chip
      // c2       c1
      // 00000000 01010001 << 10000001
      // 01010001 10000001

const int number_matrix_format[][8] = {
  {0x00,0x7e,0xc3,0x81,0x81,0xc3,0x7e,0x00}, //0
  {0x01,0x01,0xff,0xff,0x81,0x41,0x21,0x11}, //1
  {0x00,0x61,0x91,0x89,0x85,0x83,0x41,0x00}, //2
  {0x00,0x6e,0x91,0x91,0x91,0x91,0x91,0x00}, //3
  {0x00,0x08,0x08,0xff,0x48,0x28,0x18,0x08}, //4
  {0x00,0x8e,0x91,0x91,0x91,0x91,0xf1,0x00}, //5
  {0x00,0x8e,0x91,0x91,0x91,0xcb,0x7e,0x00}, //6
  {0x80,0xc0,0xa0,0x90,0x88,0x84,0x82,0x81}, //7
  {0x00,0x6e,0x91,0x91,0x91,0x91,0x6e,0x00}, //8
  {0x00,0x6f,0x90,0x90,0x90,0x90,0x60,0x00}, //9
  };

class LedMatrix {
  private:
    uint16_t cs = 7;
    uint8_t matrix_count = 1;
  public:
    void send_data(uint8_t address, uint8_t value,uint8_t matrix_index){
      digitalWrite(cs, LOW);//tell chip data is transfering
      SPI.transfer(address);
      SPI.transfer(value);
      digitalWrite(cs,HIGH); //latch data
      delay(5);
      //delay(2);
    }
    //constructor
    LedMatrix(uint16_t cs_pin, uint8_t p_matrix_count){
      cs = cs_pin;
      matrix_count = p_matrix_count;
    }
    void begin(uint8_t matrix_index){
      send_data(INTENSITY,0x0f, matrix_index);
      send_data(SCAN_LIMIT,0x07, matrix_index); //display all dots
      send_data(DECODE_MODE,0x00, matrix_index); //directly address pixels
      on(matrix_index);
    }
    void off(uint8_t matrix_index){
      send_data(SHUTDOWN,0x00,matrix_index);
    }
    void on(uint8_t matrix_index){
      send_data(SHUTDOWN,0x01,matrix_index);
    }
    void set_column(uint8_t column,uint8_t bitmask, uint8_t matrix_index){ //or together R_* to make a column
      send_data(column+1,bitmask, matrix_index);      
    }
    void display_number(uint8_t number,uint8_t matrix_index){
      for (uint8_t i = 0; i < 8; i++){
        set_column(i,number_matrix_format[number][i],matrix_index);
        delay(3);
      }
    }
  };
static LedMatrix led_matrix(8,4);
static DS1307 RTC;
void setup() {
  SPI.begin();
  SPI.setBitOrder(MSBFIRST); //big endian
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  delay(100);
  //================== init the led matrixes ============
  pinMode(CS,OUTPUT);
  digitalWrite(CS,HIGH);
  led_matrix.begin(0);
  //led_matrix.begin(1);
  Serial.begin(9600);
  Serial.println("Starting");
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
}

void loop() {
  for (int n = 0;n < 10;n++){
    led_matrix.display_number(1,0);
    delay(500);
  }
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
