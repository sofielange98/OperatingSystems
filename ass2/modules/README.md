Programming Assignment 2
Operating Systems
Sofia Lange
February 25th, 2019

Included in this submission are 3 files excluding this one.
These files contain a LKM which simulates a simple character device driver.

~~~~~~ Makefile ~~~~~~
This is the make file for the project, it will tell the compiler which object we would like to compile.
When you type "
make -C /lib/modules/$(uname -r) M=$PWD modules
"
it will know you want to compile a new LKM object specified as simple_character_driver.o

~~~~~~ simple_character_driver.c ~~~~~~
This contains the actual device driver code. This is a LKM which simulates communication with a device.
The device should be specified as major number 300 and minor number 0.
The module contains my implementation of the file operations functions:
llseek
read
write
open
close
as well as an init and exit function for when the module is loaded and unloaded.

~~~~~~ testprog.c ~~~~~~
This is my test program for interacting with my device driver module. It enables the user to read from and write to as well as seek within a file which is specified as being connected to the device with major number 300 and minor number 0.

To install and use the module and test program, you must have these three files, then you must make and insert the module. Then, you have to make the device file using the command "
sudo mknod –m 777 /dev/simple_char_device c 300 0
"
finally, you can compile and run the test program to read and write and seek in the device file. 
