#include <DueTimer.h>
#include <stdlib.h>

enum States {
    INIT,
    GET_USER_INPUT,
    PARSE_USER_INPUT,
    GENERATE_WAVES,
    USER_INPUT_ERROR_STATE,
};

struct stateStatus {
    States next;
    States current;
    States previous;
};

stateStatus gState = {
    .next = States::INIT,
    .current = States::INIT,
    .previous = States::INIT
};

// User input variables
int frequency, height1, portion1, height2, portion2, height3, portion3, height4, portion4;
int numWaves, delayBetweenBatches, numBatches;
int accuracy = 10; // Accuracy of the duty cycle in percentage
bool shouldGoNext = false; // Flag to indicate if the next batch should be started
String input; // String to store the received input
int waveType = 1; // Variable to store the type of wave to generate

void setup() { 
  Serial.begin(9600); // Begin serial communication at 9600 baud
  analogWriteResolution(12); // Set resolution of the voltage from 0 to 4095
  gState.current = States::INIT; // Initialize the first state
}

void loop() {
  switch(gState.current) {
    case INIT:
      Serial.println("Enter parameters: ");
      gState.next = States::GET_USER_INPUT;
      break;

    case GET_USER_INPUT:
      if (Serial.available()) {
        input = Serial.readStringUntil('\n');
        if (input == "r" || input == "R") {
          analogWrite(DAC0, 0);
          NVIC_SystemReset();
        } else {
          gState.next = States::PARSE_USER_INPUT;
        }
      }
      break;

    case PARSE_USER_INPUT:
      parseInput(input);
      gState.next = States::GENERATE_WAVES;
      break;

    case GENERATE_WAVES:
      generateWaves();
      gState.next = States::GET_USER_INPUT;
      break;

    case USER_INPUT_ERROR_STATE:
      // for later
      break;
  }

  gState.previous = gState.current;
  gState.current = gState.next;
}


void generateWaves() {
  for (int i = 0; i < numBatches || numBatches == 0; i++) {
    Timer1.attachInterrupt(timeISR).setFrequency(frequency * 2).start(); // Frequency doubled for square wave

    // Wait for the current batch to finish
    while (!shouldGoNext) {
      shouldResetCPU();
    } 

    // Serial.println(i);
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
  int portions[4] = {0, 0, 0, 0}; // Store the values of the portions temporarily
  int portionCount = 0; // Counter to keep track of how many portions have been assigned values

  for (int i = 0; i < 12; i++) {
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
      case 2: portions[0] = val; break;
      case 3: height2 = mapVoltage(val); break;
      case 4: portions[1] = val; break;
      case 5: height3 = mapVoltage(val); break;
      case 6: portions[2] = val; break;
      case 7: height4 = mapVoltage(val); break;
      case 8: portions[3] = val; break;
      case 9: numWaves = val; break;
      case 10: delayBetweenBatches = val; break;
      case 11: numBatches = val; break;
    }

    lastCommaIndex = commaIndex;
  }

  // Calculate the sum of the portions that have been set by the user
  int total = 0;
  for(int i = 0; i < 4; i++) {
    if(portions[i] != -1) {
      total += portions[i];
      portionCount++;
    }
  }


  // Calculate the sum to be divided among the random portions
  int randomSum = 10 - total;

  srand (micros());
  // Assign random values to the portions that were set to -1
  for(int i = 0; i < 4; i++) {
    if(portions[i] == -1) {
      // If there's only one portion left to assign, give it the remaining sum
      if(4 - portionCount == 1) {
        portions[i] = randomSum;
      } else {
        // Otherwise, assign a random value between 1 and the remaining sum
        portions[i] = rand() % randomSum;
        randomSum -= portions[i];
      }
      portionCount++;
    }
  }

  // Assign the portions to the corresponding variables
  portion1 = portions[0];
  portion2 = portions[1];
  portion3 = portions[2];
  portion4 = portions[3];
  Serial.println(portions[0]);
  Serial.println(portions[1]);
  Serial.println(portions[2]);
  Serial.println(portions[3]);
}



int mapVoltage(int voltage) {
  // Map the voltage from -1690mV to 1690mV to 0 to 4095
  return 1.2116*voltage + 2048;
}

void shouldResetCPU(){
    if (Serial.available()) { // If data is available to read
    input = Serial.readStringUntil('\n'); // Read it until newline

    if (input == "r" || input == "R") {
      analogWrite(DAC0, 0);
      NVIC_SystemReset(); // Reset the CPU
    }
  }
}
