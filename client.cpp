#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <strings.h>
#include <string.h>
#include <string>

typedef struct {
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

InputBuffer* new_input_buffer() {
  InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;

  return input_buffer;
}

void print_prompt() { printf("db > "); }

void read_input(InputBuffer* input_buffer) {
  ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

  if (bytes_read <= 0) {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }

  // Ignore trailing newline
  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

int main( int argc, char *argv[] ) {
    int sockfd, client_fd, valread;
    int portnum = 5000;
	std::string ip = "127.0.0.1";
	
    struct sockaddr_in serv_addr;
    struct hostent *server;
    //char buffer[256];

    server = gethostbyname(ip.c_str());
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portnum);

	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to open a socket");
        exit(1);
    }

    int tr = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int)) == -1) {
        perror("Setsockopt error");
        exit(1);
    }

    if((client_fd = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) < 0) {
        perror("Unable to connect to the server");
        exit(1);
    }
	
	char buffer[1024] = { 0 };
	valread = read(sockfd, buffer, 1024);
	printf("%s\n", buffer);
	
	InputBuffer* input_buffer = new_input_buffer();
    while (true) {
        print_prompt();
        read_input(input_buffer);

        if (strcmp(input_buffer->buffer, ".exit") == 0) {
            close_input_buffer(input_buffer);
			
			// closing the connected socket
			close(client_fd);
			return 0;
        }
		else {
			send(sockfd, input_buffer->buffer, strlen(input_buffer->buffer), 0);
			
			valread = read(sockfd, buffer, 1024);
			buffer[valread] = '\0'; 
			printf("%s\n", buffer);
			printf("Recieved %d bytes from server\n", valread);
		}

    }
	return 0;
}