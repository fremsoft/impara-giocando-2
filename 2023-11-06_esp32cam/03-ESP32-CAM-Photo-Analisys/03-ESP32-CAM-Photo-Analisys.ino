/* 
 *  Questo Sketch serve a elaborare le immagini 
 *  acquisite con la telecamera di ESP32-CAM
 *  direttamente a bordo di ESP32-CAM
 *  
 *  Visualizza l'esperienza di laboratorio completa:  
 *  https://youtube.com/live/iDwJ_sivwzE
 *  
 */
  
#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
 
RTC_DATA_ATTR int bootCount = 0;

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// 4 for flash led or 33 for normal led
#define LED_GPIO_NUM       4

/*  
typedef struct {
    uint8_t * buf;              Pointer to the pixel data
    size_t len;                 Length of the buffer in bytes
    size_t width;               Width of the buffer in pixels
    size_t height;              Height of the buffer in pixels
    pixformat_t format;         Format of the pixel data
    struct timeval timestamp;   Timestamp since boot of the first DMA buffer of the frame
} camera_fb_t;
*/
camera_fb_t *fb;
int pictureNumber;
  
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
 
  Serial.setDebugOutput(true);
 
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 5000000; //20000000;
  config.pixel_format = PIXFORMAT_RGB565; //PIXFORMAT_JPEG;

  /*
 typedef enum {
    PIXFORMAT_RGB565,    // 2BPP/RGB565
    PIXFORMAT_YUV422,    // 2BPP/YUV422
    PIXFORMAT_YUV420,    // 1.5BPP/YUV420
    PIXFORMAT_GRAYSCALE, // 1BPP/GRAYSCALE
    PIXFORMAT_JPEG,      // JPEG/COMPRESSED
    PIXFORMAT_RGB888,    // 3BPP/RGB888
    PIXFORMAT_RAW,       // RAW
    PIXFORMAT_RGB444,    // 3BP2P/RGB444
    PIXFORMAT_RGB555,    // 3BP2P/RGB555
} pixformat_t;
   */

  pinMode(LED_GPIO_NUM, INPUT);
  digitalWrite(LED_GPIO_NUM, LOW);
  rtc_gpio_hold_dis(GPIO_NUM_4);
 /*
  * ACCENSIONE DEL FLASH SU GPIO4
  * 
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  rtc_gpio_hold_dis(GPIO_NUM_4);
  */
  
  config.frame_size = FRAMESIZE_QQVGA; 
  config.fb_count = 1;

/*
typedef enum {
    FRAMESIZE_96X96,    // 96x96
    FRAMESIZE_QQVGA,    // 160x120
    FRAMESIZE_QCIF,     // 176x144
    FRAMESIZE_HQVGA,    // 240x176
    FRAMESIZE_240X240,  // 240x240
    FRAMESIZE_QVGA,     // 320x240
    FRAMESIZE_CIF,      // 400x296
    FRAMESIZE_HVGA,     // 480x320
    FRAMESIZE_VGA,      // 640x480
    FRAMESIZE_SVGA,     // 800x600
    FRAMESIZE_XGA,      // 1024x768
    FRAMESIZE_HD,       // 1280x720
    FRAMESIZE_SXGA,     // 1280x1024
    FRAMESIZE_UXGA,     // 1600x1200
    // 3MP Sensors
    FRAMESIZE_FHD,      // 1920x1080
    FRAMESIZE_P_HD,     //  720x1280
    FRAMESIZE_P_3MP,    //  864x1536
    FRAMESIZE_QXGA,     // 2048x1536
    // 5MP Sensors
    FRAMESIZE_QHD,      // 2560x1440
    FRAMESIZE_WQXGA,    // 2560x1600
    FRAMESIZE_P_FHD,    // 1080x1920
    FRAMESIZE_QSXGA,    // 2560x1920
    FRAMESIZE_INVALID
} framesize_t;
 */
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  pictureNumber = 0;
  fb = NULL;
} 

void loop() {
  uint32_t i, sumR, sumG, sumB, val;
  int x, y, n;
  
  // Take Picture with Camera
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }

/*
  Serial.print("Acquisito immagine ");
  Serial.print( fb->len / 2 );
  Serial.println(" pixel in formato RGB565");
*/

  i=0, x=0, y=0;
  sumR=0, sumG=0, sumB=0, n=0; 
  while (i < fb->len) {
    val  = (fb->buf[i] & 0xFF) << 8;          i++;
    val |= (fb->buf[i] & 0xFF);               i++;    

    if ((x >= 30) && (x <= 60) && (y >= 30) && (y <= 60)) {
      /* area di interesse */
      
      sumR += ((val >> 11) << 3) & 0xFF;
      sumG += ((val >>  5) << 2) & 0xFF;
      sumB += ((val & 0x1F) << 3);
      n++;
      
    }

    x++;
    if (x >= fb->width) {
      x = 0; y++;
    }
  }

  if (n != 0) { /* assert */
    int r,g,b;
    r = sumR / n;
    g = sumG / n;
    b = sumB / n;
    Serial.print("r:");
    Serial.print(r);
    Serial.print(", g:");
    Serial.print(g);
    Serial.print(", b:");
    Serial.println(b);
  }
    
  // libera le risorse
  esp_camera_fb_return(fb);
  
  //delay(100);

}
