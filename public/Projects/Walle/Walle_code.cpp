//Calibration

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN  150   // Pulse length for 0°
#define SERVOMAX  600   // Pulse length for 180°
#define NUM_SERVOS 12   // Total number of servos/channels used
#define SERVO_TIMEOUT 1000  // milliseconds to hold position
#define PCA9685_ADDR 0x40   // Default I2C address for PCA9685

int activeChannel = -1;
unsigned long moveTime = 0;

void setup() {
  Serial.begin(115200);

  // Set I2C pins for ESP32 (SDA=21, SCL=22)
  Wire.begin(21, 22);
  delay(200);

  // Check if PCA9685 is connected
  Wire.beginTransmission(PCA9685_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println("❌ ERROR: PCA9685 not detected at address 0x40. Check wiring and power.");
    while (true) {
      delay(1000);  // Halt program
    }
  }

  // Initialize PCA9685
  pwm.begin();
  pwm.setPWMFreq(50); // 50 Hz typical for servos
  delay(500);

  Serial.println("✅ PCA9685 detected successfully!");
  Serial.println("Enter: <channel> <angle>");
  Serial.println("Example: 3 90");
}

void loop() {
  // Turn off active servo after timeout
  if (activeChannel != -1 && millis() - moveTime > SERVO_TIMEOUT) {
    pwm.setPWM(activeChannel, 0, 0);
    Serial.print("Timed out servo ");
    Serial.println(activeChannel);
    activeChannel = -1;
  }

  // Process serial input
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    int spaceIndex = input.indexOf(' ');
    if (spaceIndex == -1) {
      Serial.println("Invalid format. Use: <channel> <angle>");
      return;
    }

    int channel = input.substring(0, spaceIndex).toInt();
    int angle = input.substring(spaceIndex + 1).toInt();

    if (channel < 0 || channel >= NUM_SERVOS || angle < 0 || angle > 180) {
      Serial.println("Invalid channel or angle.");
      return;
    }

    // Turn off previous servo if switching channels
    if (activeChannel != -1 && activeChannel != channel) {
      pwm.setPWM(activeChannel, 0, 0);
    }

    int pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
    pwm.setPWM(channel, 0, pulse);

    Serial.print("Set servo ");
    Serial.print(channel);
    Serial.print(" to ");
    Serial.print(angle);
    Serial.println("°");

    activeChannel = channel;
    moveTime = millis();
  }
}

        
          Full Code but replace current servo angles with your servo angles
      
#include &lt;Bluepad32.h&gt;
#include &lt;Wire.h&gt;
#include &lt;Adafruit_PWMServoDriver.h&gt;
#include &lt;HardwareSerial.h&gt;
#include &quot;DFRobotDFPlayerMini.h&quot;
#include &lt;U8g2lib.h&gt;

// ===== DFPlayer Mini Setup =====
HardwareSerial dfSerial(1);  // Use UART1
DFRobotDFPlayerMini dfPlayer;
#define DF_RX 18  // ESP32 RX pin to DFPlayer TX
#define DF_TX 19  // ESP32 TX pin to DFPlayer RX

// Motor pins
#define ENA 13
#define IN1 12
#define IN2 14
#define ENB 25
#define IN3 27
#define IN4 26

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVO_MIN  150
#define SERVO_MAX  600

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// Servo positions
#define HEAD_LEFT     115
#define HEAD_FORWARD   85
#define HEAD_RIGHT     65
#define NECK_TOP_EXTEND 165
#define NECK_TOP_LOWER   15
#define NECK_BOTTOM_EXTEND 150
#define NECK_BOTTOM_LOWER   30
#define RIGHT_EYE_RAISED   90
#define RIGHT_EYE_LOWERED 150
#define LEFT_EYE_RAISED    60
#define LEFT_EYE_LOWERED    0
#define LEFT_ARM_RAISED    20
#define LEFT_ARM_LOWERED  100
#define RIGHT_ARM_RAISED  140
#define RIGHT_ARM_LOWERED  70

bool leftArmUp = false, rightArmUp = false;
bool leftEyeUp = false, rightEyeUp = false;
bool neckUp = false;

unsigned long lastServoTime[16] = {0};
const unsigned long servoTimeout = 1500;
unsigned long lastMotorTime = 0;
const unsigned long motorTimeout = 2000;

