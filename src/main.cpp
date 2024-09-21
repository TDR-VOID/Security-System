#include <Arduino.h>

#include "FS.h" // File System
#include "SD.h" // SD Cards
#include "SPI.h" // Serial Peripheral Interface

#define PIR_PIN 13
#define SpeakPin 14
#define LDR_PIN 34

int LDRValue = 0;

TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1code( void * pvParameters );
void Task2code( void * pvParameters );

String sensorValueString;


void writeFile(fs::FS &fs, const char * path, const char * message, bool append = false){
  Serial.printf("Writing file: %s\n", path);

  File file;
  if (append) {
    file = fs.open(path, FILE_APPEND); // Open file in append mode
  } else {
    file = fs.open(path, FILE_WRITE); // Open file in write mode
  }

  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }

  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void setup(){
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT_PULLDOWN);
  pinMode(SpeakPin, OUTPUT);


  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */


  xTaskCreatePinnedToCore(
    Task2code, /* Task function. */
    "Task2",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task2,    /* Task handle to keep track of created task */
    1);        /* pin task to core 1 */


  if(!SD.begin(5)){ // CS pin - 5 (SPI communication)
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

}

void Task1code(void * pvParameters) {
  while (1) {  // Run indefinitely

      // Get the time since the program started
    unsigned long currentMillis = millis();
    unsigned long seconds = currentMillis / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;


    String timeString = String(hours) + "h " + String(minutes % 60) + "m " + String(seconds % 60) + "s";
    Serial.print("Elapsed Time: ");
    Serial.print(timeString);

    // Write the time to the SD card
    int sensorValue = digitalRead(PIR_PIN);  // Read the PIR sensor value (HIGH or LOW)
    String txt = timeString + " | "+sensorValueString;
    Serial.println(txt);
    writeFile(SD, "/time_log1.txt", txt.c_str(), true);
    
    delay(1000); // Log every second
    }
  }
  

void Task2code(void * pvParameters) {
  while (1) {  // Run indefinitely
  // Serial.println("Task2 Running");
  LDRValue = analogRead(LDR_PIN);
  String LDRValueString = String(LDRValue);
  int sensorValue = digitalRead(PIR_PIN);  // Read the PIR sensor value (HIGH or LOW)
  
  if (sensorValue == HIGH) {
    Serial.println("Motion Detected! LDR Value: "+LDRValueString);
    sensorValueString = "Motion Detected... | LDR Value: "+LDRValueString+"\n";
    digitalWrite(SpeakPin, HIGH);
    delay(500);  // Wait for 2 seconds
    digitalWrite(SpeakPin, LOW);
    delay(2000);  // Wait for 2 seconds
  
  } 
  else if (sensorValue == LOW){
    Serial.println("No Motion !  LDR Value: "+LDRValueString);
    sensorValueString = "No Motion... | LDR Value: "+LDRValueString+"\n";
  }
  
  else {
    Serial.println("Undefined");
    sensorValueString = "Undefined...\n";
    
  }
  
  delay(1000);  // Wait for 1 second before the next reading
  }
}

void loop(){
  
}