#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <OxProto_Regular14pt7b.h>

// Shared SPI pins
#define SPI_SCK  12
#define SPI_MISO 13
#define SPI_MOSI 11

// Sd specific pins
#define SD_CS_PIN     6

//Left screen pin definitions
#define DC_L_PIN   9
#define RES_L_PIN  5
#define BUSY_L_PIN 4
#define SCREEN_L_CS_PIN 10

//Right screen pin definitions
#define DC_R_PIN   8
#define RES_R_PIN  15
#define BUSY_R_PIN 18
#define SCREEN_R_CS_PIN 7

SPIClass sharedSpi = SPIClass(FSPI);

// Left display init
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> displayLeft(
    GxEPD2_420_GDEY042T81(SCREEN_L_CS_PIN, DC_L_PIN, RES_L_PIN, BUSY_L_PIN)
);

//Right display init
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> displayRight(
    GxEPD2_420_GDEY042T81(SCREEN_R_CS_PIN, DC_R_PIN, RES_R_PIN, BUSY_R_PIN)
);


void setup() {
  Serial.begin(115200);
  for(int i = 3; i > 0; i--) { delay(1000); }
  
  pinMode(SPI_MISO, INPUT_PULLUP);
  pinMode(BUSY_L_PIN, INPUT_PULLUP);
  pinMode(BUSY_R_PIN, INPUT_PULLUP);

  Serial.println("Booting Shared SPI bus...");
  sharedSpi.begin(SPI_SCK, SPI_MISO, SPI_MOSI, -1);

  displayLeft.epd2.selectSPI(sharedSpi, SPISettings(2000000, MSBFIRST, SPI_MODE0));
  displayRight.epd2.selectSPI(sharedSpi, SPISettings(2000000, MSBFIRST, SPI_MODE0));
  
  Serial.println("Initializing display...");
  displayLeft.init(115200, true, 50, false);
  displayRight.init(115200, true, 50, false);

  Serial.println("Pinging card...");
  if(!SD.begin(SD_CS_PIN, sharedSpi, 1000000)) {
    Serial.println("[ERROR]: Mount Failed. Board is physically unresponsive.");
    return;
  }

  Serial.println("[SUCCESS]: Card Mounted!");
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);


  Serial.println("Executing full left page refresh...");
  displayLeft.setRotation(1);
  displayLeft.setFont(&OxProto_Regular14pt7b);
  displayLeft.setTextColor(GxEPD_BLACK);
  displayLeft.setFullWindow();
  displayLeft.firstPage();

  Serial.println("Printing to left screen");
  do {
    displayLeft.fillScreen(GxEPD_WHITE);
    displayLeft.setCursor(20, 50);
    displayLeft.printf("Size: %llu MB", cardSize);

  } while (displayLeft.nextPage());

  displayLeft.hibernate();

  Serial.println("Executing full right page refresh...");
  displayRight.setRotation(1);
  displayRight.setFont(&OxProto_Regular14pt7b);
  displayRight.setTextColor(GxEPD_BLACK);
  displayRight.setFullWindow();
  displayRight.firstPage();

  Serial.println("Printing to right screen");
  do {
    displayRight.fillScreen(GxEPD_WHITE);
    displayRight.setCursor(20, 50);
    displayRight.printf("Initializing right display after left");

  } while (displayRight.nextPage());

  displayRight.hibernate();

  Serial.printf("Size: %llu MB\n", cardSize);
}

void loop() {
}
