# ptbm2
Parentheses Tree Based Multicast header processor 2

## Usage:

```ptbm2 [options]```

**Options:**

* **--brackets BRACKETS**
Brackets in the header (default: none).
For example: ()(())
* **--numbers NUM1,NUM2,NUM3..**
Numbers in the header (default: none)
For example: 3,0,8
* **--virtual NUM1,NUM2,NUM3..**
Virtual ports (default: none).
* **--origttl NUM**
Original TTL (default: 64)
* **--currttl NUM**
Current TTL (default: 64)
* **--generate**
Generate header (binary)
* **--input**
Input header (binary)
* **--print**
Print header
* **--help**
Print usage

### Scenerio 1

You have the multicast tree in its textual form (brackets, numbers, ttls) and the router has one virtual port (number: 1). You want simulate the processing of this header by the router. Original and current TTLs are 64, so we are the first router in the tree. 

Command to execute:

```./ptbm2 --brackets "()((()())()())((()()())())(())((()(()()()))())" --numbers 2,3,4,5,6,7,8,9,0,0,1,2,10,11,12,1,0,2,3,4,5,6,1 --virtual 1 --origttl 64 --currttl 64```

Result (port_number ttl_to_send) :

```
2 0
3 62
9 56
11 50
32 47
33 0
```

### Scenerio 2

You have the multicast tree in its textual form (brackets, numbers, ttls). You want simulate the processing of this header by the router. Original TTL is 64, the current TTL is 62, so we are the router at port 3 (of Scenerio 1's router)

Command to execute:

```./ptbm2 --brackets "()((()())()())((()()())())(())((()(()()()))())" --numbers 2,3,4,5,6,7,8,9,0,0,1,2,10,11,12,1,0,2,3,4,5,6,1 --origttl 64 --currttl 62```

Result (port_number ttl_to_send) :

```
4 61
7 0
8 0
```

### Scenerio 3

Converting the multicast tree from its textual form (brackets, numbers, ttls) to its binary form.

Command to execute:

```./ptbm2 --brackets "()((()())()())((()()())())(())((()(()()()))())" --numbers 2,3,4,5,6,7,8,9,0,0,1,2,10,11,12,1,0,2,3,4,5,6,1 --origttl 64 --currttl 64 --generate```

Result:

```0100000001000000000000000000000000000000000000000000000000000000000000000000000000000001011001010100001100100000000111001011101000100001000000001001100001110110010101000011001000000000000000000000000000000000000010001010110111001100100101011100101001011101```

### Scenerio 4

Converting the multicast tree from its its binary form to its textual form (brackets and numbers).

Command to execute:

```--input --print```

Note: you have to enter binary from the console or redirect STDIN

Result:

```()((()())()())((()()())())(())((()(()()()))()) 2,3,4,5,6,7,8,9,0,0,1,2,10,11,12,1,0,2,3,4,5,6,1 64,64```

### Scenerio 5

Simulate processing of the header in its binary form.

Command to execute:

```--input```
Note: you have to enter binary from the console or redirect STDIN

Result:

```
0 55
10 0
```

## Build
Program should build on any UNIX like or Windows operation system with a standard C++11 compiler, qmake and make utility.

To compile:
```
qmake ptbm2.pro
make
```

To run:
```
./ptbm2 --help
```
To clean:
```
make clean
```

## Emscripten

Copy ptbm2 folder to the Emscripten folder. Then run:

```
emcc --bind -Iptbm ptbm/ptbm2-emscripten.cpp -o ptbm2.js
```

Upload ptbm2.js, ptbm2.wasm and ptbm2/ptbm2.html to a directory of a web server.

Note: application/wasm mime type must be set on the server! 

Live demo: https://majdan.hu/ptbm/ptbm2.html

## Author

Written by Andras Majdan.

License: GNU General Public License Version 3

Report bugs to <majdan.andras@gmail.com>

