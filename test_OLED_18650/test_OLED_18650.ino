#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <driver/adc.h>

// การตั้งค่าหน้าจอและพิน
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define I2C_SDA 2
#define I2C_SCL 3
#define ADC_CHAN ADC1_CHANNEL_5 // GPIO 6

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ตัวแปรสำหรับเก็บค่ากราฟ
int values[SCREEN_WIDTH]; // เก็บค่าตามความกว้างหน้าจอ
int pos = 0;               // ตำแหน่งปัจจุบันของกราฟ

void setup() {
  Serial.begin(115200);

  // 1. ตั้งค่า ADC (Low Level ตามที่ต้องการ)
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC_CHAN, ADC_ATTEN_DB_11);

  // 2. ตั้งค่า I2C และ OLED
  Wire.begin(I2C_SDA, I2C_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;); // หยุดถ้าติดต่อจอไม่ได้
  }

  display.clearDisplay();
  
  // ล้างค่าใน Array เริ่มต้น
  for(int i=0; i<SCREEN_WIDTH; i++) values[i] = SCREEN_HEIGHT - 1;
}

void loop() {
  // --- 1. อ่านค่า Analog ---
  uint32_t raw = adc1_get_raw(ADC_CHAN);
  // คำนวณเป็น Volt (ใช้สูตรเดิมของคุณ)
  float voltage = (float)(map(raw, 0, 4095, 0, 3300)) / 1000.00;

  // --- 2. ประมวลผลกราฟ ---
  // แปลงค่า Raw (0-4095) ให้พอดีกับความสูงจอ (0-63) 
  // และกลับค่าเพื่อให้ 0 อยู่ด้านล่างจอ
  int y_val = map(raw, 0, 4095, SCREEN_HEIGHT - 1, 10); 

  // เลื่อนค่าใน Array ไปทางซ้าย (Shift left) เพื่อให้กราฟวิ่ง
  for(int i = 0; i < SCREEN_WIDTH - 1; i++) {
    values[i] = values[i+1];
  }
  values[SCREEN_WIDTH - 1] = y_val; // เก็บค่าใหม่ไว้ที่ท้ายสุด

  // --- 3. วาดภาพลงจอ ---
  display.clearDisplay();
  
  // วาดเส้นกราฟ
  for(int x = 0; x < SCREEN_WIDTH - 1; x++) {
    display.drawLine(x, values[x], x + 1, values[x+1], SSD1306_WHITE);
  }

  // แสดงตัวเลข Volt ที่มุมบนซ้าย
  display.fillRect(0, 0, 60, 10, SSD1306_BLACK); // ลบพื้นหลังส่วนข้อความ
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(voltage, 2);
  display.print(" V");

  // แสดงผลบนจอจริง
  display.display();

  delay(10); // ปรับความเร็วของกราฟที่นี่
}