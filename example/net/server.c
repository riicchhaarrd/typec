// ./build && ./typec -f example/net/messages.type > example/net/messages.type.h
// gcc -g -w server.c serialize.c -o server && ./server

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "serialize.h"
#include "messages.type.h"

#include <unistd.h>
#include <arpa/inet.h>

#include "settings.h"

int main()
{
	int server_socket, client_socket, addr_len;
	struct sockaddr_in server_addr, client_addr;
	char buffer[MAX_BUFFER_SIZE];

	if((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	if(bind(server_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}

	printf("Server listening on port %d...\n", PORT);
	char send_data[16384];
	while(1)
	{
		addr_len = sizeof(client_addr);
		ssize_t len =
			recvfrom(server_socket, (char *)buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len);
		buffer[len] = '\0';

		printf("Client: %s", buffer);

		Packet p = { 0 };
		type_init(&p, k_ETypePacket);
		p.message_count = 1;

		size_t buffer_index = 0;

		serialize_to_buffer(&send_data[buffer_index], sizeof(send_data) - buffer_index, &buffer_index, &p, 1);

		MsgText msg = { 0 };
		type_init(&msg, k_ETypeMsgText);
		snprintf(msg.message, sizeof(msg.message), "%s", buffer);

		serialize_to_buffer(&send_data[buffer_index], sizeof(send_data) - buffer_index, &buffer_index, &msg, 1);

		// printf("Server: ");
		// fgets(buffer, MAX_BUFFER_SIZE, stdin);
		printf("Sending %d bytes\n", buffer_index);
		sendto(server_socket,
			   (const char *)send_data,
			   buffer_index,
			   0,
			   (const struct sockaddr *)&client_addr,
			   sizeof(client_addr));
	}
	close(server_socket);
	return 0;
}