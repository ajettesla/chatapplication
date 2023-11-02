TARGET: client server

client: client.o
	g++ -g client.o -o cchat -lpthread -lstdc++
client.o: client.cpp
	g++ -g -c client.cpp -o client.o
server: server.o
	g++ -g server.o -o cserverd -lpthread -lstdc++
server.o: server.cpp
	g++ -g -c server.cpp -o server.o 
	
clean:
	rm -rf client.o
	rm -rf server.o 
	rm -rf cchat
	rm -rf cserverd


