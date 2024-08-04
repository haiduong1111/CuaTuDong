#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RST_PIN         9 // Chân RST của RC522
#define SS_PIN          10 // Chân SDA của RC522
#define PIR_PIN         8 // Chân OUT của cảm biến PIR
#define MOTOR_ENA_PIN   6 // Chân ENA của L298N
#define MOTOR_ENB_PIN   7 // Chân ENB của L298N
#define MOTOR_IN1_PIN   2 // Chân IN1 của L298NA
#define MOTOR_IN2_PIN   3 // Chân IN2 của L298N
#define MOTOR_IN3_PIN   4 // Chân IN3 của L298N
#define MOTOR_IN4_PIN   5 // Chân IN4 của L298N
#define BUZZER_PIN      A1 // Chân để kết nối buzzer
#define ANALOG_IN_PIN   A0 // Chân kết nối cảm biến rung

MFRC522 mfrc522(SS_PIN, RST_PIN);

#define LCD_ADDR        0x27
#define LCD_COLUMNS     16
#define LCD_ROWS        2

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLUMNS, LCD_ROWS);

bool motorActivated = false; // Biến trạng thái để kiểm soát việc kích hoạt động cơ
bool cardPresent = false;    // Biến trạng thái để xác định nếu một thẻ đang được đặt trên đầu đọc
bool buzzerActivated = false; // Biến trạng thái để kiểm soát việc kích hoạt loa
unsigned long lcdLastUpdated = 0; // Biến lưu thời gian cập nhật cuối cùng trên màn hình LCD
const unsigned long lcdDisplayTime = 5000; // Thời gian hiển thị trên màn hình LCD (5 giây)
const unsigned long buzzerDuration = 100; // Thời gian kêu của loa (100ms)
int sensorVal = 0;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT); // Đặt chân A1 làm đầu ra cho buzzer
  pinMode(PIR_PIN, INPUT);
  pinMode(MOTOR_ENA_PIN, OUTPUT);
  pinMode(MOTOR_ENB_PIN, OUTPUT);
  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);
  pinMode(MOTOR_IN3_PIN, OUTPUT);
  pinMode(MOTOR_IN4_PIN, OUTPUT);

  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  lcd.init();
  lcd.backlight();
  displayScanCardMessage(); // Hiển thị "Quét thẻ" khi mới bật lên
}

void loop() {
  bool isMotionDetected = false;

  // Đọc giá trị từ cảm biến rung
  sensorVal = analogRead(ANALOG_IN_PIN);

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Handle card scanning
    handleCardScan();
  }

  if (digitalRead(PIR_PIN) == HIGH) {
    // Handle motion detection
    isMotionDetected = true;
  }

  handleMotorAction(isMotionDetected);
  
  // Kích hoạt buzzer nếu giá trị từ cảm biến rung nhỏ hơn 1019
  if (sensorVal < 1019) {
    activateBuzzer();
  }

  // Kiểm tra nếu đã hiển thị trên LCD đủ thời gian, thì xóa màn hình LCD và hiển thị lại "Quét thẻ"
  if (millis() - lcdLastUpdated >= lcdDisplayTime) {
    displayScanCardMessage();
  }
}

void handleCardScan() {
  // Check if the scanned card has the correct ID
  if (checkRFIDCardID()) {
    Serial.println("Card scanned! Accept");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Accept");
    lcdLastUpdated = millis(); // Ghi lại thời gian hiện tại khi hiển thị LCD
    // Reset motor activation status
    motorActivated = false;
    // Set card present flag to true
    cardPresent = true;
    // Activate the buzzer for 100ms
    tone(BUZZER_PIN, 1000); // Phát âm thanh ở tần số 1000Hz
    delay(100); // Dừng trong 100ms
    noTone(BUZZER_PIN); // Tắt buzzer
  } else {
    Serial.println("Unauthorized access! Reject");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reject");
    lcdLastUpdated = millis(); // Ghi lại thời gian hiện tại khi hiển thị LCD
    delay(2000);
    
    tone(BUZZER_PIN, HIGH); // Phát âm thanh ở tần số 1000Hz
    delay(100); // Dừng trong 100ms
    
    tone(BUZZER_PIN, HIGH); // Phát âm thanh ở tần số 1000Hz
    delay(300);
    noTone(BUZZER_PIN); // Tắt buzzer
    lcd.clear(); 
    cardPresent = false;
  }
}