// Animations for D-Pad emotes
bool animA = false, animB = false, animX = false, animY = false;
unsigned long animTimers[4] = {0};
int animSteps[4] = {-1, -1, -1, -1};

// ===== OLED Battery Display Setup =====
#define OLED
#define BATTERY_PIN 33
#define R1 10000.0
#define R2 2000.0
#define ADC_MAX 4095.0
#define ADC_REF_VOLTAGE 3.3

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

#ifdef OLED
void displayLevel(int batlevel) {
  u8g2.firstPage();
  do {
    u8g2.setDrawColor(1);
    drawSun();
    drawBatt10();
    if (batlevel > 55) drawBatt20();
    if (batlevel > 60) drawBatt30();
    if (batlevel > 65) drawBatt40();
    if (batlevel > 70) drawBatt50();
    if (batlevel > 75) drawBatt60();
    if (batlevel > 80) drawBatt70();
    if (batlevel > 85) drawBatt80();
    if (batlevel > 90) drawBatt90();
    if (batlevel > 95) drawBatt100();

    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(40, 10);
    u8g2.print(batlevel);
    u8g2.print("%");
  } while (u8g2.nextPage());
}

void drawBatt10()  { u8g2.drawBox(108, 0, 16, 40); }
void drawBatt20()  { u8g2.drawBox(96, 0, 7, 40); }
void drawBatt30()  { u8g2.drawBox(84, 0, 7, 40); }
void drawBatt40()  { u8g2.drawBox(72, 0, 7, 40); }
void drawBatt50()  { u8g2.drawBox(60, 0, 7, 40); }
void drawBatt60()  { u8g2.drawBox(48, 0, 7, 40); }
void drawBatt70()  { u8g2.drawBox(36, 0, 7, 40); }
void drawBatt80()  { u8g2.drawBox(24, 0, 7, 40); }
void drawBatt90()  { u8g2.drawBox(12, 0, 7, 40); }
void drawBatt100() { u8g2.drawBox(0, 0, 7, 40); }

void drawSun() {
  u8g2.drawDisc(20, 55, 3);
  u8g2.drawLine(20, 50, 20, 46);
  u8g2.drawLine(20, 60, 20, 64);
  u8g2.drawLine(15, 55, 11, 55);
  u8g2.drawLine(25, 55, 29, 55);
  u8g2.drawLine(22, 54, 25, 47);
  u8g2.drawLine(25, 53, 29, 50);
  u8g2.drawLine(16, 53, 12, 50);
  u8g2.drawLine(18, 51, 16, 47);
  u8g2.drawLine(16, 58, 12, 60);
  u8g2.drawLine(18, 60, 16, 63);
  u8g2.drawLine(25, 58, 29, 60);
  u8g2.drawLine(22, 60, 25, 63);
}
#endif

float readBatteryVoltage() {
  int raw = analogRead(BATTERY_PIN);
  float vADC = (raw / ADC_MAX) * ADC_REF_VOLTAGE;
  float vBAT = vADC * ((R1 + R2) / R2);
  return vBAT;
}

int voltageToPercent(float v) {
  if (v >= 12.6) return 100;
  if (v <= 9.9) return 0;
  return (int)((v - 9.9) * 100.0 / (12.6 - 9.9));
}

uint16_t angleToPulse(int angle) {
  return map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
}

void moveServo(uint8_t ch, int angle) {
  pwm.setPWM(ch, 0, angleToPulse(angle));
  lastServoTime[ch] = millis();
}

void moveTwoServos(uint8_t ch1, int angle1, uint8_t ch2, int angle2) {
  moveServo(ch1, angle1);
  moveServo(ch2, angle2);
}

void stopInactiveServos() {
  for (int i = 0; i < 16; i++) {
    if (millis() - lastServoTime[i] > servoTimeout) {
      pwm.setPWM(i, 0, 0);
    }
  }
}

void driveMotor(int ena, int in1, int in2, int power) {
  lastMotorTime = millis();
  if (power > 0) {
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  } else if (power < 0) {
    digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  }
  analogWrite(ena, abs(power));
}

void stopMotorsIfInactive() {
  if (millis() - lastMotorTime > motorTimeout) {
    driveMotor(ENA, IN1, IN2, 0);
    driveMotor(ENB, IN3, IN4, 0);
  }
}

