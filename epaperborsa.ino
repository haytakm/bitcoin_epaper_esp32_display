/* Includes ------------------------------------------------------------------*/
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <stdlib.h>

// WiFi bilgileri
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Bitcoin fiyatı API'si
const char* api_url = "https://api.coindesk.com/v1/bpi/currentprice/BTC.json";

UBYTE *BlackImage;
/* Entry point ----------------------------------------------------------------*/
void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    printf("EPD_2IN9_test Demo\r\n");
    DEV_Module_Init();

    printf("e-Paper Init and Clear...\r\n");
    EPD_2IN9_Init(EPD_2IN9_FULL);
    EPD_2IN9_Clear();
    DEV_Delay_ms(500);

    /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
    UWORD Imagesize = ((EPD_2IN9_WIDTH % 8 == 0)? (EPD_2IN9_WIDTH / 8 ): (EPD_2IN9_WIDTH / 8 + 1)) * EPD_2IN9_HEIGHT;
    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        while(1);
    }
    printf("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_2IN9_WIDTH, EPD_2IN9_HEIGHT, 270, WHITE);

    fetchAndDisplayBitcoinPrice();

}

/* The main loop -------------------------------------------------------------*/
void loop()
{
  // Her 10 dakikada bir fiyatı güncelle
    delay(600000);
    fetchAndDisplayBitcoinPrice();
}

void fetchAndDisplayBitcoinPrice() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(api_url);
        int httpCode = http.GET();

        if (httpCode > 0) {
            String payload = http.getString();
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);
            float bitcoinPrice = doc["bpi"]["USD"]["rate_float"];

            // Ekranı temizle ve yeni fiyatı göster
            Paint_SelectImage(BlackImage);
            Paint_Clear(WHITE);
            Paint_DrawString_EN(10, 10, "BTC Price:", &Font24, WHITE, BLACK);
            char priceStr[10];
            sprintf(priceStr, "$%.2f", bitcoinPrice);
            Paint_DrawString_EN(10, 40, priceStr, &Font24, WHITE, BLACK);

            EPD_2IN9_Display(BlackImage);
            DEV_Delay_ms(599999);
        } else {
            Serial.println("Error on HTTP request");
        }
        http.end();
    } else {
        Serial.println("WiFi not connected");
        free(BlackImage);
        BlackImage = NULL;
    }
}