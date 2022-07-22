#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <thread>
#include <vector>
#include <string>
#include <cstring>

#define BUF_SIZE 512

void connect_client(int sock);

int main( int argc, char *argv[] ) {
	int master_socket, max_sd, sd, activity, new_socket, addrlen, valread,
	    portnum = 5000, clilen, max_clients = 30, client_socket[max_clients];
	struct sockaddr_in serv_addr, address;
	
    //set of socket descriptors 
    fd_set readfds;
	
	//data buffer of 1K
	char buffer[1025] = { 0 };

    //initialise all client_socket[] to 0 so not checked 
    for (int i = 0; i < max_clients; i++)  
    {  
        client_socket[i] = 0;  
    }  

    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        perror("Unable to open a socket");
        exit(EXIT_FAILURE);
    }
	
	int tr = 1;
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
	
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portnum);
	
    if (bind(master_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0 ) {
        perror("Error on binding");
        exit(EXIT_FAILURE);
    }
	
    if (listen(master_socket, 5) < 0) {
        perror("Error on listening");
        exit(EXIT_FAILURE);
    }
	printf("Server stared. Wating for connections..."); 
	addrlen = sizeof(address);  
	while (true) {
		
        //clear the socket set 
        FD_ZERO(&readfds); 
		
        //add master socket to set 
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;
		
        //add child sockets to set 
        for (int i = 0 ; i < max_clients ; i++)  
        {  
            //socket descriptor 
            sd = client_socket[i];  
                 
            //if valid socket descriptor then add to read list 
            if(sd > 0) FD_SET( sd , &readfds);  
                 
            //highest file descriptor number, need it for the select function 
            if(sd > max_sd) max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
       
        if ((activity < 0) && (errno!=EINTR))  
        {  
            printf("select error");  
        }

        //If something happened on the master socket , 
        //then its an incoming connection 
        if (FD_ISSET(master_socket, &readfds))  
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }
			//inform user of socket number - used in send and receive commands 
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

            //send new connection greeting message
			std::string message = "Connected to server";
            if( send(new_socket, message.c_str(), message.length(), 0) != message.length() )  
            {  
                perror("send");  
            }  

            //add new socket to array of sockets 
            for (int i = 0; i < max_clients; i++)  
            {  
                //if position is empty 
                if( client_socket[i] == 0 )  
                {  
                    client_socket[i] = new_socket;  
                    printf("Adding to list of sockets as %d\n" , i);  
                    break;  
                }  
            }  
		}
		
        //else its some IO operation on some other socket
        for (int i = 0; i < max_clients; i++)  
        {
            sd = client_socket[i];
            if (FD_ISSET( sd , &readfds))  
            {  
                //Check if it was for closing , and also read the 
                //incoming message
                if ((valread = read(sd , buffer, 1024)) == 0)
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);  
                    printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
                         
                    //Close the socket and mark as 0 in list for reuse 
                    close( sd );  
                    client_socket[i] = 0;  
                }
				
                //Echo back the message that came in 
                else 
                {  
                    //set the string terminating NULL byte on the end 
                    //of the data read 
                    buffer[valread] = '\0';  
                    send(sd , buffer , strlen(buffer) , 0 );
					printf("Sending %d bytes to ip %s port %d \n", strlen(buffer), inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                } 
				
			}
		}

    }
    // closing the connected socket
    close(master_socket);
    // closing the listening socket
    shutdown(master_socket, SHUT_RDWR);
	
    return 0;
}
