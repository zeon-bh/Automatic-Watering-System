/* Automatic Watering System v1.2 - Implemented millis() to track time */

// Moisture level sensor
int ms_read = A0;
int ms_power = 2;
int ms_value = 0;

// Water level Sensor
int wl_read = A1;
int wl_power = 3;
int wl_value = 0;

// Water pump switch
int water_pump = 12;
boolean pump_cycle_1 = false;
boolean pump_cycle_2 = false;

// Manual Override button
int manual_override = 13;
boolean readvalue = false;

//RGB LED Pins
int rgb_red = 5;
int rgb_blue = 7;
int red_state = LOW;
int blue_state = LOW;
boolean led_off = false;
/**************************** TIMERS *********************************/
//System Clock
unsigned long system_clock; //  Main System Clock millis()
int system_cycle = 0; // No. of sensor reading cycles every 1 hour
boolean record_time = false; // Record millis() timestamp;

// Sensors
unsigned long sensor_time = 0; // Current time of the sensor clock
unsigned long sensor_interval = 3600000; // Get readings every 1 hour
//unsigned long sensor_interval = 15000; // For Testing Purpose only!!!

//LED
unsigned long rgb_red_time = 0; // RGB Red current time
unsigned long rgb_blue_time = 0; // RGB Blue current time
unsigned long rgb_dual_time = 0; // RGB dual current time
unsigned int led_interval = 1000; // RGB led blink rate

//Water Pump
unsigned long pump_time_1 = 0; // Water pump current time 1
unsigned long pump_time_2 = 0; // Water pump current time 2
unsigned int pump_duration_1 = 5000; // Water pump run duration for cycle 1 ---------------------- Change this for Standard pump duration (ms).
unsigned int pump_duration_2 = 1500; // Water pump run duration for cycle 2 ---------------------- Change this for backup pump duration (ms).

// Serial port timings
unsigned long serial_time = 0;
unsigned long serial_interval = 1000;

void setup() {
  // Initialize pins
  pinMode(ms_power, OUTPUT);
  pinMode(ms_read, INPUT);
  pinMode(wl_power, OUTPUT);
  pinMode(wl_read, INPUT);
  pinMode(water_pump, OUTPUT);
  pinMode(rgb_red, OUTPUT);
  pinMode(rgb_blue, OUTPUT);
  pinMode(manual_override, INPUT);
  
  // Set Pin values
  digitalWrite(ms_power, LOW);
  digitalWrite(wl_power, LOW);
  digitalWrite(water_pump, LOW);
  digitalWrite(rgb_red, LOW);
  digitalWrite(rgb_blue, LOW);

  // Initial Check up
  SensorReading();
  SystemCheck();
//  Serial.begin(9600); // For Testing Purpose only!!!
}

void loop() {
  SystemOverride(); // Override function has the highest priority
  system_clock = millis(); // Initialize system clock
  RunLed();
  AutomaticWaterSystem();
//  ReadValues(); // For Testing Purpose only!!!
}
/*************************************** Main Water System ******************************************/

void AutomaticWaterSystem() { // Main Loop Function to keep track of time and device intervals
  if(system_clock - sensor_time >= sensor_interval) { // After one hour check sensor values and water plant if needed
    sensor_time = system_clock;
    Serial.println("Reading values......");
    SensorReading();
    if(ms_value < 50) {
      system_cycle++;
    }
    SystemCheck();
  }
  if(pump_cycle_1) { // if true, initialize pump clock and run pump for 5 seconds
    if(!record_time) {
      pump_time_1 = system_clock;
      record_time = true;
    }
    digitalWrite(water_pump, HIGH);
    if(system_clock - pump_time_1 >= pump_duration_1) {
      digitalWrite(water_pump, LOW);
      pump_cycle_1 = false;
      record_time = false;
    }
  }
  if(pump_cycle_2) { // if true, initialize pump clock and run pump for 1.5 seconds
    if(!record_time) {
      pump_time_2 = system_clock;
      record_time = true;
    }
    digitalWrite(water_pump, HIGH);
    if(system_clock - pump_time_2 >= pump_duration_2) {
      digitalWrite(water_pump, LOW);
      pump_cycle_2 = false;
      record_time = false;
    }
  }
}

void SensorReading() { // Gets ms_value and wl_value
  // Moisture level Reading
  digitalWrite(ms_power, HIGH);
  ms_value = analogRead(ms_read);
  digitalWrite(ms_power, LOW);

  //Water level Reading
  digitalWrite(wl_power, HIGH);
  wl_value = analogRead(wl_read);
  digitalWrite(wl_power, LOW);
}

void SystemCheck() { // SystemCheck decides wheter to start a pump cycle
  if(((ms_value < 400) && (ms_value > 50)) && (wl_value > 200)) {
    pump_cycle_1 = true;
    system_cycle = 0;
  } else if (((ms_value < 10) && (wl_value > 200)) && (system_cycle >= 24)) {
    pump_cycle_2 = true;
    system_cycle = 0;
  }
}

/*************************************** Diagnostic LEDS ******************************************/

void RunLed() { // Main diagnostic led control method
  if((ms_value < 50)  && (wl_value > 200)) { // If moisture sensor has an error, blink red
     if(system_clock - rgb_red_time >= led_interval) {
      rgb_red_time = system_clock;
      digitalWrite(rgb_blue, LOW);
      led_off = false;
      BlinkRed();
     }
  } else if ((ms_value > 50)  && (wl_value < 200)){ // If water level is low, blink blue
    if(system_clock - rgb_blue_time >= led_interval) {
      rgb_blue_time = system_clock;
      digitalWrite(rgb_red, LOW);
      led_off = false;
      BlinkBlue();
     }
  } else if((ms_value < 50)  && (wl_value < 200)) { // Blink both led's when both sensor's are malfunctioning
    if(system_clock - rgb_dual_time >= led_interval) {
      rgb_dual_time = system_clock;
      if(!led_off) {
        digitalWrite(rgb_red, LOW);
        digitalWrite(rgb_blue, LOW);
        led_off = true;
      }
      BlinkRed();
      BlinkBlue();
     }
  } else {
    digitalWrite(rgb_blue, LOW);
    digitalWrite(rgb_red, LOW);
  }
}

void BlinkBlue() { // Blue LED Control
  if (blue_state == LOW) {
              blue_state = HIGH;
          } else {
              blue_state = LOW;
          }
      digitalWrite(rgb_blue, blue_state);
}

void BlinkRed() { // Red LED Control
  if (red_state == LOW) {
              red_state = HIGH;
          } else {
              red_state = LOW;
          }
      digitalWrite(rgb_red, red_state);
}

/*************************************** System Override ******************************************/

void SystemOverride() { // Manual override of pump function
  if(digitalRead(manual_override) == LOW) {
    digitalWrite(water_pump, HIGH);
    if(!readvalue) {
      SensorReading();
      readvalue = true;
    }
  } else {
    digitalWrite(water_pump, LOW);
    readvalue = false;
  }
}

void ReadValues() { // For Testing Purpose only!!!
  if(system_clock - serial_time >= serial_interval) {
    serial_time = system_clock;
    Serial.print("ms value = ");
    Serial.println(ms_value);
  
    Serial.print("wl value = ");
    Serial.println(wl_value);

    Serial.print("System cycle = ");
    Serial.println(system_cycle);
  }
}