void handleMotorAction(bool isMotionDetected) {
  if (cardPresent) {
    // If a card has been scanned successfully, proceed with motor action
    if (isMotionDetected) {
      Serial.println("Motion detected! Someone's here");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Someone's here");
      lcdLastUpdated = millis(); // Ghi lại thời gian hiện tại khi hiển thị LCD
      // Reset motor activation status
      motorActivated = false;
    }

    if (!motorActivated) {
      // Forward motion
      digitalWrite(MOTOR_IN1_PIN, HIGH);
      digitalWrite(MOTOR_IN2_PIN, LOW);
      digitalWrite(MOTOR_IN3_PIN, HIGH);
      digitalWrite(MOTOR_IN4_PIN, LOW);
      analogWrite(MOTOR_ENA_PIN, 255);
      analogWrite(MOTOR_ENB_PIN, 255);
      delay(1000); // Run motor for 1 second
      
      // Stop motor
      digitalWrite(MOTOR_IN1_PIN, LOW);
      digitalWrite(MOTOR_IN2_PIN, LOW);
      digitalWrite(MOTOR_IN3_PIN, LOW);
      digitalWrite(MOTOR_IN4_PIN, LOW);
      analogWrite(MOTOR_ENA_PIN, 0);
      analogWrite(MOTOR_ENB_PIN, 0);
      delay(5000); // Pause for 5 seconds
      
      // Reverse motion
      digitalWrite(MOTOR_IN1_PIN, LOW);
      digitalWrite(MOTOR_IN2_PIN, HIGH);
      digitalWrite(MOTOR_IN3_PIN, LOW);
      digitalWrite(MOTOR_IN4_PIN, HIGH);
      analogWrite(MOTOR_ENA_PIN, 255);
      analogWrite(MOTOR_ENB_PIN, 255);
      delay(1000); // Run motor for 1 second
      
      // Stop motor
      digitalWrite(MOTOR_IN1_PIN, LOW);
      digitalWrite(MOTOR_IN2_PIN, LOW);
      digitalWrite(MOTOR_IN3_PIN, LOW);
      digitalWrite(MOTOR_IN4_PIN, LOW);
      analogWrite(MOTOR_ENA_PIN, 0);
      analogWrite(MOTOR_ENB_PIN, 0);
      
      // Set motor activation status to true
      motorActivated = true;
    }
  }
}

bool checkRFIDCardID() {
  // Define the expected ID
  byte expectedID[] = {0x73, 0x08, 0x64, 0x16};

  // Check if the scanned card ID matches the expected ID
  for (int i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] != expectedID[i]) {
      return false; // ID does not match
    }
  }
  // All bytes match, ID is correct
  return true;
}

void activateBuzzer() {
  // Activate the buzzer only if it's not already activated
  if (!buzzerActivated) {
    // Turn on buzzer
    tone(BUZZER_PIN, HIGH); // Phát âm thanh ở tần số 3000Hz
    delay(2000); // Dừng trong 100ms
    noTone(BUZZER_PIN); // Tắt buzzer
    // Set buzzer activated flag to true
    buzzerActivated = true;
    // Set the buzzer end time
    unsigned long buzzerEndTime = millis() + buzzerDuration;
    
    // Keep the buzzer on until buzzer end time
    while (millis() < buzzerEndTime) {
      // Do nothing, just wait
    }
    
    // Reset buzzer activated flag
    buzzerActivated = false;
  }
}

void displayScanCardMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan card");
  lcdLastUpdated = millis(); // Cập nhật thời gian hiện tại
}
