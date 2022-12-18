#include <Arduino.h>
#include <pin_define.h>
#include <motor.h>
#include <bluetooth.h>
#include <FastLED.h>

bool line_follow_activate;
bool sensor1_value;
bool sensor2_value;
bool sensor3_value;
bool sensor4_value;
bool sensor5_value;
bool sensorL_value;
bool sensorR_value;

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
    if (line_follow_activate){
      sc_distance = ultrasonic_distance();
      if (sc_distance <= 10){
        stop('F', line_stop_speed);
        beep(100);
        fanWrite(255);
        delay(2000);
        fanWrite(0);
        line_follow_activate = false;
      }
      else{
        sensor1_value = digitalRead(sensor1);
        sensor2_value = digitalRead(sensor2);
        sensor3_value = digitalRead(sensor3);
        sensor4_value = digitalRead(sensor4);
        sensor5_value = digitalRead(sensor5);
        sensorL_value = digitalRead(sensorL);
        sensorR_value = digitalRead(sensorR);
        //***************************************no line detect**********************************************
        //stop
        if (!sensor1_value &! sensor2_value &! sensor3_value &! sensor4_value &! sensor5_value){ //00000
          stop('T', 255);
          beep(1000);
          line_follow_activate = false;
        }
        //****************************************keep forward**********************************************
        else if (
          (sensor1_value && sensor2_value &! sensor3_value && sensor4_value && sensor5_value) || /* 11011 */
          (sensor1_value &! sensor2_value &! sensor3_value &! sensor4_value && sensor5_value) /* 10001 */
        )
        {
          forward(line_forward_speed, line_forward_speed);
        }
        //****************************************left error***************************************************
        //turn left
        else if (!sensor1_value &! sensor2_value && sensor3_value && sensor4_value && sensor5_value){//00111
          spin_left(spin_speed);
        }
        //turn left
        else if (!sensor1_value && sensor2_value && sensor3_value && sensor4_value && sensor5_value){//01111
          spin_left(spin_speed2);
        }
        //lean left abit
        else if (sensor1_value &! sensor2_value &! sensor3_value && sensor4_value && sensor5_value){//10011
          forward(turn_line_speed1, turn_line_speed2);
        }
        //lean left more
        else if (sensor1_value &! sensor2_value && sensor3_value && sensor4_value && sensor5_value){//10111
          forward(turn_line_speed3, turn_line_speed4);
        }
        //***************************************right error**************************************************
        //turn right
        else if (sensor1_value && sensor2_value && sensor3_value &! sensor4_value &! sensor5_value){//11100
          spin_right(spin_speed);
        }
        //turn right
        else if (sensor1_value && sensor2_value && sensor3_value && sensor4_value &! sensor5_value){//11110
          spin_right(spin_speed2);
        }
        //lean right abit
        else if (sensor1_value && sensor2_value &! sensor3_value &! sensor4_value && sensor5_value){//11001
          forward(turn_line_speed2, turn_line_speed1);
        }
        //lean right more
        else if (sensor1_value && sensor2_value && sensor3_value &! sensor4_value && sensor5_value){//11101
          forward(turn_line_speed4, turn_line_speed3);
        }
        //***************************************out of line*************************************************
        else if(sensor1_value && sensor2_value && sensor3_value && sensor4_value && sensor5_value){//11111
          forward(line_forward_speed, line_forward_speed);
          delay(50);
          stop('T', 255);
          delay(500);
          beep(1000);
          line_follow_activate = false;
        #ifdef line_debug
          static unsigned long line_debug_delay;
          if ((millis() - line_debug_delay) > 50){
            Serial.print(sensor1_value);
            Serial.print(sensor2_value);
            Serial.print(sensor3_value);
            Serial.print(sensor4_value);
            Serial.print(sensor5_value);
            Serial.print(sensorL_value);
            Serial.println(sensorR_value);
            line_debug_delay = millis();
          }
        #endif
        }
      }
    }

  } 
}

//core 0 
void Task0code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  //loop
  for(;;){
    beep_tick();
    //WS2812B variable
    static uint8_t hue;
    static int led_index;
    static unsigned long led_delay;
    uint8_t K_slider = k_slider_check();
    int8_t led_function_index = led_function_check();
    uint8_t brightness_slider = brightness_check();
    switch (led_function_index){
    //ranndom noise color
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
    //running to center rainbow
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
    //solid rainbow color
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
    //chosen solid color
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
    //breathing rainbow color
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
    //turn all off
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
          beep_none_delay(200);
          if(!line_follow_activate){
            stop('F', 255);
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

    //voltage mornitor
    #ifdef voltage_mornitor
      static unsigned long volt_meter_delay;
      if((millis() - volt_meter_delay) > 1000){
        Volt_meter(analogRead(volt_meter_pin));
        volt_meter_delay = millis();
      }
    #endif
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
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(sensor3, INPUT);
  pinMode(sensor4, INPUT);
  pinMode(sensor5, INPUT);
  pinMode(sensorL, INPUT);
  pinMode(sensorR, INPUT);

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