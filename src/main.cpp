#include <M5Core2.h>
#include <WiFi.h>
#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include "Adafruit_TCS34725.h"
#include "colors.h"


#define OUTPUT_GAIN 50
#define COMMAND_PLAY 1

typedef struct _audioCommand {
  int cmd;
  char* fileName;
} audioCommand;

QueueHandle_t commandQueue;
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_154MS, TCS34725_GAIN_16X);

// 与えられたRGBにもっとも近いcolorvoiceを返す
// colorVoiceの定義はcolors.hを参照
ColorVoice getNearestColorVoice(uint16_t r, uint16_t g, uint16_t b){
  ColorVoice nearestColorVoice;
  float minDistance = 450; //0,0,0から255,255,255までの距離より大きく
  for(int i=0;i<sizeof(colorVoices)/sizeof(ColorVoice);i++){
    ColorVoice colorVoice = colorVoices[i];
    float distance = sqrt(sq(colorVoice.r - r)+sq(colorVoice.g - g)+sq(colorVoice.b - b));
    if(distance<minDistance){
      minDistance = distance;
      nearestColorVoice = colorVoice;
    }
  }
  return nearestColorVoice;
}

bool sendCommand(audioCommand* command){
  int i;
  for(i=0;i<5;i++){
      BaseType_t ret = xQueueSend(commandQueue, &command, 1000);
      if(ret==pdTRUE){
        Serial.println("Command has queued");
        return true;
      }
      delay(1);
  }
  if(i==5){
    Serial.println("Sending command failed");
    return false;
  }  
}


void audioTask(void* pvParameters){
  AudioFileSourceSPIFFS *file = NULL;
  Serial.println("Audio task started");
  audioLogger = &Serial;
  Serial.println("setup ESP8266 audio library...");
  AudioOutputI2S *out = new AudioOutputI2S(I2S_NUM_1, 0); // Output to ExternalDAC
  out->SetPinout(12, 0, 2);
  out->SetOutputModeMono(true);
  out->SetGain((float)OUTPUT_GAIN/100.0);
  AudioGeneratorWAV *wav = new AudioGeneratorWAV();
  Serial.println("done!");
  while(1){
      audioCommand *command = NULL;
      // コマンドがqueueにたまっていれば処理する
      if(xQueueReceive(commandQueue, &command, 0)==pdTRUE){
        Serial.printf("command received: %d\n", command->cmd);
        switch(command->cmd){
          case COMMAND_PLAY:
            if(wav->isRunning()) wav->stop();
            if(file!=NULL){
              delete file;
              file = NULL;
            }
            file = new AudioFileSourceSPIFFS(command->fileName);
            wav->begin(file, out);
            Serial.println("audiofile started");
            break;
        }
        free(command);
        command = NULL;
      }
      if (wav->isRunning()) {
        if (!wav->loop()) wav->stop();
      }
      delay(1);
  }
}

void setup()
{
  M5.begin(true, true, true, true, kMBusModeOutput, false);
  delay(50);
  if(!SPIFFS.begin()) Serial.println("Spiffs mount failed!");
  M5.Axp.SetSpkEnable(true);
  WiFi.mode(WIFI_OFF); 
  delay(500);

  M5.Lcd.setTextFont(2);
  M5.Lcd.printf("Sample WAV playback begins...\n");
  Serial.print("AudioCommandQueue create...");
  commandQueue = xQueueCreate(4, sizeof(void *));
  Serial.println("Audio task create...");
  xTaskCreateUniversal(
    audioTask,
    "audioTask",
    8192,
    NULL,
    2,
    NULL,
    APP_CPU_NUM
  );
  {
    audioCommand* command = new audioCommand();
    command->cmd = COMMAND_PLAY;
    command->fileName = "/boot.wav";
    sendCommand(command);
  }
  Serial.println("Color sensor setup...");
  while(!tcs.begin());
  Serial.println("done!");
}

void loop()
{
  m5.update();
  if(m5.BtnA.wasReleased()){
    Serial.println("BTNa was pressed.");
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);
    // r = r>>8;
    // g = g>>8;
    // b = b>>8;
    Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
    Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
    Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
    Serial.println(" ");
    //   ColorVoice voice = getNearestColorVoice(r, g, b);
    //   Serial.println(voice.fileName);
    //   if(SPIFFS.exists(voice.fileName)){
      //     audioCommand* command = new audioCommand();
      //     command->cmd = COMMAND_PLAY;
      //     command->fileName = voice.fileName;
      //     sendCommand(command);
    //   }
  }
  delay(100);
}
