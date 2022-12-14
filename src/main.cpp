#include <Arduino.h>
#include <pin_define.h>
#include <func.h>
#include <FastLED.h>

bool line_follow_activate;

uint8_t sc_distance;

CRGBArray<NUM_LEDS> leds;

TaskHandle_t Task0;
TaskHandle_t Task1;

//core 1
void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  //loop
  for(;;){
    bluetooth_check();
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
      sc_distance = ultrasonic_distance();
    }

    static uint8_t hue;
    static int led_index;
    static unsigned long led_delay;
    //change rgb function
    uint8_t K_slider = k_slider_check();
    int8_t led_function_index = led_function_check();
    uint8_t brightness_slider = brightness_check();
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
        FastLED.setBrightness(brightness_slider);
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
          FastLED.setBrightness(brightness_slider);
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
        FastLED.setBrightness(brightness_slider);
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
    static int manual_led_index;
    if (manual_led_index != led_function_check()){
      manual_led_index = led_function_check();
    }
    if ((millis() - switch_hold_delay) > 1500 && !click_hold && one_click){
      manual_led_index ++;
      led_function_write(manual_led_index);
      if (manual_led_index > max_led_funtion){
        manual_led_index = 1;
        led_function_write(manual_led_index);
      }
      click_hold = true;
    }
    static unsigned long volt_meter_delay;
    if((millis() - volt_meter_delay) > 1000){
      Volt_meter(analogRead(volt_meter_pin));
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
  bluetooth_begin();
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

  //ultrasonic pin setup
  pinMode(sc_trig, OUTPUT);
  pinMode(sc_echo, INPUT);

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