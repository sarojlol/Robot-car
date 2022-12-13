#include <Arduino.h>
#include <pin_define.h>
#include <func.h>
#include <BluetoothSerial.h>
#include <FastLED.h>
#include <NewPing.h>

bool line_follow_activate;

uint8_t j_slider;

NewPing ultrasonic(sc_trig, sc_echo, MAX_DISTANCE);
uint8_t sc_distance;

CRGBArray<NUM_LEDS> leds;
int8_t led_function_index = 1;
uint8_t K_slider = 220;


BluetoothSerial bluetooth;

TaskHandle_t Task0;
TaskHandle_t Task1;

void bluetooth_check(char bt_value){
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
      case 'Z':
        backward(motorspeed, motorspeed/turn_sensitivity);
        previous_bt_value = 'Z';
        break;
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
        if(fan_toggle){
          fan_pwm_value = map(fan_pwm_value, 0, 100, 0, 255);
          ledcWrite(fan_pwm, fan_pwm_value);
        }
        break;
      //change rgb funtion
      case 'Y':
          if(led_function_index != max_led_funtion){
            led_function_index++;
            FastLED.setBrightness(255);
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

//core 1
void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  //loop
  for(;;){
    if (bluetooth.available()){
      bluetooth_check(bluetooth.read());
    }
    static bool E_Stop;
    if (sc_distance <= 10 && line_follow_activate){
      E_Stop = true;
    }
    if(E_Stop){
      stop('F', 255);
      line_follow_activate = false;
      E_Stop = false;
    }

    //follow line
    static bool afterTurn;
    if (line_follow_activate){
      if (!digitalRead(center_line)){
        forward(forward_line_speed, forward_line_speed);
        afterTurn = false;
      }
      if(!digitalRead(left_center_line)){
        forward(forward_line_speed/2, forward_line_speed);
      }
      if(!digitalRead(right_center_line)){
        forward(forward_line_speed, forward_line_speed/2);
      }
      if(!digitalRead(right_line) &! digitalRead(right_center_line) &! afterTurn){
        delay(250);
        stop('F', turn_line_speed);
        turn_right(turn_line_speed);
        delay(turn_line_speed*1.4);
        stop('T', turn_line_speed);
        afterTurn = true;
      }
      // if(digitalRead(right_line) == LOW && afterTurn){
      //   forward(forward_line_speed, forward_line_speed/2);
      //   afterTurn = false;
      // }
      // if(digitalRead(left_line) == LOW && afterTurn){
      //   forward(forward_line_speed/2, forward_line_speed);
      //   afterTurn = false;
      // }
      // if(digitalRead(left_line) == LOW && digitalRead(left_center_line) == LOW && digitalRead(center_line) == LOW && 
      // digitalRead(right_center_line) == LOW && digitalRead(right_line) == LOW){
      //   stop('T', 255);
      //   line_follow_activate = false;
      // }
    }
  } 
}

//core 0 
void Task0code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  //loop
  for(;;){
    //read ultrasonic sensor
    if (line_follow_activate){
      sc_distance = ultrasonic.ping_cm();
    }

    //update mortor speed percentage
    static uint8_t previous_j_slider;
    if (j_slider != previous_j_slider){
      bluetooth.println("*P" + String(j_slider) + "*");
      previous_j_slider = j_slider;
    }

    static uint8_t hue;
    static int led_index;
    static unsigned long led_delay;
    //change rgb function
    switch (led_function_index){
    case 1:{
      int16_t t = millis()/4;
      uint16_t x = 0;
      int scale;
      scale = beatsin8(10, 10, 30);
      if (led_index < NUM_LEDS){
        hue = inoise8(led_index * scale + x, t);
        leds[led_index] = CHSV(hue, 255, 255);
        led_index ++;
      }
      else{
        FastLED.show();
        led_index = 0;
      }
      break;
    }
    case 2:{
      if((millis() - led_delay) > 255 - K_slider){
        if (led_index < NUM_LEDS/2){
          led_index ++;
          // fade everything out
          leds.fadeToBlackBy(50);

          // let's set an led value
          leds[led_index] = CHSV(hue++, 255, 255);

          // now, let's first 20 leds to the top 20 leds,
          leds(NUM_LEDS/2, NUM_LEDS-1) = leds(NUM_LEDS/2 -1  ,0);
          FastLED.show();
        }
        if (led_index >= NUM_LEDS/2){
          led_index = -1;
        }
        led_delay = millis();
      }
      break;
    }
    case 3:{
      if(led_index < NUM_LEDS){
        leds[led_index++] = CHSV(hue, 255, 255);
        FastLED.show();
      }
      else{
        led_index = 0;
      }
      if((millis() - led_delay) > 255-K_slider){
        hue += 2;
        led_delay = millis();
      }
      break;
    }
    case 4:{
      if(led_index < NUM_LEDS){
        leds[led_index++] = CHSV(K_slider, 255, 255);
        FastLED.show();
      }
      else if(led_index > NUM_LEDS-1) {
        led_index = 0;
      }
      break;
    }
    case 5:{
      static bool fade_flag;
      static int brightness;
      if((millis() - led_delay) > 255 - K_slider){
        if (brightness >= 0 && !fade_flag){
          brightness += 10;
          if (brightness >= 254){
            brightness = 255;
            hue += 8;
            fade_flag = true;
          }
        }
        else if (brightness <= 256 && fade_flag){
          brightness -= 10;
          if(brightness <= 1){
            brightness = 0;
            hue += 8;
            fade_flag = false;
          }
        }
        led_delay = millis();
        FastLED.setBrightness(brightness);
      }

      if (led_index < NUM_LEDS){
        led_index ++;
      }
      else{
        led_index = 0;
      }
      brightness = min(brightness, 255);
      brightness = max(brightness, 0);
      leds[led_index] = CHSV(hue, 255, 255);
      FastLED.show();
      //Serial.println(brightness);
      break;
    }
    case max_led_funtion:{
      FastLED.clear();
      FastLED.show();
      break;
    }
    }

    //read start button
    static unsigned long button_filter;
    static unsigned long switch_hold_delay;
    static bool click_hold = false;
    static bool one_click;
    if((millis() - button_filter) > 20){
      if (!digitalRead(button_pin) &! one_click &! click_hold){
        switch_hold_delay = millis();
        one_click = true;
      }
      else if (digitalRead(button_pin) && one_click){
        if(!click_hold){
          line_follow_activate =! line_follow_activate;
          if(!line_follow_activate){
            stop('F', forward_line_speed);
          }
          one_click = false;
        }
        else{
          click_hold = false;
          one_click = false;
        }
      }
      button_filter = millis();
    }
    //switch hold function
    if ((millis() - switch_hold_delay) > 1500 && !click_hold && one_click){
      led_function_index ++;
      if (led_function_index > max_led_funtion){
        led_function_index = 1;
      }
      click_hold = true;
    }
    static unsigned long volt_meter_delay;
    if((millis() - volt_meter_delay) > 1000){
      float volt = Volt_meter(analogRead(volt_meter_pin));
      bluetooth.println("*v" + String(volt) + "*");
      volt_meter_delay = millis();
    }
  } 
}

