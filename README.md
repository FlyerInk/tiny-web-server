# Tiny Web Server

## Introduction

`Tiny Web Server` is a small C program implementing a simple web server of limited features. The name `Tiny Web Server` is taken from the textbook `Computer System: A Programmer's Perspective` and the code here is re-typed almost according to that textbook.

When run as a web server, this program has limited functions: serving a HTML web page, supporting only the GET method and a few HTTP Status Code. The most intriguing feature of this simple server is that it supports concurrency implemented with totally C APIs.

According to the textbook, concurrency can be achieved by multi-processes(using `fork`), I/O multiplexing(using `select`) and `Pthread`. The code here covers the forking process and pthread now. The `I/O multiplexing` is not implemented yet and will be added sometime.

## Build and run

**Prerequisite for building:**

I have built this program successfully on a 64bit Debian virtual machine hosted on Mac OS. I can not make sure it works on other machines, so here is machine information as the output of  `uname -a` for your reference:
> Linux debian 3.16.0-4-amd64 #1 SMP Debian 3.16.7-ckt25-2 (2016-04-08) x86_64 GNU/Linux

**Steps for building:**

1. Clone this repository.
2. Change into root directory of this project.
3. Run the makefile in the Shell by typing `make`. <br>If built successfully, the executable file `ws.out` could be generated into the root directory.
4. Start the server by typing `./ws.out`. <br>If everything goes well, the server will listen on the default port 8007. Alternatively, you can specify the listening port when starting it as `./ws.out <port_number>`.
5. Open a web browser and visit the home page by entering the url `http://localhost:8007` (default port) or `http://localhost:<port_number>` if you have specified the listening port other than the default one.

To test the concurrency based on different implementations, you can check out the branch `server-fork` or `server-pthread` and follow the building steps to build and run again.

## Tips

You can install and run the `siege` program to test the concurrency of the Tiny Web server.

## Comments

For me, `Tiny Web Server` is a simple but exciting program to learn the charm of the C language and help me to comprehend many fundamental and influential programming concepts in a running code way. I take some notes resided in the `notes` directory. You are welcome to visit my blog for [my notes about the Tiny Web Server](http://www.xiesiyi.com).

## License

Since I just use this code for learning, the code here is almost the the same as the textbook, so I can NOT grant any permission to anyone.

For commercial use, you should contact the author or the publisher of the book `Computer Systems: A Programmer's Perspective` for authorization.
