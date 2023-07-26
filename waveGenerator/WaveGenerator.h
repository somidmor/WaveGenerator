#ifndef WaveGenerator_h
#define WaveGenerator_h
#define MICROSECONDS_IN_SECOND 1000000 // 1 second = 1000000 microseconds
#define HEIGHT_LIMIT 1690 // 1690 is the maximum height of the wave in milivolts
#define ACCURACY 10 // 10 is the number of points per wave
#define NUMBER_OF_PARAMS 19 // 19 is the number of parameters for generating a wave provided by the user

#include "Arduino.h"
class WaveGenerator
{

private:

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
        if greater than 0 it will generate amplitude modulation, the value is the amplitude of the modulation each steps */
    /*AMSclae: it is the scale of the amplitude modulation, if it is 0, it will not generate amplitude modulation
        if not 0 it will generate amplitude modulation, the value is the scale of the modulation 
        it syncs the first height with the third height so always height1/height3 = AMScale even negative*/
    /* isFM: 0-1690, it is the frequency modulation of the wave, if it is 0, it will not generate frequency modulation
        if greater than 0 it will generate frequency modulation, the value is the frequency of the modulation each steps */
    /* maxFrequency: 0-13KHz, it is the maximum frequency of the wave during frequency modulation */
    /* isBDM(Block Delay Modulation): 0-1690, it is the block delay modulation of the wave, if it is 0, it will not generate block delay modulation
        if greater than 0 it will generate block delay modulation, the value is the delay of the modulation each steps */
    /* minBlockDelay: it is the minimum delay between each block of waves */
    unsigned long cycleCounter = 0; // counts the number of cycles
    unsigned long waveCounter = 0; // counts the number of waves in a block
    // User input variables
    int userFrequency,groundHeight, height1, portion1, height2, portion2, height3, portion3, height4, portion4;
    int numWaves, delayBetweenBlocks, numBlock, isAM, AMScale, isFM, maxFrequency, isBDM, minBlockDelay;

    // current frequency of the wave
    int currentFrequency;

    // Array to store the heights of the wave
    int cycleWaveHeights[accuracy];

    // Flag to indicate if the next batch should be started
    bool shouldGoNext = false;

    // String to store the received input
    char* input;

    // Store the values of the users portions
    int userPortions[4] = {0, 0, 0, 0};

    // Store the values of the users heights
    int userHeights[4] = {0, 0, 0, 0};

    //private methods
    void resetCounters();
    // This function is used to parse the user input
    void parseInput(char* str);
    // This function is used to map the voltage from range of 0 to 1690 to 0 to 4095
    int mapVoltage(int voltage);
    // This function is used to reset the CPU if the user enters 'r' or 'R'
    void shouldResetCPU();
    // This function is used to set the portions to random values
    void setPortions(int (&portions)[4]);
    // this function is used to generate the amplitude modulation
    void amplitudeModulation();
    // Block delay modulation function
    void blockDelayModulation();
    // Frequency modulation function
    void frequencyModulation();
    // This function updates the currentHeight based on the current cycleCounter.
    void updateWaveHeight();
    // This function generates a block of waves based on the user input and the current frequency of the wave
    void generateBlock();
    // This function generates a block of waves based on the user input and the current frequency of the wave
    void generateWave();



    /*Time functions*/
    void setupTC3(uint32_t frequency);
    void TC3_Handler();
    void stopTimer();
    /****************/



public:
    WaveGenerator(/* args */);
    void setInputs(char* str);
    void start();
    void stop();
};

#endif
