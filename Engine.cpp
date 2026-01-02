#include <Adafruit_SSD1306.h>
#include <Wire.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define Hmap 16
#define Wmap 16
uint8_t Map[Hmap][Wmap/8] {
  {0b11111111, 0b11111111},
  {0b10000000, 0b00000001},
  {0b10000000, 0b00000001},
  {0b10000000, 0b00000001},
  {0b10000000, 0b00000001},
  {0b10000000, 0b00000001},
  {0b10000000, 0b00000001},
  {0b10000000, 0b00000001},
  {0b10000000, 0b00000001},
  {0b10000000, 0b00000001},
  {0b10000000, 0b00000001},
  {0b10000000, 0b00000001},
  {0b10000000, 0b00000001},
  {0b10000000, 0b00000001},
  {0b10000000, 0b00000001},
  {0b11111111, 0b11111111},
};

float sinLUT[360];
float cosLUT[360];


#define but1 2
#define but2 1
#define but3 0

float posx = 1.5;
float posy = 1.5;
float angle = 90.0;
float viewDist = 5.0;
float fov = 60.0;
float InspectSpeed = 10.0;
bool shake = 0;
float Playerspeed = 0.3;

byte fps = 0;
uint64_t tmr1 = 0;
uint64_t tmr2 = 0;
uint64_t tmr3 = 0;

void setup() {
  for (int i = 0; i < 360; i++) {
    sinLUT[i] = sin(radians(i));
    cosLUT[i] = cos(radians(i));
  }
  Serial.begin(115200);
  pinMode(but1, INPUT);
  pinMode(but2, INPUT);
  pinMode(but3, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);           
  display.setTextColor(SSD1306_WHITE);      
  display.setCursor(0,0); 
  display.display();
  render();
  display.display();


}

void loop() {
  fps++;
  if (millis()-tmr1 >= 1000) {
    tmr1 = millis();
    Serial.println(fps);
    fps = 0;
  }
  if (millis()-tmr2 >= 10) {
    tmr2 = millis();
    render();
  }
  if (digitalRead(but1)) {
    if (angle == 0.0) {
      angle = 360.0;
    }else{
      angle -= InspectSpeed;
    }
    Serial.println(angle);
  }else if (digitalRead(but2)) {
    if (angle == 360.0) {
      angle = 0.0;
    }else{
      angle += InspectSpeed;
    }
    Serial.println(angle);
  }
  if (digitalRead(but3)) {
    if (millis()-tmr3 >= 300) {
      tmr3 = millis();
      shake = !shake;
    }
    float newX = posx + cos(radians(angle)) * Playerspeed;
    float newY = posy + sin(radians(angle)) * Playerspeed;
      
    if (canMoveTo(newX, posy)) posx = newX;
    if (canMoveTo(posx, newY)) posy = newY;
  }

}


bool canMoveTo(float x, float y) {
  if ((int)x < 0 || (int)x >= Wmap || (int)y < 0 || (int)y >= Hmap) { return false;}
  return !(Map[(int)y][(int)x/8] & (1 << ((int)x % 8)));
}

void render() {
  display.clearDisplay();
  
  for (int i = 0; i < SCREEN_WIDTH; i += 4) { // Рендерим каждый 2-й столбец
    float rayAngle = angle - fov/2 + (fov * i) / SCREEN_WIDTH;
    rayAngle = fmod(rayAngle + 360, 360);
    
    float rayX = posx, rayY = posy;
    float rayDirX = cosLUT[(int)rayAngle];
    float rayDirY = sinLUT[(int)rayAngle];
    
    float dist = 0;
    bool hitWall = false;
    
    while (!hitWall && dist < viewDist) {
      rayX += rayDirX * 0.4; // Шаг луча
      rayY += rayDirY * 0.4;
      dist += 0.4;
      
      if (rayX < 0 || rayX >= Wmap || rayY < 0 || rayY >= Hmap) {
        break; // Выход за границы карты
      }
      
      if (Map[(int)rayY][(int)rayX / 8] & (1 << ((int)rayX % 8))) {
        hitWall = true;
      }
    }
    
    // Упрощенный расчет высоты стены
    int wallHeight = (int)(SCREEN_HEIGHT / (dist + 0.001)); // +0.001 чтобы избежать деления на 0
    wallHeight = min(wallHeight, SCREEN_HEIGHT);
    
    // Рисуем столбец (в 2 раза шире для компенсации разрешения)
    display.fillRect(i, (SCREEN_HEIGHT - wallHeight) / 2, 4, wallHeight, SSD1306_WHITE);
  }
  
  display.display();
}
