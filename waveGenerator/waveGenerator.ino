#include <stdlib.h>


// user input format:
// userFrequency: 1-13KHz, it is the userFrequency of the wave
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
    GENERATE_NOT_FM_WAVES,
    GENERATE_FM_WAVES,
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
const int microseceondsInSecond = 1000000;

// Accuracy of the duty cycle in percentage
const int accuracy = 10;

// User input variables
const int numberOfParams = 17; //number of user parameter
int userFrequency,groundHeight, height1, portion1, height2, portion2, height3, portion3, height4, portion4;
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
  analogWrite(DAC0, groundHeight); // Set the voltage to groundHeight between blocks
  waveCounter = 0;
  cycleCounter = 0;
  shouldGoNext = true;
  stop();
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
      case 0: userFrequency = val; break;
      case 1: groundHeight = mapVoltage(val); break;
      case 2: userHeights[0] = mapVoltage(val); break;
      case 3: userPortions[0] = val; break;
      case 4: userHeights[1] = mapVoltage(val); break;
      case 5: userPortions[1] = val; break;
      case 6: userHeights[2] = mapVoltage(val); break;
      case 7: userPortions[2] = val; break;
      case 8: userHeights[3] = mapVoltage(val); break;
      case 9: userPortions[3] = val; break;
      case 10: numWaves = val; break;
      case 11: delayBetweenBlocks = val; break;
      case 12: numBlock = val; break;
      case 13: isAM = val; break;
      case 14: AMScale = val; break;
      case 15: isFM = val; break;
      case 16: maxFrequency = val; break;
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
        analogWrite(DAC0, groundHeight);
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
        result[i] = rand() % randomSum + 1;
        randomSum -= result[i];  // Subtract the value assigned to result[i]
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
    static int lowLimit = height2;
    if (isAM != 0) {
        //set the height of the first portion
        height1 += isAM * sign;
        height1 = (height1 >= highLimit) ? highLimit : height1;
        height1 = (height1 <= lowLimit) ? lowLimit : height1;
        sign = (height1 >= highLimit || height1 <= lowLimit) ? -sign : sign;
        // if the user wants to generate amplitude modulation with scale
        // calculate the height of the third portion
        height3 = lowLimit-(lowLimit/AMScale) + ((height1)/AMScale);
    }
}

// Frequency modulation function
void frequencyModulation() {
    static int sign = 1;
    static int highLimit = maxFrequency;
    static int lowLimit = userFrequency;
    static int counter = 0;
    if (isFM != 0 && counter == 10) {
        //set the height of the first portion
        currentFrequency -= isFM * sign;
        currentFrequency = (currentFrequency >= highLimit) ? highLimit : currentFrequency;
        currentFrequency = (currentFrequency <= lowLimit) ? lowLimit : currentFrequency;
        sign = (currentFrequency >= highLimit || currentFrequency <= lowLimit) ? -sign : sign;
        TC_SetRC(TC1, 0, VARIANT_MCK/2/currentFrequency/accuracy);
        counter = 0;
    }
    counter++;
}

// This function updates the currentHeight based on the current cycleCounter.
void updateWaveHeight() {
    if (cycleCounter < portion1) {
      cycleWaveHeights[cycleCounter] = height1;

    } else if (cycleCounter < portion1 + portion2) {
      cycleWaveHeights[cycleCounter] = height2;
    } else if (cycleCounter < portion1 + portion2 + portion3) {
      cycleWaveHeights[cycleCounter] = height3;
    } else if (cycleCounter < portion1 + portion2 + portion3 + portion4) {
      cycleWaveHeights[cycleCounter] = height4;
    } else {
      cycleWaveHeights[cycleCounter] = 0;
    }
}


// This function generates a block of the wave
void generateBlock() {
  if (cycleCounter >= accuracy) {
    waveCounter++;
    frequencyModulation();
    setPortions(userPortions);
    amplitudeModulation();
    cycleCounter = 0;
  }
  // update all the heights of the wave

  // lookup the height of the wave in the current cycle and write it to the DAC
  analogWrite(DAC0, cycleWaveHeights[cycleCounter]);

  
  cycleCounter++;
  updateWaveHeight();
  

  // Reset the counters and stop the timer when the number of waves is reached
  // if numWaves is 0, the timer will run indefinitely
  if (waveCounter >= numWaves && numWaves != 0) {
    resetCounters();
  }
}


// This function generates the waves
void generateWaves() {
  
  // Initialize i outside the loop and the user frequency to the current frequency
  int i = 0;
  currentFrequency = userFrequency;
  
  // Use a while loop that runs indefinitely when numBlock is 0
  while (numBlock == 0 || i < numBlock) {
    setupTC3(currentFrequency);

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
      if(isFM == 0){
        gState.next = States::GENERATE_NOT_FM_WAVES;
      } else {
        gState.next = States::GENERATE_FM_WAVES;
      }
      break;

    case GENERATE_NOT_FM_WAVES:
      generateWaves();
      gState.next = States::GET_USER_INPUT;
      break;

    case GENERATE_FM_WAVES:
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



/***********************TIMER FUNTIONS************************/
// Function to setup the timer counter
void setupTC3(uint32_t frequency) {
  // Disable TC3 interrupt
  NVIC_DisableIRQ(TC3_IRQn);
  // Reset TC3
  TC_GetStatus(TC1, 0);
  // Enable TC3 clock
  pmc_enable_periph_clk(TC3_IRQn);
  // Set up TC3
  TC_Configure(TC1, 0, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK1);
  // Set RC value for TC3 to match the desired frequency
  TC_SetRC(TC1, 0, VARIANT_MCK/2/frequency/accuracy);
  // Enable TC3 interrupt on RC compare
  TC1->TC_CHANNEL[0].TC_IER=TC_IER_CPCS;
  TC1->TC_CHANNEL[0].TC_IDR=~TC_IER_CPCS;
  // Enable TC3 interrupt
  NVIC_EnableIRQ(TC3_IRQn);
  // Start TC3
  TC_Start(TC1, 0);
}

// TC3 interrupt service routine
void TC3_Handler() {
  // Get TC3 status
  TC_GetStatus(TC1, 0);
  // Generate block
  generateBlock();
}

// Function to stop the timer counter
void stop() {
  // Disable TC3 interrupt
  NVIC_DisableIRQ(TC3_IRQn);
  // Reset TC3
  TC_GetStatus(TC1, 0);
  // Stop TC3
  TC_Stop(TC1, 0);
}