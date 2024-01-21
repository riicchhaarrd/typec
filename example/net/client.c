// ./build && ./typec -f example/net/messages.type > example/net/messages.type.h
// gcc client.c serialize.c -o client && ./client

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "serialize.h"
#include "../../arena.h"
#include "messages.type.h"

#include <unistd.h>
#include <arpa/inet.h>

#include "settings.h"

int visit_(void *ctx, const char *key, void *value, size_t size, size_t num_elements)
{
	printf("%s = %02X\n", key ? key : "", value);
	return 0;
}
int data_type_int_to_string(int *value, size_t num_elements, char *buf, size_t bufsz)
{
	return snprintf(buf, bufsz, "%d", *value);
}

int main()
{
	MsgConnect msg;
	type_init(&msg, k_ETypeMsgConnect);
	char tmp[32];
	for(int i = 0; i < type_field_count(&msg); ++i)
	{
		ReflectionFieldTypeInfo info;
		type_field_info(&msg, i, &info);
		info.to_string(info.field, info.element_count, tmp, sizeof(tmp));
		printf("%d: %s '%s'\n", i, info.name, tmp);
	}
	
	return 0;
	int client_socket;
	struct sockaddr_in server_addr;
	char buffer[MAX_BUFFER_SIZE];

	if((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);

	if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
	{
		perror("Invalid address");
		exit(EXIT_FAILURE);
	}
	TypeFieldVisitor visitor = { 0 };
	visitor.visit = visit_;
	while(1)
	{
		printf("> ");
		fgets(buffer, MAX_BUFFER_SIZE, stdin);
		sendto(client_socket,
			   (const char *)buffer,
			   strlen(buffer),
			   0,
			   (const struct sockaddr *)&server_addr,
			   sizeof(server_addr));

		ssize_t len = recvfrom(client_socket, (char *)buffer, MAX_BUFFER_SIZE, 0, NULL, NULL);
		if(len > sizeof(Packet))
		{
			Arena scratch = { 0 };
			char temp[MAX_BUFFER_SIZE]; // 16KB
			// arena_init(&scratch, temp, sizeof(temp));

			scratch.beg = temp;
			scratch.end = temp + sizeof(temp);
			if(setjmp(scratch.jmp_oom))
			{
				printf("Out of memory, can't read messages\n");
				continue;
			}

			Packet *p = new(&scratch, Packet, 1);
			type_init(p, k_ETypePacket);
			size_t buffer_index = 0;
			deserialize_from_buffer(&buffer[buffer_index],
									sizeof(buffer) - buffer_index,
									&buffer_index,
									p,
									k_ETypePacket,
									1);

			visit(p, &visitor, NULL);
			printf("%d messages\n", p->message_count);
			for(size_t i = 0; i < p->message_count; i++)
			{
				if(buffer_index >= sizeof(buffer))
				{
					printf("Too many messages!\n");
					break;
				}
				// Try to read the message
				int32_t type_index = *(int32_t *)&buffer[buffer_index];
				size_t type_size = type_sizeof(type_index);

				if(type_size == 0)
				{
					printf("Error, no such type\n");
					// Error, this type doesn't exist.
					break;
				}
				char *message = new(&scratch, char, type_size);
				type_init(message, type_index);
				deserialize_from_buffer(&buffer[buffer_index],
										sizeof(buffer) - buffer_index,
										&buffer_index,
										message,
										type_index,
										1);
				const char *name = type_name(type_index);
				printf("Visiting %s\n", name ? name : "?");

				visit(message, &visitor, NULL);
				// If a field is private for a type, then it won't be visited so can't rely on visited fields to
				// increment a streampos e.g fwrite/fread.
			}
		}
		else
		{
			printf("Received %d bytes\n", len);
		}
	}
	close(client_socket);
	return 0;
}