// Animation handlers for D-Pad emotes
void handleAnimationA() {
  if (!animA) return;
  unsigned long now = millis();
  switch (animSteps[0]) {
    case 0: 
      moveTwoServos(1, NECK_TOP_EXTEND, 2, NECK_BOTTOM_EXTEND); 
      animTimers[0] = now; animSteps[0]++;
      break;
    case 1: 
      if (now - animTimers[0] >= 200) { 
        moveTwoServos(5, LEFT_ARM_RAISED, 6, RIGHT_ARM_RAISED); 
        animTimers[0] = now; animSteps[0]++; 
      }
      break;
    case 2: 
      if (now - animTimers[0] >= 500) { 
        moveServo(0, HEAD_LEFT); 
        animTimers[0] = now; animSteps[0]++; 
      }
      break;
    case 3: 
      if (now - animTimers[0] >= 500) { 
        moveTwoServos(3, RIGHT_EYE_RAISED, 4, LEFT_EYE_RAISED); 
        animTimers[0] = now; animSteps[0]++; 
      }
      break;
    case 4: 
      if (now - animTimers[0] >= 1000) { 
        moveTwoServos(3, RIGHT_EYE_LOWERED, 4, LEFT_EYE_LOWERED); 
        moveServo(0, HEAD_FORWARD); 
        animTimers[0] = now; animSteps[0]++; 
      }
      break;
    case 5: 
      if (now - animTimers[0] >= 800) { 
        moveTwoServos(1, NECK_TOP_LOWER, 2, NECK_BOTTOM_LOWER); 
        moveTwoServos(5, LEFT_ARM_LOWERED, 6, RIGHT_ARM_LOWERED); 
        animA = false; animSteps[0] = -1; 
      }
      break;
  }
}

void handleAnimationB() {
  if (!animB) return;
  unsigned long now = millis();
  if (animSteps[1] == 0) { 
    moveServo(0, HEAD_RIGHT); 
    animTimers[1] = now; animSteps[1]++; 
  }
  else if (animSteps[1] == 1 && now - animTimers[1] >= 400) { 
    moveServo(0, HEAD_FORWARD); 
    animTimers[1] = now; animSteps[1]++; 
  }
  else if (animSteps[1] == 2 && now - animTimers[1] >= 400) { 
    moveTwoServos(3, RIGHT_EYE_RAISED, 4, LEFT_EYE_RAISED); 
    animTimers[1] = now; animSteps[1]++; 
  }
  else if (animSteps[1] == 3 && now - animTimers[1] >= 400) { 
    moveTwoServos(3, RIGHT_EYE_LOWERED, 4, LEFT_EYE_LOWERED); 
    animB = false; animSteps[1] = -1; 
  }
}

void handleAnimationX() {
  if (!animX) return;
  unsigned long now = millis();
  if (animSteps[2] == 0) { 
    moveTwoServos(5, LEFT_ARM_RAISED, 6, RIGHT_ARM_RAISED); 
    animTimers[2] = now; animSteps[2]++; 
  }
  else if (animSteps[2] == 1 && now - animTimers[2] >= 400) { 
    moveTwoServos(5, LEFT_ARM_LOWERED, 6, RIGHT_ARM_LOWERED); 
    animX = false; animSteps[2] = -1; 
  }
}

void handleAnimationY() {
  if (!animY) return;
  unsigned long now = millis();
  if (animSteps[3] == 0) { 
    moveServo(0, HEAD_LEFT); 
    animTimers[3] = now; animSteps[3]++; 
  }
  else if (animSteps[3] == 1 && now - animTimers[3] >= 400) { 
    moveServo(0, HEAD_RIGHT); 
    animTimers[3] = now; animSteps[3]++; 
  }
  else if (animSteps[3] == 2 && now - animTimers[3] >= 400) { 
    moveServo(0, HEAD_FORWARD); 
    animY = false; animSteps[3] = -1; 
  }
}

void handleAnimations() {
  handleAnimationA();
  handleAnimationB();
  handleAnimationX();
  handleAnimationY();
}

