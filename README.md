# CHATSERVER

The server hosts multiple concurrent clients and needs to be run once. Then each different client can use the './chatclient' binary to connect to the server. 
The clients can choose to message everyone in the chat or direct message certain users.

### Compiling the server
```
$ cd ./server
$ make
g++ -std=c++11 -c -o chatserver.o server.cpp
g++ -std=c++11 -lpthread -lcrypto -lz chatserver.o -o chatserver
```
### Compiling the client
```
$ cd ./client
$ make
g++ -std=c++11 -c -o chatclient.o client.cpp
g++ -o chatclient -std=c++11 -lpthread -lcrypto -lz chatclient.o
```

### Launching Server
First we have to launch the server so it can connect multiple clients.
Use the following command in the '/server' folder where the chatserver binary lives. Replace 8000 below with any PORT NUMBER:
```
$ ./chatserver [PORT NUMBER]
```
Example:
```
$ ./chatserver 8000
Accepting connections on port 4000
```
### Connecting Clients
 To connect to the server you use the following sytax (IP Address of the machine the server binary is running on):
```
$ ./chatclient
```
Example:
```
$ ./chatclient
Enter the Ip Address: 127.0.0.1
Enter the Port no.: 8000
Enter the Username: Shubham
Connecting to 127.0.0.1 on port 8000
Conected to 127.0.0.1
pass: password

Successfully Joined Chat
Please enter a command: P (Public message), D (Direct message), Q (Quit)
> 
```
### Messaging 
#### Public
Here is an example of how a public message would look from one client:
```
Please enter a command: P (Public message), D (Direct message), Q (Quit)
> P
Enter the public message: test
```
The other client would see this in their console:
```
Please enter a command: P (Public message), D (Direct message), Q (Quit)
> 
*** Incoming public message ***: test
> 
```
#### Direct
Here is an example of how direct messaging works with clients:
```
Please enter a command: P (Public message), D (Direct message), Q (Quit)
> D
Peers online: 
 Alexander
 Shubham
 Partha

Peer to message: Alexander
Message: test2
```
The other client would see this in their console:
```
Please enter a command: P (Public message), D (Direct message), Q (Quit)
> 
*** Incoming private message from Sean***: test2
>
```
## How 'users.csv' Works
These are just the comma-separated list of users allowed to use the system. If upon login a user is not recognized, their username and password is added to this file. The file follows the following format:
```
[user], [password for user]
```
