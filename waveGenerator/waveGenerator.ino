#include <DueTimer.h>
#include <stdlib.h>


// user input format:
// userFrequency: 1-15000, it is the userFrequency of the wave
// height1: -1690-1690, it is the height of the wave for the first portion
// portion1: 0-10, it is the portion of the wave for the first portion
// height2: -1690-1690, it is the height of the wave for the second portion
// portion2: 0-10, it is the portion of the wave for the second portion
// height3: -1690-1690, it is the height of the wave for the third portion
// portion3: 0-10, it is the portion of the wave for the third portion
// height4: -1690-1690, it is the height of the wave for the fourth portion
// portion4: 0-10, it is the portion of the wave for the fourth portion
// numWaves: 0-1000, it is the number of waves to generate, if it is 0, it will generate waves indefinitely
// delayBetweenBlocks: 0-1000, it is the delay between each block of waves
// numBlock: 0-1000, it is the number of blocks of waves to generate, if it is 0, it will generate blocks indefinitely
/* isAM: 0-1690, it is the amplitude modulation of the wave, if it is 0, it will not generate amplitude modulation
    if greater than 0 it will generate amplitude modulation, the value is the amplitude of the modulation */




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



static unsigned long cycleCounter = 0;
static unsigned long waveCounter = 0;
static double currentHeight = 0;
const int heightLimit = 1690;

// Accuracy of the duty cycle in percentage
const int accuracy = 10;

// User input variables
const int numberOfParams = 16; //number of user parameter
int userFrequency, height1, portion1, height2, portion2, height3, portion3, height4, portion4;
int numWaves, delayBetweenBlocks, numBlock, isAM, AMScale, isFM, maxFrequency;

// current frequency of the wave
int currentFrequency;

// Array to store the heights of the wave
int cycleWaveHeights[accuracy];

// Flag to indicate if the next batch should be started
bool shouldGoNext = false;

// String to store the received input
String input;

// Variable to store the type of wave to generate
int waveType = 1;

// Store the values of the users portions
int userPortions[4] = {0, 0, 0, 0};

// Store the values of the users heights
int userHeights[4] = {0, 0, 0, 0};


void resetCounters() {
  // Reset the counters and stop the timer
  analogWrite(DAC0, 0); // Set the voltage to 0 between blocks
  waveCounter = 0;
  cycleCounter = 0;
  shouldGoNext = true;
  Timer1.stop();
}



void parseInput(String str) {
  // Parse the input string and update the user input variables
  int commaIndex = 0, lastCommaIndex = -1;
  int portionCount = 0; // Counter to keep track of how many portions have been assigned values

  for (int i = 0; i < numberOfParams; i++) {
    commaIndex = str.indexOf(',', lastCommaIndex + 1);

    String substr;
    if (commaIndex == -1) {
      substr = str.substring(lastCommaIndex + 1);
    } else {
      substr = str.substring(lastCommaIndex + 1, commaIndex);
    }

    int val = substr.toInt();

    switch (i) {
      case 0: userFrequency = val * accuracy / 2; break;
      case 1: userHeights[0] = mapVoltage(val); break;
      case 2: userPortions[0] = val; break;
      case 3: userHeights[1] = mapVoltage(val); break;
      case 4: userPortions[1] = val; break;
      case 5: userHeights[2] = mapVoltage(val); break;
      case 6: userPortions[2] = val; break;
      case 7: userHeights[3] = mapVoltage(val); break;
      case 8: userPortions[3] = val; break;
      case 9: numWaves = val; break;
      case 10: delayBetweenBlocks = val; break;
      case 11: numBlock = val; break;
      case 12: isAM = val; break;
      case 13: AMScale = val; break;
      case 14: isFM = val; break;
      case 15: maxFrequency = val * accuracy / 2; break;
    }

    lastCommaIndex = commaIndex;
  }

  // Assign the portions to the corresponding variables
  portion1 = userPortions[0];
  portion2 = userPortions[1];
  portion3 = userPortions[2];
  portion4 = userPortions[3];
  height1 = userHeights[0];
  height2 = userHeights[1];
  height3 = userHeights[2];
  height4 = userHeights[3];
  currentFrequency = userFrequency;

}

int mapVoltage(int voltage) {
  // Map the voltage from -1690mV to 1690mV to 0 to 4095
  return 1.2116*voltage + 2048;
}

