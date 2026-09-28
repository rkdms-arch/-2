#define LED 7

void setup() {
  pinMode(LED, OUTPUT);

  // LED 켜기
  digitalWrite(LED, LOW);
}

void loop() {

  // 1초 LED 켜기
  digitalWrite(LED, LOW);
  delay(1000);

  // 1초 동안 5번 깜빡이기
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED, HIGH); // 끄기
    delay(100);

    digitalWrite(LED, LOW);  // 켜기
    delay(100);
  }

  // LED 끄기
  digitalWrite(LED, HIGH);

  // 잠시 꺼진 상태
  delay(1000);
}
