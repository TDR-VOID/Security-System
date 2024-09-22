#include <Arduino.h>

#include "FS.h" // File System
#include "SD.h" // SD Cards
#include "SPI.h" // Serial Peripheral Interface

#define PIR_PIN 13
#define SpeakPin 14
#define LDR_PIN 34
#define BUTTON1_PIN 12
#define BUTTON2_PIN 27
#define BUTTON3_PIN 33
#define LED_Task1 32
#define LED_Task2 26
#define LED 25

volatile bool  
 = false;
volatile bool task2Active = false;
volatile bool startTasks = false;
volatile bool bothTasksActive = false;

int LDRValue = 0;
int k = 1;
bool in = false;

TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1code( void * pvParameters );
void Task2code( void * pvParameters );
void IRAM_ATTR button1ISR();
void IRAM_ATTR button2ISR();
void IRAM_ATTR button3ISR();

String sensorValueString;


unsigned long lastDebounceTime = 0;  // To handle button debouncing
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long lastDebounceTime3 = 0;
unsigned long debounceDelay = 50;    // Minimum delay between button presses

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
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(LED_Task1, OUTPUT);
  pinMode(LED_Task2, OUTPUT);
  pinMode(LED, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), button1ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), button2ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON3_PIN), button3ISR, FALLING);


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
  if (cardType != CARD_SD) {
    digitalWrite(LED, HIGH);
    in = true;
  }
  

}

// ISR function for button press
// ISR function for button press
void IRAM_ATTR button1ISR() {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime1) > debounceDelay) {
    task1Active = true;
    task2Active = false;
    // bothTasksActive = false;  // Disable both-tasks mode
    lastDebounceTime1 = currentTime;
  }
}
void IRAM_ATTR button2ISR() {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime2) > debounceDelay) {
    task1Active = false;
    task2Active = true;
    // bothTasksActive = false;  // Disable both-tasks mode
    lastDebounceTime2 = currentTime;
  }
}


void IRAM_ATTR button3ISR() {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime3) > debounceDelay) {
    bothTasksActive = !bothTasksActive;  // Toggle both-tasks mode
    // task1Active = bothTasksActive;
    // task2Active = bothTasksActive;
    if (bothTasksActive) {
      task1Active = true;
      task2Active = true;
    } else {
      task1Active = false;
      task2Active = false;
    }

    lastDebounceTime3 = currentTime;
  }
}


void Task1code(void * pvParameters) {
  while (1) {  // Run indefinitely
    if (task1Active) {
      if (in == false)
      {
        digitalWrite(LED, HIGH);
        delay(1000);
        digitalWrite(LED, LOW);
        delay(1000);
      }
      
    digitalWrite(LED_Task1, HIGH);

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
    else {
      digitalWrite(LED_Task1, LOW);
      Serial.println("Task1 Not Active\n"); 
      delay(1500);
  }
}
}


void Task2code(void * pvParameters) {
  while (1) {  // Run indefinitely
    if (task2Active) {

      if (in == false)
      {
        digitalWrite(LED, HIGH);
        delay(1000);
        digitalWrite(LED, LOW);
        delay(1000);
      }
      
  digitalWrite(LED_Task2, HIGH);
    
  // Serial.println("Task2 Running");
  LDRValue = analogRead(LDR_PIN);
  String LDRValueString = String(LDRValue);
  int sensorValue = digitalRead(PIR_PIN);  // Read the PIR sensor value (HIGH or LOW)

  if (k == 0 || LDRValue > 2300) {
    for (int i = 0; i < 10; i++){
    digitalWrite(SpeakPin, HIGH);
    delay(200);
    digitalWrite(SpeakPin, LOW);
    delay(200);
    }
    k = 1;
  }
  
  if (sensorValue == HIGH) {
    Serial.println("Motion Detected! LDR Value: "+LDRValueString);
    sensorValueString = "Motion Detected... | LDR Value: "+LDRValueString+"\n";
    k =0;
    delay(100);  
  
  } 
  else if (sensorValue == LOW){
    Serial.println("No Motion !  LDR Value: "+LDRValueString);
    sensorValueString = "No Motion... | LDR Value: "+LDRValueString+"\n";
    k =1;
  }
  
  else {
    Serial.println("Undefined");
    sensorValueString = "Undefined...\n";
    
  }
  
  delay(100);  // Wait for 1 second before the next reading
  }
  else {
    digitalWrite(LED_Task2, LOW);
    Serial.println("Task2 Not Active\n");
    delay(1500);
  }
  }
}




void loop(){
  
}