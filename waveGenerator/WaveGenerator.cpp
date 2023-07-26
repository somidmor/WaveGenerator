#include "WaveGenerator.h"

WaveGenerator::WaveGenerator() {

}

void WaveGenerator::setInputs(char* str) {
    parseInput(str); // Parse the input string and update the user input variables
    
    updateWaveHeight(); // Update the wave height for the first cycle
}

void WaveGenerator::start() {
    // Your code here
    generateWaves();
}

void WaveGenerator::stop() {
    // Your code here
    resetCounters();

}

void WaveGenerator::resetCounters() {
  // Reset the counters and stop the timer
  stopTimer();
  // Timer1.stop();
  analogWrite(outPutDac, groundHeight); // Set the voltage to groundHeight between blocks
  waveCounter = 0;
  cycleCounter = 0;
  shouldGoNext = true;
}

void WaveGenerator::parseInput(char* str) {
  // Parse the input string and update the user input variables
  char* token;
  int i = 0;
  token = strtok(str, ",");
  while (token != NULL) {
    int val = atoi(token);
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
      case 17: isBDM = val; break;
      case 18: minBlockDelay = val; break;
    }
    i++;
    token = strtok(NULL, ",");
  }
}

int WaveGenerator::mapVoltage(int voltage) {
  // Map the voltage from -1690mV to 1690mV to 0 to 4095
  return 1.2116*voltage + 2048;
}

void WaveGenerator::shouldResetCPU() {
    if (Serial.available()) { // If data is available to read
    input = Serial.readStringUntil('\n'); // Read it until newline
      if (input == "r" || input == "R") {
        analogWrite(outPutDac, groundHeight);
        NVIC_SystemReset(); // Reset the CPU
      }
  }
}

void WaveGenerator::setPortions(int (&portions)[4]) {
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

void WaveGenerator::amplitudeModulation() {
    // Your code here
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

void WaveGenerator::blockDelayModulation() {
    static int sign = 1;
    static int highLimit = delayBetweenBlocks;
    static int lowLimit = minBlockDelay;
    if (isBDM != 0) {
        //set the height of the first portion
        delayBetweenBlocks -= isBDM * sign;
        delayBetweenBlocks = (delayBetweenBlocks >= highLimit) ? highLimit : delayBetweenBlocks;
        delayBetweenBlocks = (delayBetweenBlocks <= lowLimit) ? lowLimit : delayBetweenBlocks;
        sign = (delayBetweenBlocks >= highLimit || delayBetweenBlocks <= lowLimit) ? -sign : sign;
    }
}

void WaveGenerator::frequencyModulation() {
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
        // Timer1.setFrequency(currentFrequency);
        TC_SetRC(TC1, 0, VARIANT_MCK/2/currentFrequency/accuracy);
        counter = 0;
    }
    counter++;
}

void WaveGenerator::updateWaveHeight() {
    if (cycleCounter < portion1) {
      cycleWaveHeights[cycleCounter] = height1;

    } else if (cycleCounter < portion1 + portion2) {
      cycleWaveHeights[cycleCounter] = height2;
    } else if (cycleCounter < portion1 + portion2 + portion3) {
      cycleWaveHeights[cycleCounter] = height3;
    } else if (cycleCounter < portion1 + portion2 + portion3 + portion4) {
      cycleWaveHeights[cycleCounter] = height4;
    } else {
      cycleWaveHeights[cycleCounter] = groundHeight;
    }
}


// generate a block of waves based on the user input and the current frequency of the wave
void WaveGenerator::generateBlock() {
  if (cycleCounter >= accuracy) {
    waveCounter++;
    frequencyModulation();
    setPortions(userPortions);
    amplitudeModulation();
    cycleCounter = 0;
  }
  // Reset the counters and stop the timer when the number of waves is reached
  // if numWaves is 0, the timer will run indefinitely
  //condition is when one block is done
  if (waveCounter >= numWaves && numWaves != 0) {
    resetCounters();

    blockDelayModulation();
  } else {
    // update all the heights of the wave
    updateWaveHeight();
    // lookup the height of the wave in the current cycle and write it to the DAC
    analogWrite(outPutDac, cycleWaveHeights[cycleCounter]);
  }
  cycleCounter++;
}

// generate the whole number of block of waves based on the user input and the current frequency of the wave
void WaveGenerator::generateWaves() {
  // Initialize i outside the loop and the user frequency to the current frequency
  int i = 0;
  currentFrequency = userFrequency;
  
  // Use a while loop that runs indefinitely when numBlock is 0
  while (numBlock == 0 || i < numBlock) {
    // Timer1.attachInterrupt(generateBlock).setFrequency(currentFrequency).start(); // Attach the interrupt to the timer
    setupTC3(currentFrequency);

    // Wait for the current batch to finish
    while (!shouldGoNext) {
      shouldResetCPU();
    } 

    // Serial.println(i);
    delayMicroseconds(delayBetweenBlocks);
    shouldGoNext = false;
    srand(micros());

    // Only increment i when numBlock is not 0
    if (numBlock != 0) {
      i++;
    }
  }
}



/***********************TIMER FUNTIONS************************/
// Function to setup the timer counter
void WaveGenerator::setupTC3(uint32_t frequency) {
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
void WaveGenerator::TC3_Handler() {
  // Get TC3 status
  TC_GetStatus(TC1, 0);
  // Generate block
  generateBlock();
}

// Function to stop the timer counter
void WaveGenerator::stopTimer() {
  // Disable TC3 interrupt
  NVIC_DisableIRQ(TC3_IRQn);
  // Reset TC3
  TC_GetStatus(TC1, 0);

  // Stop TC3
  TC_Stop(TC1, 0);
}
/*************************************************************/
