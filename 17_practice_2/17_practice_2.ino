#include <Servo.h>

// ---------- Arduino pin assignment ----------
#define PIN_LED   9    // 10~25cm 범위일 때 켜기
#define PIN_SERVO 10   // 서보 핀 (17_example_3.ino 와 동일하게 맞출 것)
#define PIN_IR    A0   // 적외선 센서

// ---------- 거리 범위 (mm) ----------
#define _DIST_MIN 100.0   // 10cm
#define _DIST_MAX 250.0   // 25cm

// ---------- 서보 duty (us) : 본인 서보에 맞게 캘리브레이션 ----------
#define _DUTY_MIN 553     // 0도
#define _DUTY_NEU 1476    // 90도
#define _DUTY_MAX 2399    // 180도

// ---------- 필터/주기 ----------
#define INTERVAL 20       // loop 주기 (ms), 20 이상
#define _EMA_ALPHA 0.3    // 0~1, 작을수록 부드럽지만 반응 느림

Servo myservo;

int a_value;
float dist_raw, dist_filtered, dist_ema = 0.0, dist_prev = _DIST_MIN;
float duty;
unsigned long last_sampling_time = 0;
bool first_run = true;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(_DUTY_NEU);
  Serial.begin(1000000);
}

void loop() {
  // 주기 맞추기
  if (millis() < last_sampling_time + INTERVAL) return;
  last_sampling_time += INTERVAL;

  // 1) 센서 값 읽기 + 거리 변환 (mm)
  a_value = analogRead(PIN_IR);
  dist_raw = (6762.0 / (a_value - 9) - 4.0) * 10.0 - 60.0;

  // 2) 범위 필터
  bool in_range = (dist_raw >= _DIST_MIN && dist_raw <= _DIST_MAX);
  if (dist_raw < _DIST_MIN || dist_raw > 1000.0 || dist_raw != dist_raw) {
    // 너무 가깝거나 이상값 -> 이전 값 유지
    dist_filtered = dist_prev;
    in_range = false;
  } else if (dist_raw > _DIST_MAX) {
    // 25cm보다 멀면 경계값으로 제한
    dist_filtered = _DIST_MAX;
  } else {
    dist_filtered = dist_raw;
  }
  dist_prev = dist_filtered;

  // 3) EMA 필터
  if (first_run) {
    dist_ema = dist_filtered;
    first_run = false;
  } else {
    dist_ema = _EMA_ALPHA * dist_filtered + (1.0 - _EMA_ALPHA) * dist_ema;
  }

  // 4) LED: 10~25cm 범위일 때 켜기
  digitalWrite(PIN_LED, in_range ? HIGH : LOW);

  // 5) 서보 제어 (map() 사용 금지 -> 직접 계산)
  if (dist_ema <= _DIST_MIN) {
    duty = _DUTY_MIN;
  } else if (dist_ema >= _DIST_MAX) {
    duty = _DUTY_MAX;
  } else {
    duty = _DUTY_MIN +
           (dist_ema - _DIST_MIN) / (_DIST_MAX - _DIST_MIN) * (_DUTY_MAX - _DUTY_MIN);
  }
  myservo.writeMicroseconds((int)duty);

  // 6) 시리얼 플로터 출력
  Serial.print("_DUTY_MIN:"); Serial.print(_DUTY_MIN);
  Serial.print(",_DIST_MIN:"); Serial.print(_DIST_MIN);
  Serial.print(",IR:");        Serial.print(a_value);
  Serial.print(",dist_raw:");  Serial.print(dist_raw);
  Serial.print(",ema:");       Serial.print(dist_ema);
  Serial.print(",servo:");     Serial.print(duty);
  Serial.print(",_DIST_MAX:"); Serial.print(_DIST_MAX);
  Serial.print(",_DUTY_MAX:"); Serial.print(_DUTY_MAX);
  Serial.println("");
}
