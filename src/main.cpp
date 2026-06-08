#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

// Shared SPI pins
#define SPI_SCK  12
#define SPI_MISO 13
#define SPI_MOSI 11

// Individual Chip Selects
#define SD_CS_PIN     6
#define SCREEN_CS_PIN 10

// Screen control pins (Using your verified working pins!)
#define DC_PIN   9
#define RES_PIN  5
#define BUSY_PIN 4

// FIX 1: Instantiate the ONE shared custom SPI bus object
SPIClass sharedSpi = SPIClass(FSPI);

// Initializing screen on pins
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(
    GxEPD2_420_GDEY042T81(SCREEN_CS_PIN, DC_PIN, RES_PIN, BUSY_PIN)
);

void setup() {
  Serial.begin(115200);
  for(int i = 3; i > 0; i--) { delay(1000); }
  
  // Redundant software pull-ups
  pinMode(SPI_MISO, INPUT_PULLUP);
  pinMode(BUSY_PIN, INPUT_PULLUP);

  Serial.println("Booting Shared SPI bus...");
  // Start the shared SPI bus once
  sharedSpi.begin(SPI_SCK, SPI_MISO, SPI_MOSI, -1);

  // Link the shared SPI object to the screen library before init
  display.epd2.selectSPI(sharedSpi, SPISettings(2000000, MSBFIRST, SPI_MODE0));
  
  Serial.println("Initializing display...");
  display.init(115200, true, 50, false);

  Serial.println("Pinging card...");
  // Pass the sharedSpi object to the SD library
  if(!SD.begin(SD_CS_PIN, sharedSpi, 1000000)) {
    Serial.println("[ERROR]: Mount Failed. Board is physically unresponsive.");
    return;
  }
  Serial.println("[SUCCESS]: Card Mounted!");

  Serial.println("Executing full page refresh...");
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);

  display.setFullWindow();
  display.firstPage();

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);

  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(20, 50);
    // FIX 2: Use printf instead of print for formatting strings!
    display.printf("Size: %llu MB", cardSize);
  } while (display.nextPage());
  
  display.hibernate();

  Serial.printf("Size: %llu MB\n", cardSize);
}

void loop() {
  // Silent loop
}
