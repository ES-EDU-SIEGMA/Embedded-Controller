# Drink Mixing Machine Controller

This repository provides the source code for the RP2040 based stepper driver controller.
Further documentation of the hardware can be found in the [main](https://git.uni-due.de/embedded-systems/student-projects/ss23-drink-mixing-machine/main) repository.

**The main branch of this repository can be considered stable and should be working all time.**

## Clone Repository

### Submodules

After cloning the repository it is necessary to import the required submodules to build all targets.

> Required Submodules:
>
> - `pico-sdk` for RP2040 hardware support
> - `unity` for Unit-Tests

This can be accomplished by running:

```bash
git submodule update --init --recursive
```

inside your console.

### Toolchain

Despite the submodules the required tools for the toolchain are required:

- gcc (C-Compiler)
- arm-none-eabi-gcc (ARM C-Compiler)
- CMake (Build Script Generator)
- Ninja/Make (Build Tool)

## Contribution Guidelines

The source code should be formatted according to the style specified in the [.clang-format](.clang-format) file.

There are no direct commits to the main branch allowed.
If you want to contribute source code feel free to open a merge request with your desired changes.

### Folder Structure

The source code for the target device can be found under [src](src).
The [integration](test/integration) folder provides you with the integration tests which also double as an example for the usage of the provided libraries.
The unit test aim to be executed on the local machine/server and are implemented under [unit](test/unit).
The [extern](extern) folder contains all external dependencies like git submodules.

## Troubleshooting

### Serial Communication (Debugging)

The application detects the end of a send command by searching for _EOL_ (0x0A) or `\n`. 
This character is not on the standard keyboard!
In minicom for example it can be send with `CTRL+J`.


### Stepper Driver not working

Try with Arduino mega with example code from TMC2209 library each stepper driver.
If the stepper driver are still not working, test after taking each of those steps:

- Check each connection
- Make sure TX and RX are correctly wired
- Use multiple different stepper driver
- Use different USB cable to connect your computer
- Replace resistors
- Try with a different computer

If it works with the Arduino, the connections are all correct.
Try again with the pico.
If it does not work:

- Replace the pico
- Use different cable to connect your computer
- Use a different computer
