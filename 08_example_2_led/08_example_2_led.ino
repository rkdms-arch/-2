#include <Servo.h>

// Arduino pin assignment
#define PIN_LED   9   // LED active-low (PWM pin)
#define PIN_TRIG  12  // sonar sensor TRIGGER
#define PIN_ECHO  13  // sonar sensor ECHO
#define PIN_SERVO 10  // servo motor

// configurable parameters for sonar
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25       // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100.0   // minimum distance to be measured (unit: mm)
#define _DIST_MAX 400.0   // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // coefficent to convert duration to distance

#define _EMA_ALPHA 0.3    // EMA weight of new sample (range: 0 to 1)
                          // Setting EMA to 1 effectively disables EMA filter.

// LED brightness profile (unit: mm)
#define _LED_ZERO_LOW  100.0  // LED off at this distance
#define _LED_PEAK      200.0  // LED brightest at this distance
#define _LED_ZERO_HIGH 300.0  // LED off at this distance

// Target Distance
#define _TARGET_LOW  250.0
#define _TARGET_HIGH 290.0

// duty duration for myservo.writeMicroseconds()
// NEEDS TUNING (servo by servo)
#define _DUTY_MIN 1000 // servo full clockwise position (0 degree)
#define _DUTY_NEU 1500 // servo neutral position (90 degree)
#define _DUTY_MAX 2000 // servo full counterclockwise position (180 degree)

// global variables
float  dist_ema, dist_prev = _DIST_MAX; // unit: mm
unsigned long last_sampling_time;       // unit: ms

Servo myservo;

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);    // sonar TRIGGER
  pinMode(PIN_ECHO, INPUT);     // sonar ECHO
  digitalWrite(PIN_TRIG, LOW);  // turn-off Sonar

  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(_DUTY_NEU);

  // initialize USS related variables
  dist_prev = _DIST_MIN; // raw distance output from USS (unit: mm)
  dist_ema = _DIST_MIN;  // initial value of EMA

  // LED off at start (active-low: 255 = off)
  analogWrite(PIN_LED, 255);

  // initialize serial port
  Serial.begin(57600);

  last_sampling_time = millis();
}

void loop() {
  float dist_raw, dist_filtered;
  float brightness;   // 0.0 (off) ~ 1.0 (brightest)
  int   led_value;    // analogWrite value (active-low: 0 = brightest, 255 = off)

  // wait until next sampling time.
  // millis() returns the number of milliseconds since the program started.
  // will overflow after 50 days.
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // get a distance reading from the USS
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // the range filter
  if ((dist_raw == 0.0) || (dist_raw > _DIST_MAX)) {
    dist_filtered = dist_prev;
  } else if (dist_raw < _DIST_MIN) {
    dist_filtered = dist_prev;
  } else {    // In desired Range
    dist_filtered = dist_raw;
    dist_prev = dist_raw;
  }

  // EMA filter
  dist_ema = _EMA_ALPHA * dist_filtered + (1.0 - _EMA_ALPHA) * dist_ema;

  // LED brightness control (triangle profile)
  //   100mm -> off, 150mm -> 50%, 200mm -> max, 250mm -> 50%, 300mm -> off
  if (dist_ema <= _LED_ZERO_LOW || dist_ema >= _LED_ZERO_HIGH) {
    brightness = 0.0;
  } else if (dist_ema <= _LED_PEAK) {
    brightness = (dist_ema - _LED_ZERO_LOW) / (_LED_PEAK - _LED_ZERO_LOW);
  } else {
    brightness = (_LED_ZERO_HIGH - dist_ema) / (_LED_ZERO_HIGH - _LED_PEAK);
  }

  // active-low: brightest -> 0, off -> 255
  led_value = (int)(255.0 * (1.0 - brightness) + 0.5);
  analogWrite(PIN_LED, led_value);

  // adjust servo position according to the USS read value
  // add your code here!


  // output the distance to the serial port
  Serial.print("Min:");     Serial.print(_DIST_MIN);
  Serial.print(",Low:");    Serial.print(_TARGET_LOW);
  Serial.print(",dist:");   Serial.print(dist_ema);
  Serial.print(",LED:");    Serial.print(led_value);
  Serial.print(",Servo:");  Serial.print(myservo.read());
  Serial.print(",High:");   Serial.print(_TARGET_HIGH);
  Serial.print(",Max:");    Serial.print(_DIST_MAX);
  Serial.println("");

  // update last sampling time
  last_sampling_time += INTERVAL;
}

// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm
}
