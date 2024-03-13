# Use a base image with g++ installed
FROM gcc:latest

# Set the working directory inside the container
WORKDIR /usr/src/app

# Copy the source code files into the container
COPY processor.cpp generator ./

# Compile the source code with g++
RUN g++ -std=c++20 -o processor processor.cpp

# Copy the shell script into the container
COPY run.sh ./

# Make the shell script executable
RUN chmod +x run.sh

# Set the entry point to the shell script
ENTRYPOINT ["./run.sh"]

