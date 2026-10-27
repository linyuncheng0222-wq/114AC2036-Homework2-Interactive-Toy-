 
/*
  Mood Buddy 心情娃娃
  Arduino Nano + RGB LED + Button

  RGB LED (共陽極)
  U -> 5V
  R -> D9
  B -> D11
  G -> D10

  Button
  IN  -> D2
  VCC -> 5V
  GND -> GND
*/

// ============================
// 1. 腳位設定
// ============================

const int buttonPin = 2;

const int redPin = 9;
const int greenPin = 10;
const int bluePin = 11;

// 按下時為 LOW
// 如果按鈕模組按下輸出 HIGH，改成 HIGH
const int PRESSED_STATE = LOW;

// RGB LED 為共陽極
const bool COMMON_ANODE = true;


// ============================
// 2. 遊戲分數設定
// ============================

int mood = 10;

const int MAX_MOOD = 20;
const int MIN_MOOD = 0;

bool sleeping = false;


// ============================
// 3. 時間設定（毫秒）
// ============================

// 5秒沒按開始扣分
const unsigned long WAIT_TIME = 5000;

// 每1秒扣1分
const unsigned long LOSE_INTERVAL = 1000;

// 0分10秒後睡眠
const unsigned long SLEEP_TIME = 10000;

// 按鈕防彈跳
const unsigned long DEBOUNCE_TIME = 40;


// ============================
// 4. 計時變數
// ============================

unsigned long lastPressTime = 0;
unsigned long lastLoseTime = 0;
unsigned long zeroStartTime = 0;

unsigned long lastDebounceTime = 0;

int lastReading;
int stableState;


// ============================
// 5. RGB LED 顏色控制
// ============================

void setColor(int r, int g, int b) {

  // 共陽極 LED 需要反轉 PWM 數值
  if (COMMON_ANODE) {
    r = 255 - r;
    g = 255 - g;
    b = 255 - b;
  }

  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}


// ============================
// 6. 根據分數顯示顏色
// ============================

void updateMoodLight() {

  int r = 0;
  int g = 0;
  int b = 0;

  // 0~10分：紅色逐漸變綠色
  if (mood <= 10) {

    r = map(mood, 0, 10, 255, 0);
    g = map(mood, 0, 10, 0, 255);
    b = 0;

  }

  // 11~20分：綠色逐漸變藍色
  else {

    r = 0;
    g = map(mood, 10, 20, 255, 0);
    b = map(mood, 10, 20, 0, 255);

  }


  // ==========================
  // 額外功能1：滿分心跳燈
  // ==========================

  if (mood == 20) {

    unsigned long phase = millis() % 1400;

    if (phase < 150) {
      b = map(phase, 0, 150, 80, 255);
    }
    else if (phase < 300) {
      b = map(phase, 150, 300, 255, 80);
    }
    else if (phase < 450) {
      b = map(phase, 300, 450, 80, 210);
    }
    else if (phase < 600) {
      b = map(phase, 450, 600, 210, 80);
    }
    else {
      b = 80;
    }

  }


  // ==========================
  // 額外功能2：睡眠模式
  // ==========================

  if (sleeping) {

    r = r / 20;
    g = g / 20;
    b = b / 20;

  }

  setColor(r, g, b);
}


// ============================
// 7. Arduino 初始化
// ============================

void setup() {

  // 先將共陽極 RGB 的輸出設為關閉
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);

  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  Serial.begin(9600);

  // 初始化按鈕狀態
  lastReading = digitalRead(buttonPin);
  stableState = lastReading;

  // 初始化時間
  lastPressTime = millis();
  lastLoseTime = millis();

  Serial.println("Mood Buddy Start!");
  Serial.println("Mood = 10");

  // 初始顯示綠色
  updateMoodLight();
}


// ============================
// 8. 遊戲主程式
// ============================

void loop() {

  unsigned long now = millis();

  // --------------------------
  // A. 偵測按鈕
  // --------------------------

  int reading = digitalRead(buttonPin);

  // 按鈕訊號變化時重新計時
  if (reading != lastReading) {
    lastDebounceTime = now;
  }

  // 防彈跳處理
  if (now - lastDebounceTime >= DEBOUNCE_TIME) {

    if (reading != stableState) {

      stableState = reading;

      // 按鈕被按下
      if (stableState == PRESSED_STATE) {

        // 如果正在睡眠，先喚醒
        sleeping = false;

        // 每按一次加1分
        if (mood < MAX_MOOD) {
          mood++;
        }

        // 重設時間
        lastPressTime = now;
        lastLoseTime = now;

        // 清除0分計時
        zeroStartTime = 0;

        // 顯示分數
        Serial.print("Pressed! Mood = ");
        Serial.println(mood);
      }
    }
  }

  lastReading = reading;


  // --------------------------
  // B. 5秒沒按開始扣分
  // --------------------------

  if (!sleeping &&
      now - lastPressTime >= WAIT_TIME &&
      now - lastLoseTime >= LOSE_INTERVAL) {

    if (mood > MIN_MOOD) {

      mood--;

      Serial.print("Mood decreased = ");
      Serial.println(mood);

      // 降到0分時開始計時
      if (mood == 0) {
        zeroStartTime = now;
      }
    }

    lastLoseTime = now;
  }


  // --------------------------
  // C. 0分10秒後睡眠
  // --------------------------

  if (!sleeping &&
      mood == 0 &&
      zeroStartTime != 0 &&
      now - zeroStartTime >= SLEEP_TIME) {

    sleeping = true;

    Serial.println("Sleeping...");
  }


  // --------------------------
  // D. 更新LED燈光
  // --------------------------

  updateMoodLight();

}
