#include <Arduino.h>
#include <motor.h>
#include <bluetooth.h>
#include <pin_define.h>

void forward(int left_speed, int right_speed){
  left_speed = constrain(left_speed, -255, 255);
  right_speed = constrain(right_speed, -255, 255);
  if (left_speed > 0){
    ledcWrite(left_pwmA, left_speed);
    ledcWrite(left_pwmB, 0);
  }
  else if (left_speed < 0){
    ledcWrite(left_pwmA, 0);
    ledcWrite(left_pwmB, left_speed*-1);
  }

  if (right_speed > 0){
    ledcWrite(right_pwmA, right_speed);
    ledcWrite(right_pwmB, 0);
  }
  else if (right_speed < 0){
    ledcWrite(right_pwmA, 0);
    ledcWrite(right_pwmB, right_speed*-1);
  }
}

void backward(int left_speed, int right_speed){
  left_speed = constrain(left_speed, -255, 255);
  right_speed = constrain(right_speed, -255, 255);
  if (left_speed > 0){
    ledcWrite(left_pwmA, 0);
    ledcWrite(left_pwmB, left_speed);
  }
  else if (left_speed < 0){
    ledcWrite(left_pwmA, left_speed*-1);
    ledcWrite(left_pwmB, 0);
  }

  if (right_speed > 0){
    ledcWrite(right_pwmA, 0);
    ledcWrite(right_pwmB, right_speed);
  }
  else if (right_speed < 0){
    ledcWrite(right_pwmA, right_speed*-1);
    ledcWrite(right_pwmB, 0);
  }
}

void spin_left(int speed){
  speed = constrain(speed, 0, 255);
  ledcWrite(left_pwmB, speed);
  ledcWrite(left_pwmA, 0);
  ledcWrite(right_pwmA, speed);
  ledcWrite(right_pwmB, 0);
}
void spin_right(int speed){
  speed = constrain(speed, 0, 255);
  ledcWrite(left_pwmA, speed);
  ledcWrite(left_pwmB, 0);
  ledcWrite(right_pwmB, speed);
  ledcWrite(right_pwmA, 0); 
}
void stop(char move, int speed){
  speed = constrain(speed, 0, 255);
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
  bluetooth_write("*v" + String(volt) + "*");
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
static unsigned long beep_delay;
static bool beep_flag;
static int duration1;
void beep_tick(){
  if (((millis() - beep_delay) > duration1) && beep_flag){
    digitalWrite(buzzer_pin, LOW);
    beep_flag = false;
  }
}

void beep_none_delay(int duration){
  digitalWrite(buzzer_pin, HIGH);
  beep_delay = millis();
  duration1 = duration;
  beep_flag = true;
}

void beep(int duration){
  digitalWrite(buzzer_pin, HIGH);
  delay(duration);
  digitalWrite(buzzer_pin, LOW);
}

void fanWrite(uint8_t pwm){
  ledcWrite(fan_pwm, pwm);
}