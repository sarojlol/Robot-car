void forward(int left_speed, int right_speed);
void backward(int left_speed, int right_speed);
void turn_left(int speed);
void turn_right(int speed);
void stop(char move, int speed);

void Volt_meter(uint16_t analog_value);

int ultrasonic_distance();

uint8_t brightness_check();
void bluetooth_begin();
void bluetooth_write(String data);
void led_function_write(int index);
int8_t led_function_check();
uint8_t j_slider_check();
uint8_t k_slider_check();
void bluetooth_check();