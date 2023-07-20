String profile;
long frequency; // Variable to store the frequency
long voltageTrans; // Variable to store the voltage level
int dutyCycle; // Variable to store the duty cycle
int numWaves; // Variable to store the number of wave cycles in each batch
int numBatches; // Variable to store the number of batches
int delayBetweenBatches; // Time delay between batches in ms
int resolution = 12; // resolution of the voltage from 0 to 4095
bool shouldGoNext = false; // Flag to indicate if the next batch should be started
int timeSinceLastBatch = 0; // Variable to store the time since the last batch started
int dcAccuracy = 10;
String input; // String to store the received input

#include <DueTimer.h>

void parseInput(String str){
    int commaIndex1 = str.indexOf(',');
    profile = str.substring(0, commaIndex1);

    int commaIndex2 = str.indexOf(',', commaIndex1 + 1);
    frequency = str.substring(commaIndex1 + 1, commaIndex2).toInt();
    frequency *= 5;

    int commaIndex3 = str.indexOf(',', commaIndex2 + 1);
    voltageTrans = str.substring(commaIndex2 + 1, commaIndex3).toInt();
    voltageTrans  = voltageTrans * 1.861 - 1861; //mapping the voltage from 0 to 3200 to 0 to 4095

    int commaIndex4 = str.indexOf(',', commaIndex3 + 1);
    dutyCycle = str.substring(commaIndex3 + 1, commaIndex4).toInt();

    int commaIndex5 = str.indexOf(',', commaIndex4 + 1);
    numWaves = str.substring(commaIndex4 + 1, commaIndex5).toInt();

    int commaIndex6 = str.indexOf(',', commaIndex5 + 1);
    delayBetweenBatches = str.substring(commaIndex5 + 1, commaIndex6).toInt();

    numBatches = str.substring(commaIndex6 + 1).toInt();
}

void setup() { 
  Serial.begin(9600); // Begin serial communication at 9600 baud
  analogWriteResolution(resolution);
}

void loop() {
  if (Serial.available()) { // If data is available to read
    input = Serial.readStringUntil('\n'); // read it until newline

    // parse the input string
    parseInput(input);

    for (int i = 0; i < numBatches; i++){
        /* code */
        Timer1.attachInterrupt(timerIsr).setFrequency(frequency * 2).start(); // Frequency doubled for square wave
        while (!shouldGoNext) {
          Serial.print(""); //DO NOT remove this line,otherwise it won't work. NO IDEA
        } 
        Serial.println(i);
        delay(delayBetweenBatches);
        shouldGoNext = false;
    }
    }
  }

void timerIsr() {
    static unsigned long dutyCycleCounter = 0;
    static unsigned long waveCounter = 0;

    if (waveCounter < numWaves) {
        if (dutyCycleCounter< (dcAccuracy-dutyCycle)) {
            analogWrite(DAC0, 0);
        } else {
            analogWrite(DAC0, voltageTrans);
        }
        if (dutyCycleCounter == dcAccuracy-1) {
            waveCounter++;
        }
        dutyCycleCounter = (dutyCycleCounter + 1) % dcAccuracy;
    }
    else {
        analogWrite(DAC0, 0);
        waveCounter = 0;
        dutyCycleCounter = 0;
        shouldGoNext = true;
        Timer1.stop();
    }
}
