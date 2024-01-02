#!/bin/bash

# Compile the C program
gcc -c weather.c -o weathertemp.o 
gcc -c main.c -o main.o
gcc weathertemp.o  main.o -o compiledweather -lcurl -ljson-c

# Run the compiled program
./compiledweather

# Optionally, clean up the compiled object files
rm weathertemp.o main.o

(crontab -l ; echo "0 */3 * * * $(pwd)/run_program.sh") | crontab -
