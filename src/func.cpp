#include <Arduino.h>
#include <func.h>
#include <pin_define.h>

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