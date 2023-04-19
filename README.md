# Drink Mixing Machine Controller

## Clone Repository

After cloning the repository it is necessary to it is necessary to import the required submodules to build the targets.
This can be accomplished by running:

```bash
git submodule update --init --recursive
```

Despite this the required tools are:

* gcc
* arm-none-eabi-gcc
* CMake
* Ninja/Make

## Contribution Guidelines

The source code should be formatted according to the in the [.clang-format](.clang-format) file specified style.

### Folder Structure

The source code for the target device can be found under [src](src), whereas the integration test are implemented under [integration](test/integration).
The unit test aim to be executed on the local machine/server are implemented under [unit](test/unit).
The [extern](extern) folder contains all external dependencies like git submodules.

## Troubleshooting

Try with Arduino mega with example code from TMC2209 library each stepper driver

If not working with Arduino, test after taking each of those steps:
    - Check each connection
    - Make sure tx and rx are correct
    - Use different stepper driver
    - Use different cable to computer
    - Replace resistor
    - Try on different computer
    
If it works with the arduino, connections are right, try with pico again, but with commit 208c23bc8ce1775b5c6087e88812553b228f1036
If it works on that commit but not currently the code is probably wrong.
If it does not work on that commit try:
    - Replacing the pico
    - Use different cable to computer
    - Use different computer
