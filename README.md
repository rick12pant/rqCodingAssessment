# README for the coding assement project

## Overview

This project was built around gRPC leveraging a Ubuntu container to make it easy to distribute and make sure environments are the same. All source files are included. The client and the server leverage two seperate cmakes and will need to be compiled seperately. The also share two seperate but identical interface.proto files. This could be cleaned up in a seperate revision so that the project leverages one high level cmake and .proto, but alas, I ran out of time.

## Building the container

The dockerfile is provided to build a container that has all needed grpc dependencies. I built the container using podman on mac, but the dockerfile should translate to docker on any other OS. Once built, there are several ways to enter the container but the two most common are either by exec into the container or by leveraging VSCode.

## Compiling the Client

When inside the container, pull the repo to a directory of your choice. Navigate to the Client/build directory and execute the following to build the client application:

```
cmake .. && make
```

The application will be produced in the same build directory where the build was started. The application will be named "client"

## Compiling the Server

Similar to the client application, navigate to the Server/build directory and execute the following to build the server application:

```
cmake .. && make
```

The application will be produced in the same build directory where the build was started. The application will be named "client"

## Running the CLI and Server

The Client and the server can be brought up in any order that is desired, but the recommended procedure is to bring up first the server, followed by the client. If the client is brought up first, it will come up without an issue but commands given will produce an error.

Note: multiple CLI's can talk with the server at any given time. 

## Compiler Used

This application was built using gcc, leverage grpc, protoc, and cmake for development.

## Reasoning behind choice of data structure

I elected to leverage a std:map<uint64_t,time_t> because given the coding challenge description, a map fit most of the criteria with no modification with efficient 0(log n) for both insert and delete.

- Uniqueness: a map enforces uniquess of inputs, so duplicates would be caught.
- Sorted by default from least to greatest.
- key-value pair matches the data,time paradigm that was requested.

## Conclusions

Please let me know if additional compiling issues where discovered or if the project is lacking in any functionality.
