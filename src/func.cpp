#include <Arduino.h>
#include <func.h>
#include <pin_define.h>
#include <BluetoothSerial.h>

BluetoothSerial bluetooth;

void forward(int left_speed, int right_speed){
  left_speed = max(left_speed, 0);
  right_speed = min(right_speed, 255);
  right_speed = max(right_speed, 0);
  ledcWrite(left_pwmA, left_speed);
  ledcWrite(left_pwmB, 0);
  ledcWrite(right_pwmA, right_speed);
  ledcWrite(right_pwmB, 0);
}

void backward(int left_speed, int right_speed){
  left_speed = max(left_speed, 0);
  right_speed = min(right_speed, 255);
  right_speed = max(right_speed, 0);
  ledcWrite(left_pwmB, left_speed);
  ledcWrite(left_pwmA, 0);
  ledcWrite(right_pwmB, right_speed);
  ledcWrite(right_pwmA, 0);
}

void turn_left(int speed){
  speed = min(speed, 255);
  speed = max(speed, 0);
  ledcWrite(left_pwmB, speed);
  ledcWrite(left_pwmA, 0);
  ledcWrite(right_pwmA, speed);
  ledcWrite(right_pwmB, 0);
}
void turn_right(int speed){
  speed = min(speed, 255);
  speed = max(speed, 0);
  ledcWrite(left_pwmA, speed);
  ledcWrite(left_pwmB, 0);
  ledcWrite(right_pwmB, speed);
  ledcWrite(right_pwmA, 0); 
}
void stop(char move, int speed){
  speed = min(speed, 255);
  speed = max(speed, 0);
  if (move == 'F'){
    ledcWrite(left_pwmB, speed/stop_force);
    ledcWrite(left_pwmA, 0);
    ledcWrite(right_pwmB, speed/stop_force);
    ledcWrite(right_pwmA, 0);
    delay(100);
    move = 'T';
  }
  else if (move == 'B'){
    ledcWrite(left_pwmA, speed/stop_force);
    ledcWrite(left_pwmB, 0);
    ledcWrite(right_pwmA, speed/stop_force);
    ledcWrite(right_pwmB, 0);
    delay(100);
    move = 'T';
  }
  if (move == 'T'){
    ledcWrite(left_pwmA, 0);
    ledcWrite(left_pwmB, 0);
    ledcWrite(right_pwmA, 0);
    ledcWrite(right_pwmB, 0);
  }
}

void Volt_meter(uint16_t analog_value){
  float volt = (analog_value/4096.0)*19.2;
  bluetooth.println("*v" + String(volt) + "*");
}

int ultrasonic_distance(){
  long duration;
  int distance;
    // Clears the trigPin
  digitalWrite(sc_trig, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(sc_trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(sc_trig, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(sc_echo, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  return distance;
}

static int8_t led_function_index = 1;
static uint8_t K_slider = 220;
static uint8_t j_slider;
static uint8_t brightness = 255;

void bluetooth_write(String data){
  bluetooth.println(data);
}

uint8_t brightness_check(){
  return(brightness);
}

int8_t led_function_check(){
  return led_function_index;
}

uint8_t j_slider_check(){
  return j_slider;
}

uint8_t k_slider_check(){
  return K_slider;
}

void bluetooth_begin(){
  bluetooth.begin("A_Stupid_car");
}

void bluetooth_check(){
  static char bt_value;
  if (bluetooth.available()){
    bt_value = bluetooth.read();
    static int motorspeed;
    static char previous_bt_value;
    static uint8_t previous_led_function;
    static bool one_text;
    static bool fan_toggle;
    static uint8_t fan_pwm_value;

    if(bt_value == 'J'){
      j_slider = bluetooth.parseInt();
      motorspeed = map(j_slider, 0, 100, 0, 255);
    }
    if (bt_value != previous_bt_value){
      if(bt_value == 'J'){
        bt_value = previous_bt_value;
      }
    static uint8_t previous_j_slider;
    while (j_slider != previous_j_slider){
      bluetooth.println("*P" + String(j_slider) + "*");
      previous_j_slider = j_slider;
    }
      switch (bt_value){
        //drive forward
        case 'F':
          forward(motorspeed, motorspeed);
          previous_bt_value = 'F';
          break;
        //turn left
        case 'Q':
          forward(motorspeed/turn_sensitivity, motorspeed);
          previous_bt_value = 'Q';
          break;
        //turn right
        case 'E':
          forward(motorspeed, motorspeed/turn_sensitivity);
          previous_bt_value = 'E';
          break;
        //stop drive
        case 'S':
          if (previous_bt_value == 'F'){
            stop('F', motorspeed);
            previous_bt_value = 'S';
          }
          else if (previous_bt_value == 'G'){
            stop('B', motorspeed);
            previous_bt_value = 'S';
          }
          else{
            stop('T', motorspeed);
            previous_bt_value = 'S';
          }
          break;
        //drive backward 
        case 'G':
          backward(motorspeed, motorspeed);
          previous_bt_value = 'G';
          break;
        //turn left backward
        case 'Z':
          backward(motorspeed, motorspeed/turn_sensitivity);
          previous_bt_value = 'Z';
          break;
        //turn right bakward
        case 'C':
          backward(motorspeed/turn_sensitivity, motorspeed);
          backward(motorspeed, motorspeed/turn_sensitivity);
          previous_bt_value ='C';
          break;
        //spin left
        case 'L':
          turn_left(motorspeed);
          previous_bt_value = 'L';
          break;
        //spin right
        case 'R':
          turn_right(motorspeed);
          previous_bt_value = 'R';
          break;
        //read k slider value
        case 'K':
          K_slider = bluetooth.parseInt();
          break;
        //turn fan motor on
        case 'M':
          fan_toggle = true;
          ledcWrite(fan_pwm, fan_pwm_value);
          break;
        //turn fan motor off
        case 'm':
          fan_toggle = false;
          ledcWrite(fan_pwm, 0);
          break;
        //fan pwm value read
        case 'f':
          fan_pwm_value = bluetooth.parseInt();
          bluetooth.println("*n" + String(fan_pwm_value) + "*");
          fan_pwm_value = map(fan_pwm_value, 0, 100, 0, 255);
          if(fan_toggle){
            ledcWrite(fan_pwm, fan_pwm_value);
          }
          break;
        //change rgb funtion
        case 'Y':
            if(led_function_index != max_led_funtion){
              led_function_index++;
              if (led_function_index > max_led_funtion - 1){
                led_function_index = 1;
                one_text = false;
              }
            }
          break;
        //turn rgb on
        case 'B':
          led_function_index = previous_led_function;
          break;
        //turn rgb off    
        case 'b':
          previous_led_function = led_function_index;
          led_function_index = max_led_funtion;
          bluetooth.println("*T");
          bluetooth.print("OFF*");
          break;
        //rgb brightness
        case 'i':
          brightness = bluetooth.parseInt();
          bluetooth.println("*I" + String(map(brightness, 0, 255, 0, 100)) + "*");
          break;
        }
      }
    //display what slider do on phone
    if (!one_text){
      switch (led_function_index){
        case 1:{
          bluetooth.println("*TNone*");
          break;
        }
        case 2:{
          bluetooth.println("*TSpeed*");
          break;
        }
        case 3:{
          bluetooth.println("*TSpeed*");
          break;
        }
        case 4:{
          bluetooth.println("*THUE*");
          break;
        }
        case 5:{
          bluetooth.println("*TSpeed*");
          break;
        }
      }
    }
  }
}