
#include <SPI.h>

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
  public:
    void send_data(uint8_t address, uint8_t value){
      digitalWrite(CS, LOW);//tell chip data is transfering
      SPI.transfer(address);
      SPI.transfer(value);
      digitalWrite(CS,HIGH);//tell chip data has stopped
    }
    //constructor
    LedMatrix(){
      //send_data(DISPLAY_TEST,0x00);//normal operation
      
    }
    void setup(){
      send_data(SCAN_LIMIT,0x07); //display all dots
      send_data(DECODE_MODE,0x00); //directly address pixels
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
  };
LedMatrix led_matrix;
void setup() {
  pinMode(CS,OUTPUT);
  SPI.setBitOrder(MSBFIRST); //big endian
  SPI.begin();
  Serial.begin(9600);
  while (!Serial){
    ;
  }
  Serial.println("Starting");
  led_matrix = LedMatrix();
  led_matrix.on();
  led_matrix.setup();
  led_matrix.send_data(INTENSITY,0x00);//brightness
  led_matrix.fill_display();
  delay(500);
  /*
  ####
  ####
  ##  ##
  ##  ##
  ## #
  ####
  ##  ##
  ##  ##
  */
  led_matrix.set_column(0,R_FULL);
  led_matrix.set_column(1,R_FULL);
  led_matrix.set_column(2,R_1 | R_2 | R_5 | R_6);
  led_matrix.set_column(3,R_1 | R_2 | R_5 | R_6);
  led_matrix.set_column(4,R_3 | R_4 | R_7 | R_8);
  led_matrix.set_column(5,R_3 | R_4 | R_7 | R_8);
  led_matrix.set_column(6,R_EMPTY);
  led_matrix.set_column(7,R_EMPTY);
  delay(500);
}

void loop() {
  for (;;){
    led_matrix.empty_display();
    delay(500);
    led_matrix.fill_display();
    delay(500);
  }
}
