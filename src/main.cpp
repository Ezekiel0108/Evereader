#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <OxProto_Regular14pt7b.h>

//Button pins
#define BUTTON_PIN_1 1
#define BUTTON_PIN_2 2
#define BUTTON_PIN_3 21

// Shared SPI pins
#define SPI_SCK  12
#define SPI_MOSI 11

// SD pins
#define SD_SCK_PIN  14
#define SD_MISO_PIN 17
#define SD_MOSI_PIN  6
#define SD_CS_PIN    16

//Right screen pin definitions
#define DC_R_PIN   9
#define RES_R_PIN  5
#define BUSY_R_PIN 4
#define SCREEN_R_CS_PIN 10

//Left screen pin definitions
#define DC_L_PIN   8
#define RES_L_PIN  15
#define BUSY_L_PIN 18
#define SCREEN_L_CS_PIN 7

GFXfont font = OxProto_Regular14pt7b;

int b1_state;
int b2_state;
int b3_state;
int b1_state_prev = HIGH;
int b2_state_prev = HIGH;
int b3_state_prev = HIGH;

SPIClass sharedSpi = SPIClass(FSPI);
SPIClass sdSpi = SPIClass(HSPI);

// Left display init
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> displayLeft(
    GxEPD2_420_GDEY042T81(SCREEN_L_CS_PIN, DC_L_PIN, RES_L_PIN, BUSY_L_PIN)
);

//Right display init
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> displayRight(
    GxEPD2_420_GDEY042T81(SCREEN_R_CS_PIN, DC_R_PIN, RES_R_PIN, BUSY_R_PIN)
);

void home_screen_print(){
  Serial.println("White paging everything!");

  do {
    displayRight.fillScreen(GxEPD_WHITE);
  } while (displayRight.nextPage());
  displayRight.hibernate();

  do {
    displayLeft.fillScreen(GxEPD_WHITE);
  } while (displayLeft.nextPage());
  displayLeft.hibernate();

}

void right_screen_print(){
  Serial.println("Printing to right screen!");
  do {
    displayRight.fillScreen(GxEPD_WHITE);
    displayRight.setCursor(20, 50);
    displayRight.printf("Right Screen!");

  } while (displayRight.nextPage());

  displayRight.hibernate();

}

void left_screen_print(){
  Serial.println("Printing to left screen!");
  do {
    displayLeft.fillScreen(GxEPD_WHITE);
    displayLeft.setCursor(20, 50);
    displayLeft.printf("Left Screen!");

  } while (displayLeft.nextPage());

  displayLeft.hibernate();

}

void setup() {
  Serial.begin(115200);
  for(int i = 3; i > 0; i--) { delay(1000); }
  
  //pinMode setup
  pinMode(SD_MISO_PIN, INPUT_PULLUP);
  pinMode(BUSY_L_PIN, INPUT_PULLUP);
  pinMode(BUSY_R_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);

  //Screens setup
  Serial.println("Setting up right screen!");
  displayRight.setRotation(1);
  displayRight.setFont(&font);
  displayRight.setTextColor(GxEPD_BLACK);
  displayRight.setFullWindow();
  displayRight.firstPage();

  Serial.println("Printing left screen!");
  displayLeft.setRotation(1);
  displayLeft.setFont(&font);
  displayLeft.setTextColor(GxEPD_BLACK);
  displayLeft.setFullWindow();
  displayLeft.firstPage();

  //Booting up SPIs and selecting display ones
  sharedSpi.begin(SPI_SCK, -1, SPI_MOSI, -1);
  sdSpi.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, -1);
  displayLeft.epd2.selectSPI(sharedSpi, SPISettings(2000000, MSBFIRST, SPI_MODE0));
  displayRight.epd2.selectSPI(sharedSpi, SPISettings(2000000, MSBFIRST, SPI_MODE0));
  
  //Initializing both displays
  displayLeft.init(115200, true, 50, false);
  displayRight.init(115200, true, 50, false);

  //Card mount and display to serial
  Serial.println("Pinging card...");
  if(!SD.begin(SD_CS_PIN, sdSpi, 1000000)) {
    Serial.println("[ERROR]: Mount Failed. Board is physically unresponsive.");
    return;
  }
  Serial.println("[SUCCESS]: Card Mounted!");
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("Size: %llu MB\n", cardSize);
}

void loop() {
  b1_state = digitalRead(BUTTON_PIN_1);
  b2_state = digitalRead(BUTTON_PIN_2);
  b3_state = digitalRead(BUTTON_PIN_3);

  if(b1_state_prev == HIGH && b1_state == LOW){
    left_screen_print();
  }
  if(b2_state_prev == HIGH && b2_state == LOW){
    home_screen_print();
  }
  if(b3_state_prev == HIGH && b3_state == LOW){
    right_screen_print();
  }

  b1_state_prev = b1_state;
  b2_state_prev = b2_state;
  b3_state_prev = b3_state;
}