// This function is used to reset the CPU if the user enters 'r' or 'R'
void shouldResetCPU(){
    if (Serial.available()) { // If data is available to read
    input = Serial.readStringUntil('\n'); // Read it until newline
      if (input == "r" || input == "R") {
        analogWrite(DAC0, 0);
        NVIC_SystemReset(); // Reset the CPU
      }
  }
}

// This function is used to set the portions to random values
void setPortions(int (&portions)[4]){
  int portionCount = 0;
  int result[4] = {0, 0, 0, 0}; // Store the values of the portions temporarily
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

  
  // Assign random values to the portions that were set to -1
  for(int i = 0; i < 4; i++) {
    if(portions[i] == -1) {
      // If there's only one portion left to assign, give it the remaining sum
      if(4 - portionCount == 1) {
        result[i] = randomSum;
      } else {
        // Otherwise, assign a random value between 1 and the remaining sum
        result[i] = rand() % randomSum;
        randomSum -= portions[i];
      }
      portionCount++;
    } else {
      result[i] = portions[i];
    }
  }
  
  // Assign the portions to the corresponding variables
  portion1 = result[0];
  portion2 = result[1];
  portion3 = result[2];
  portion4 = result[3];
}

// this function is used to generate the amplitude modulation
void amplitudeModulation() {
    static int sign = 1;
    static int highLimit = 4095;
    static int lowLimit = (AMScale == 0) ? 0 : 2048;

    if (isAM > 0) {
        height1 += isAM * sign;
        height1 = (height1 >= highLimit) ? highLimit : height1;
        height1 = (height1 <= lowLimit) ? lowLimit : height1;
        sign = (height1 >= highLimit || height1 <= lowLimit) ? -sign : sign;
        //set the fist portion height of the wave in the next cycle
        for (int i = 0; i < portion1; i++){
            cycleWaveHeights[i] = height1;
            if (AMScale != 0) { // if the user wants to generate amplitude modulation with scale
            cycleWaveHeights[i + portion1] = lowLimit + ((height1)/AMScale);
            }
        }
    }
}

// Frequency modulation function
void frequencyModulation() {
  static int sign = 1;
  if (isFM > 0) {
      currentFrequency += isFM * sign;
      currentFrequency = (currentFrequency >= maxFrequency) ? maxFrequency : currentFrequency;
      currentFrequency = (currentFrequency <= userFrequency) ? userFrequency : currentFrequency;
      sign = (currentFrequency >= maxFrequency || currentFrequency <= userFrequency) ? -sign : sign;
      Timer1.attachInterrupt(generateBlock).setFrequency(currentFrequency*2).start(); // Update the frequency
  }
}

// This function generates a block of the wave
void generateBlock() {
  if (cycleCounter >= accuracy) {
    waveCounter++;
    frequencyModulation();
    amplitudeModulation();
    cycleCounter = 0;
  }

  // Lookup the height from the precalculated array
  analogWrite(DAC0, cycleWaveHeights[cycleCounter]);

  cycleCounter++;

  // Reset the counters and stop the timer when the number of waves is reached
  // if numWaves is 0, the timer will run indefinitely
  if (waveCounter >= numWaves && numWaves != 0) {
    resetCounters();
  }
}


// This function updates the currentHeight based on the current cycleCounter.
void updateWaveHeight() {
  for (int i = 0; i < accuracy; i++) {
    if (i < portion1) {
      cycleWaveHeights[i] = height1;

    } else if (i < portion1 + portion2) {
      cycleWaveHeights[i] = height2;
    } else if (i < portion1 + portion2 + portion3) {
      cycleWaveHeights[i] = height3;
    } else if (i < portion1 + portion2 + portion3 + portion4) {
      cycleWaveHeights[i] = height4;
    } else {
      cycleWaveHeights[i] = 0;
    }
  }
  
}

void generateWaves() {

  updateWaveHeight();
  
  // Initialize i outside the loop and the user frequency to the current frequency
  int i = 0;

  // Use a while loop that runs indefinitely when numBlock is 0
  while (numBlock == 0 || i < numBlock) {
    Timer1.attachInterrupt(generateBlock).setFrequency(currentFrequency*2).start(); // Frequency doubled for square wave

    // Wait for the current batch to finish
    while (!shouldGoNext) {
      shouldResetCPU();
    } 

    // Serial.println(i);
    delay(delayBetweenBlocks);
    shouldGoNext = false;
    srand(micros());

    // Only increment i when numBlock is not 0
    if (numBlock != 0) {
      i++;
    }
  }
}

 
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