void setup() {

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task0code,   /* Task function. */
                    "Task0",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task0,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
  delay(500); 

  //ws2812 setup
  Serial.begin(115200);
  bluetooth.begin("A_Stupid_car");
  FastLED.addLeds<WS2812B,fastLed_pin>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  //left motor pwm setup
  ledcSetup(left_pwmA, 500, 8);
  ledcAttachPin(left_A, left_pwmA);
  ledcSetup(left_pwmB, 500, 8);
  ledcAttachPin(left_B, left_pwmB);

  //right motor pwm setup
  ledcSetup(right_pwmA, 500, 8);
  ledcAttachPin(right_A, right_pwmA);
  ledcSetup(right_pwmB, 500, 8);
  ledcAttachPin(right_B, right_pwmB);

  //Line track pin setup
  pinMode(left_line, INPUT);
  pinMode(left_center_line, INPUT);
  pinMode(center_line, INPUT);
  pinMode(right_center_line, INPUT);
  pinMode(right_line, INPUT);

  //Acessory pin setup
  pinMode(led_builtin, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
  pinMode(volt_meter_pin, INPUT);
  ledcSetup(fan_pwm, 50, 8);
  ledcAttachPin(fan_pin, fan_pwm);

  digitalWrite(led_builtin, HIGH);
  delay(500);
  digitalWrite(led_builtin, LOW);
}
void loop() {
}