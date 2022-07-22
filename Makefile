compile: clean server client

server: server.o
	g++ server.o -o server

server.o:
	g++ -c server.cpp -o server.o
	
client: client.o
	g++ client.o -o client

client.o:
	g++ -c client.cpp -o client.o
	
clean:
	rm -f *.o *.exe
