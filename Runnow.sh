#!/bin/bash

# Compile the C program
gcc -c fetch_data.c -o fetchtemp.o
gcc -c visibilty_checks.c -o visibiltytemp.o 
gcc -c main.c -o maintemp.o
gcc fetchtemp.o  visibiltytemp.o maintemp.o  -o compiledweather -lcurl -ljson-c

# Run the compiled program
./compiledweather

# Optionally, clean up the compiled object files
rm fetchtemp.o visibiltytemp.o maintemp.o

(crontab -l ; echo "0 */3 * * * $(pwd)/Runnow.sh") | crontab -

