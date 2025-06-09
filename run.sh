#!/bin/bash
# Define log file
LOG_FILE="kmeans_output.log"
# Redirect both stdout and stderr to the log file
exec &> >(tee -a "$LOG_FILE")

# Define the datasets directory and dataset filenames
DATASETS_DIR="datasets"
DATASETS=("dataset3.txt") 

# Compile the serial program
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo "Compiling kmeans_serial..."
g++ -std=c++11 -O3 -march=native -pthread -o kmeans_serial kmeans-serial.cpp

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation of kmeans_serial failed. Exiting..."
    exit 1
fi

# Compile the serial program better
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo "Compiling kmeans_serial_improved..."
g++ -std=c++11 -O3 -march=native -pthread -o kmeans_serial_improved kmeans-serial-improved.cpp

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation of kmeans_serial_better failed. Exiting..."
    exit 1
fi


# Compile the parallel (TBB) program
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo "Compiling kmeans_parallel..."
g++ -std=c++11 -O3 -march=native -ltbb -o  kmeans_parallel kmeans-parallel.cpp

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation of kmeans_parallel failed. Exiting..."
    exit 1
fi

# Run both programs with each dataset
for dataset in "${DATASETS[@]}"; do
    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
    echo "Running kmeans_serial with $dataset..."
    cat "$DATASETS_DIR/$dataset" | ./kmeans_serial
    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"

    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
    echo "Running kmeans_serial_improved with $dataset..."
    cat "$DATASETS_DIR/$dataset" | ./kmeans_serial_improved
    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"

    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
    echo "Running kmeans_parallel_improved with $dataset..."
    cat "$DATASETS_DIR/$dataset" | ./kmeans_parallel_improved
    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"

done

# Clean up the executable files
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo "Cleaning up executables..."
rm -f kmeans_serial kmeans_serial_improved kmeans_parallel_improved

echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
echo "All done!"
