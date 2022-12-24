#include <Arduino.h>
#include <motor.h>
#include <pin_define.h>
#include <BluetoothSerial.h>
#include <PS4Controller.h>

BluetoothSerial bluetooth;

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

void led_function_write(int index){
  led_function_index = index;
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

#ifdef phone_bluetooth
  void bluetooth_begin(){
    bluetooth.begin("A_Stupid_car");
  }

  void bluetooth_check(){
    if (bluetooth.available()){
      char bt_value;
      static int motorspeed;
      static char previous_bt_value;
      static uint8_t previous_led_function;
      static bool one_text;
      static bool fan_toggle;
      static uint8_t fan_pwm_value;
      static bool reverse = true;
      static int raw_turn_sensitivity;
      static int turn_sensitivity;

      bt_value = bluetooth.read();
      //Serial.write(bt_value);

      //read mortor speed value
      if (bt_value == 'J'){
        j_slider = bluetooth.parseInt();
        motorspeed = map(j_slider, 0, 100, 0, 255);
        bluetooth.println("*P" + String(j_slider) + "*");
      }
      
      if (bt_value != previous_bt_value){
        if(bt_value == 'J'){
          bt_value = previous_bt_value;
        }
      
        //*******************************check key value*****************************************************
        switch (bt_value){
          //drive forward
          case 'F':
            forward(motorspeed, motorspeed);
            previous_bt_value = 'F';
            break;
          //turn left
          case 'Q':
            previous_bt_value = 'Q';
            break;
          //turn right
          case 'E':
            previous_bt_value = 'E';
            break;
          //stop drive
          case 'S':
            if (previous_bt_value == 'F'){
              stop('F', motorspeed);
            }
            else if (previous_bt_value == 'G'){
              stop('B', motorspeed);
            }
            else{
              stop('T', motorspeed);
            }
            previous_bt_value = 'S';
            break;
          //drive backward 
          case 'G':
            backward(motorspeed, motorspeed);
            previous_bt_value = 'G';
            break;
          case 'h':
            raw_turn_sensitivity = bluetooth.parseInt();
            bluetooth.println("*H" + String(raw_turn_sensitivity) + "*");
            break;
          //reverse mode
          case 'C': //on
            reverse = true;
            break;
          case 'c': //off
            reverse = false;
            break;
          //spin left
          case 'L':
            spin_left(motorspeed);
            previous_bt_value = 'L';
            break;
          //spin right
          case 'R':
            spin_right(motorspeed);
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
          //***********************************************rgb control*****************************************************************  
          //change rgb funtion
          case 'y':
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
      //*************************turn left or right (forward way/backward way)*********************************
      //turn left
      turn_sensitivity = map(raw_turn_sensitivity, 0, 100, 0, motorspeed*2);
      if (previous_bt_value == 'Q'){
        if (reverse){
          forward(motorspeed-turn_sensitivity, motorspeed);
        }
        else{
          backward(motorspeed-turn_sensitivity, motorspeed);
        }
      }
      //turn right
      else if (previous_bt_value == 'E'){
        if (reverse){
          forward(motorspeed, motorspeed-turn_sensitivity);
          }
        else{
          backward(motorspeed, motorspeed-turn_sensitivity);
        }
      }

      //*************************display rgb texts to phone**********************************
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
#endif

#ifdef ps4_bluetooth
  void ps4_begin(){
    PS4.begin("00:1a:7d:da:71:10");
  }
  void check_PS4(){
  }
#endif