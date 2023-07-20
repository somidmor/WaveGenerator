#include <DueTimer.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h> 

// User input variables
int frequency, height1, portion1, height2, portion2, height3, portion3, height4, portion4;
int numWaves, delayBetweenBatches, numBatches;
int accuracy = 100; // Accuracy of the duty cycle in percentage
bool shouldGoNext = false; // Flag to indicate if the next batch should be started
String input; // String to store the received input
int waveType = 1; // Variable to store the type of wave to generate

void setup() { 
  Serial.begin(9600); // Begin serial communication at 9600 baud
  analogWriteResolution(12); // Set resolution of the voltage from 0 to 4095
  printMenu(); // Print the menu options
}

void loop() {
  if (Serial.available()) { // If data is available to read
    input = Serial.readStringUntil('\n'); // Read it until newline

    if (input == "r" || input == "R") {
      analogWrite(DAC0, 0);
      NVIC_SystemReset(); // Reset the CPU
    }
    else if (input == "1" || input == "2") {
      waveType = input.toInt(); // Set the wave type
      Serial.println("Enter parameters: ");
    }
    else {
      // Parse the input string
      parseInput(input);

      // Generate the waves
      generateWaves();
    }
  }
}


void generateWaves() {
  for (int i = 0; i < numBatches || numBatches == 0; i++) {
    Timer1.attachInterrupt(timeISR).setFrequency(frequency * 2).start(); // Frequency doubled for square wave

    // Wait for the current batch to finish
    while (!shouldGoNext) {
      shoutlResetCPU();
    } 

    Serial.println(i);
    delay(delayBetweenBatches);
    shouldGoNext = false;
  }
}

void timeISR() {
  static unsigned long cycleCounter = 0;
  static unsigned long waveCounter = 0;
  static double currentHeight = 0;

  if (waveCounter < numWaves) {
    if (waveType == 1) {
      // Normal wave generation
      if (cycleCounter < portion1) {
        currentHeight = height1;
      } else if (cycleCounter < portion1 + portion2) {
        currentHeight = height2;
      } else if (cycleCounter < portion1 + portion2 + portion3) {
        currentHeight = height3;
      } else if (cycleCounter < portion1 + portion2 + portion3 + portion4) {
        currentHeight = height4;
      } else {
        currentHeight = 0;
      }
    } else if (waveType == 2) {
      // Triangle wave generation
      int targetHeight, portionSize;
      if (cycleCounter < portion1) {
        targetHeight = height1;
        portionSize = portion1;
      } else if (cycleCounter < portion1 + portion2) {
        targetHeight = height2;
        portionSize = portion2;
      } else if (cycleCounter < portion1 + portion2 + portion3) {
        targetHeight = height3;
        portionSize = portion3;
      } else if (cycleCounter < portion1 + portion2 + portion3 + portion4) {
        targetHeight = height4;
        portionSize = portion4;
      } else {
        targetHeight = 0;
        portionSize = 1;
      }
      double slope = (targetHeight - currentHeight) / (portionSize * 1.0);

      // Generate the wave
      currentHeight += slope;
    }
    
    // Output the current height
    analogWrite(DAC0, currentHeight);

    if (cycleCounter == accuracy - 1) {
      waveCounter++;
      currentHeight = 0;
    }

    cycleCounter = (cycleCounter + 1) % accuracy;
  }
  else {
    // Reset the counters and stop the timer
    analogWrite(DAC0, 0);
    waveCounter = 0;
    cycleCounter = 0;
    shouldGoNext = true;
    Timer1.stop();
  }
}

void parseInput(String str) {
  // Parse the input string and update the user input variables
  int commaIndex = 0, lastCommaIndex = -1;
  for (int i = 0; i < 11; i++) {
    commaIndex = str.indexOf(',', lastCommaIndex + 1);

    String substr;
    if (commaIndex == -1) {
      substr = str.substring(lastCommaIndex + 1);
    } else {
      substr = str.substring(lastCommaIndex + 1, commaIndex);
    }

    int val = substr.toInt();

    switch (i) {
      case 0: frequency = val * accuracy / 2; break;
      case 1: height1 = mapVoltage(val); break;
      case 2: portion1 = val; break;
      case 3: height2 = mapVoltage(val); break;
      case 4: portion2 = val; break;
      case 5: height3 = mapVoltage(val); break;
      case 6: portion3 = val; break;
      case 7: height4 = mapVoltage(val); break;
      case 8: portion4 = val; break;
      case 9: numWaves = val; break;
      case 10: delayBetweenBatches = val; break;
      case 11: numBatches = val; break;
    }

    lastCommaIndex = commaIndex;
  }
}

int mapVoltage(int voltage) {
  // Map the voltage from -1690mV to 1690mV to 0 to 4095
  return 1.2116*voltage + 2048;
}

void printMenu() {
  // Print the menu options
  Serial.println("Select wave type:");
  Serial.println("1 - Normal");
  Serial.println("2 - Triangle");
}

void shoutlResetCPU(){
    if (Serial.available()) { // If data is available to read
    input = Serial.readStringUntil('\n'); // Read it until newline

    if (input == "r" || input == "R") {
      analogWrite(DAC0, 0);
      NVIC_SystemReset(); // Reset the CPU
    }
  }
}


