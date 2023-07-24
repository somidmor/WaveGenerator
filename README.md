# Arduino Waveform Generator

This Arduino code generates a customized waveform based on user inputs. It utilizes the DueTimer library and has been developed for the Arduino Due board. The wave is generated using the DAC0 pin. 

## Features
* Custom frequency, heights, and portions of waveforms.
* Capability to generate an indefinite number of waves and blocks of waves.
* Provides a way to insert a delay between blocks of waves.
* Ability to add Amplitude Modulation to the waveform.
* Offers a command to reset the CPU.
* Possibility to randomly generate the proportions of the wave if not provided by the user.

## User Input Format
The user input should be in the following format:

```
frequency, height1, portion1, height2, portion2, height3, portion3, height4, portion4, numWaves, delayBetweenBlocks, numBlock, isAM, AMScale
```

* **frequency**: Frequency of the wave, in the range 1 - 15000 Hz.
* **heightX**: The height of the wave for the Xth portion, ranging from -1690 to 1690. X can be from 1 to 4.
* **portionX**: The portion of the wave for the Xth portion, in the range 0 - 10. The sum of all the portions should be equal to 10. X can be from 1 to 4.
* **numWaves**: The number of waves to generate, in the range 0 - 1000. If it is 0, it will generate waves indefinitely.
* **delayBetweenBlocks**: The delay between each block of waves, in the range 0 - 1000 ms.
* **numBlock**: The number of blocks of waves to generate, in the range 0 - 1000. If it is 0, it will generate blocks indefinitely.
* **isAM**: The amplitude modulation of the wave, in the range 0 - 1690. If it is 0, it will not generate amplitude modulation. If greater than 0 it will generate amplitude modulation, the value is the amplitude of the modulation.
* **AMScale**: The amplitude scale for modulation. 

Note: Portions and heights are for different parts of a wave cycle. You can split a wave cycle into 4 parts and provide heights for each part.

## How to Use
* Upload the script to the Arduino Due board.
* Open the Serial Monitor.
* Input the parameters according to the User Input Format mentioned above and press 'Enter'. The Arduino will start generating the waveform.
* If you want to reset the CPU, type 'r' or 'R' and press 'Enter'.
* You can input new parameters to generate a new waveform.

## Requirements
* [Arduino Due](https://store.arduino.cc/usa/due.html) or any board supporting analogWriteResolution and DueTimer library.
* [DueTimer library](https://github.com/ivanseidel/DueTimer)
  
## Limitations
* The actual output waveform will depend on the capabilities of your hardware, including the DAC's resolution and maximum speed.
* Amplitude Modulation only works on the first portion of the wave.
* The sum of the user portions should be equal to 10.
* The portions are integers, so the wave can only be split into parts that are multiples of 10% of the wave.

For any other information or clarifications, feel free to open an issue in this repository.