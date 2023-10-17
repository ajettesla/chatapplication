TARGET: client server

client: client.o
	g++ -g client.o -o client -lpthread -lstdc++
client.o: client.cpp
	g++ -g -c client.cpp -o client.o
server: server.o
	g++ -g server.o -o server -lpthread -lstdc++
server.o: server.cpp
	g++ -g -c server.cpp -o server.o 
	
clean:
	rm -rf client.o server.o client server


