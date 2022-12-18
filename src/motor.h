void forward(int left_speed, int right_speed);
void backward(int left_speed, int right_speed);
void spin_left(int speed);
void spin_right(int speed);
void stop(char move, int speed);

void Volt_meter(uint16_t analog_value);

int ultrasonic_distance();

void beep_tick();
void beep_none_delay(int duration);
void beep(int duration);

void fanWrite(uint8_t pwm);

