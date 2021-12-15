## General Info
This is a program that was made to connect a client to a server using sockets.The client relies on sending a request to the server program in order to perform an operation made available by the server. The 4 basic operations would include upload, download, delete, and rename. 
The server runs one or more programs that share resources with and distribute work among clients. 

##Compiling in putty/linux
	g++ -pthread server.cpp -o server 
	g++ client.cpp -o client

## Running in Linux
./server [port number here] (We used ./server 12345)
If running from the same computer 
	./client localhost 12345
If running from Lamar servers
	./client 140.158.130.245 12345 

##Files of Note:
server.cpp
client.cpp

##Notes:
For the code to work, the server and client source files--like in a real world situation--must be seperated into different file locations to work properly. 