void processControllers() {
  for (auto ctl : myControllers) {
    if (ctl && ctl->isConnected() && ctl->hasData()) {

      // Driving motors
      int leftY = ctl->axisY();
      int rightY = ctl->axisRY();
      int leftPower = map(leftY, -512, 512, 255, -255);
      int rightPower = map(rightY, -512, 512, 255, -255);
      driveMotor(ENA, IN1, IN2, leftPower);
      driveMotor(ENB, IN3, IN4, rightPower);

      // A button → Play WALL·E sound
      if (ctl->a()) {
        dfPlayer.play(1); // Plays 0001.mp3 from SD card
        delay(200);
      }

      // Left Trigger → Toggle Left Arm high/low
      if (ctl->l2()) {
        leftArmUp = !leftArmUp;
        moveServo(5, leftArmUp ? LEFT_ARM_RAISED : LEFT_ARM_LOWERED);
        delay(200);
      }

      // Right Trigger → Toggle Right Arm high/low
      if (ctl->r2()) {
        rightArmUp = !rightArmUp;
        moveServo(6, rightArmUp ? RIGHT_ARM_RAISED : RIGHT_ARM_LOWERED);
        delay(200);
      }

      // Left Bumper + Right Bumper → Look straight
      if (ctl->l1() && ctl->r1()) {
        moveServo(0, HEAD_FORWARD);
      } 
      // Left Bumper → Look left
      else if (ctl->l1()) {
        moveServo(0, HEAD_LEFT);
      } 
      // Right Bumper → Look right
      else if (ctl->r1()) {
        moveServo(0, HEAD_RIGHT);
      }

      // X → Toggle Left Eye up/down, hold for straight
      static unsigned long xPressTime = 0;
      if (ctl->x()) {
        if (xPressTime == 0) {
          xPressTime = millis();
          leftEyeUp = !leftEyeUp;
          moveServo(4, leftEyeUp ? LEFT_EYE_RAISED : LEFT_EYE_LOWERED);
        } else {
          if (millis() - xPressTime > 500) {
            moveServo(4, (LEFT_EYE_RAISED + LEFT_EYE_LOWERED) / 2);
          }
        }
      } else {
        xPressTime = 0;
      }

      // B → Toggle Right Eye up/down, hold for straight
      static unsigned long bPressTime = 0;
      if (ctl->b()) {
        if (bPressTime == 0) {
          bPressTime = millis();
          rightEyeUp = !rightEyeUp;
          moveServo(3, rightEyeUp ? RIGHT_EYE_RAISED : RIGHT_EYE_LOWERED);
        } else {
          if (millis() - bPressTime > 500) {
            moveServo(3, (RIGHT_EYE_RAISED + RIGHT_EYE_LOWERED) / 2);
          }
        }
      } else {
        bPressTime = 0;
      }

      // Y → Toggle full neck up/down
      if (ctl->y()) {
        neckUp = !neckUp;
        moveTwoServos(1, neckUp ? NECK_TOP_EXTEND : NECK_TOP_LOWER,
                      2, neckUp ? NECK_BOTTOM_EXTEND : NECK_BOTTOM_LOWER);
        delay(200);
      }

      // D-Pad → Trigger Emotes
      int dpad = ctl->dpad();
      if (dpad == DPAD_UP && !animA) { animA = true; animSteps[0] = 0; }
      else if (dpad == DPAD_RIGHT && !animB) { animB = true; animSteps[1] = 0; }
      else if (dpad == DPAD_DOWN && !animX) { animX = true; animSteps[2] = 0; }
      else if (dpad == DPAD_LEFT && !animY) { animY = true; animSteps[3] = 0; }
    }
  }
}

// Callbacks
void onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (!myControllers[i]) { myControllers[i] = ctl; break; }
  }
}
void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) { myControllers[i] = nullptr; break; }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  pwm.begin(); pwm.setPWMFreq(50);
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();
  BP32.enableVirtualDevice(false);
  randomSeed(analogRead(0));

  // DFPlayer initialization
  dfSerial.begin(9600, SERIAL_8N1, DF_RX, DF_TX);
  if (!dfPlayer.begin(dfSerial)) {
    Serial.println("DFPlayer Mini not detected!");
  } else {
    Serial.println("DFPlayer Mini ready.");
    dfPlayer.volume(25); // Volume: 0-30
  }

  // OLED init
  u8g2.begin();
  analogReadResolution(12); // ESP32 ADC: 12-bit
}

void loop() {
  if (BP32.update()) processControllers();
  handleAnimations();
  stopMotorsIfInactive();
  stopInactiveServos();

  float voltage = readBatteryVoltage();
  int percent = voltageToPercent(voltage);
#ifdef OLED
  displayLevel(percent);
#endif

  delay(10);
}
