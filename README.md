Project code is divided into libraries and Arduino specific projects which only run on specific boards.
Libraries are typically generic and can be run by most Arduino boards unless the library is specific to a particular chip. 
For example, the DS3231 libraries are specific to communicating to only the DS3231 chips and nothing else.

You will need to sync the libraries folder and the board specific .ino file in order to compile any Arduino project in this reporsitory. 
