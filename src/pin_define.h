//general config
#define sw_beep
#define phone_bluetooth
//#define ps4_bluetooth

//tcrt5000L pins
#define sensor1 33 // far left
#define sensor2 32 // left
#define sensor3 35 // center
#define sensor4 34 // right
#define sensor5 39 // far right
#define sensorL 12 // edge left
#define sensorR 13 // edge right
//line track config
#define line_forward_speed 128
#define line_stop_speed 255
#define turn_line_speed1 110 // lean abit slow side
#define turn_line_speed2 128 // lean abit fast side
#define turn_line_speed3 0 // lean more slow side
#define turn_line_speed4 200 // lean more fast side
#define spin_speed1 200//spin slowly
#define spin_speed2 200//spin faster
#define spin_speed3 200//spin fastest
//#define line_debug
//#define line_beep
#define line_beep_duration 50

//motor drive
#define left_A 19
#define left_B 17
#define right_A 4
#define right_B 16
#define left_pwmA 0
#define left_pwmB 1
#define right_pwmA 2
#define right_pwmB 3
#define stop_force 2
#define turn_sensitivity 5
#define joystick_deadzoneY 10
#define joystick_deadzomeX 10

//accessory pins
#define led_builtin 2
#define button_pin 5
#define buzzer_pin 23
#define fan_pin 27
#define fan_pwm 4
#define volt_meter_pin 36
#define voltage_mornitor

//hc-sr04 pins
#define sc_trig 25
#define sc_echo 26
#define MAX_DISTANCE 200

//WS2812b config
#define fastLed_pin 14
#define NUM_LEDS 30
#define max_led_funtion 6

