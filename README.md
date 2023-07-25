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

# Random Mode

This feature allows the user to set portions of the wave to randomly generated values. If the user specifies `-1` for any of the portion values (portion1, portion2, portion3, or portion4), the program will automatically generate a random number for that portion.

This value is computed such that the sum of all portions always equals 10. For instance, if the user sets portion1 to 3, portion2 to `-1`, portion3 to 4, and portion4 to `-1`, the program will generate random numbers for portion2 and portion4 such that their sum, along with portion1 and portion3, equals 10.

To ensure the sum is correct, the program first calculates the sum of the portions set by the user and then distributes the remaining sum among the portions set to `-1`.

To note, the random values are always integers between 1 and the remaining sum, ensuring a fair distribution of the remaining sum. 

This feature allows the user to introduce some randomness into their waveforms, providing a more versatile waveform generation experience.


# Infinity Mode

Infinity Mode allows the user to generate waves or blocks of waves indefinitely until the user manually stops the process. This mode is activated when the user sets `numWaves` or `numBlock` to `0`.

- `numWaves`: Specifies the number of waves to generate in a block. When `numWaves` is set to `0`, the program will generate waves indefinitely until manually stopped.

- `numBlock`: Specifies the number of blocks of waves to generate. When `numBlock` is set to `0`, the program will generate blocks of waves indefinitely until manually stopped.

In both cases, the program will continue to generate waves or blocks of waves indefinitely, maintaining the characteristics (height, frequency, etc.) specified by the user. This mode is useful for applications where continuous signal generation is required.

To stop the waveform generation in Infinity Mode, the user can reset the CPU by sending 'r' or 'R' to the input. This will stop the current process and reset all the parameters.

## Requirements
* [Arduino Due](https://store.arduino.cc/usa/due.html) or any board supporting analogWriteResolution and DueTimer library.
* [DueTimer library](https://github.com/ivanseidel/DueTimer)
  
## Limitations
* The actual output waveform will depend on the capabilities of your hardware, including the DAC's resolution and maximum speed.
* Amplitude Modulation only works on the first portion of the wave.
* The sum of the user portions should be equal to 10.
* The portions are integers, so the wave can only be split into parts that are multiples of 10% of the wave.

For any other information or clarifications, feel free to open an issue in this repository.