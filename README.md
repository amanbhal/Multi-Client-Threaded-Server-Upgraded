1. Introduction

The task is to implement a simple memory-only shared state key value server in C++11 on Linux.


2. Objective

Clients connect to the specified address on the specified port and issue 0 to N requests.  Requests and responses are separated by newline characters. Clients expect to receive responses via the same connection that the requests were sent.  Clients expect one response for each request. Each request will be at most 1024 bytes including the newline.  The server must support multiple concurrent client connections. The keys and values being operated on are global (i.e. changes via one connection must be observable on all other connections).  You may assume all input from the client is correct and need not guard against invalid input. The provided "test_server.py" (see below) verfies these requirements.

There are three types of requests:
* set key val
* get key
* quit

set:
The set command associates a value with a key for later retrieval. "set" should return a response of the form "key=val" without the quotes. If the specified key already exists, the value should be replaced with the newly specified value. The set command is analogous to writing to a variable.

get:
The get command retrieves a previously set value using a key. "get" should return a response of the form "key=val" without the quotes. If there is no value for the specified key, the response should be of the form "key=null". The get command is analogous to reading a variable.

quit:
The quit command instructs the server to close the connection. No response should be sent; simply close the connection. It is optional for the client to send a quit command before disconnecting (i.e. not all clients will send a quit request before disconnecting).

Example session:

Request         Response

set foo bar     foo=bar
get foo         foo=bar
get bar         bar=null
set 1a2b3c foo  1a2b3c=foo  
set 10 ten      10=ten
get foo         foo=bar
set foo baz     foo=baz
get ten         ten=null
get foo         foo=baz
get qux         qux=null
quit            <server closes connection>

All keys and values are ASCII characters at least 1 character in length and consist solely of some combination of a-z, A-Z and 0-9 (e.g. keys and values do not contain spaces or special characters). All arguments are separated from each other (and the respective command, e.g. "get" or "set") by a single space.

"server.cpp" has been provided as a starting point. The code will listen on the address and port specified on the command line and provides an accept_new_connection() method which is a thin wrapper around the accept call. You may use, modify, copy, or ignore the provided code in any way you see fit.


3. Verification

"test_server.py" has been provided to help in testing your implementation. It consists of basic tests to verify adherence to the specifications described here. The tests provided take 1-2 minutes depending on the speed of your computer and your implementation of the server. There are additional tests not provided that will be used when evaluating your submission. You may add any additional tests you find useful, but you must not modify any of the existing tests. Submissions that do not pass all of the provided tests will not be evaluated.

"test_server.py" expects to find an executable named "server" in the same directory it is run from and will manage starting/stopping the server (i.e. you should not have a server running when you call "test_server.py"). "test_server.py" uses python unittest. Calling it with no arguments will run all tests while calling with a test name (e.g. TestServer.test_something) will only call that test. You may find "test_server.py -v" and/or "test_server.py --help" useful. Running individual tests at a time will allow you to view the logs for that specific run.

It is highly suggested you read through the "test_server.py" code to aid in understanding how the server is tested which is an approximate embodiment of the requirements/specifications listed here.

"test_server.py" uses unittest from python 2.7. If you encounter errors, check if your system is using python 3.


4. Hints

A makefile to build the server has been provided for your convenience. You may copy and/or modify it as required to fit your submission so long as you do not modify the compiler options (i.e. the CPP_FLAGS variable).

You may use any standard libraries/functions/data structures/containers as well as any standard Linux/system/POSIX headers/functions, but you may not use any third party packages or libraries. (e.g. not Boost)

You may use any non-human, non-interactive resources you desire. Books, blogs, forums, websites, etc. are all acceptable resources so long as you are not posting questions in relation to the assignment. The work must be your own.

Your submission will be compiled with g++ 4.8.4 on Ubuntu Linux though the provided compiler options should ensure code that compiles on your system will have a high likelihood of compiling on any other modern Linux system with an up-to-date compiler.

If you are unfamiliar with sockets on Linux, you may find the commands (and/or google search terms) "man recv", "man send" and/or "man accept4" useful.

In the context of this task, "newline" refers to the newline character, most easily represented as "\n" - specifically the single byte represented in hex as 0x0a.
