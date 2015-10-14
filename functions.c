#include "header.h"

// Receives from socket until newline.
int recv_all(int sockfd, char* buf, size_t len, int flags)
{
	char onebuf[1];
	size_t n = 0, i = 0;
	int numRead = 0;

	while (len)
	{
		n = recv(sockfd, onebuf, 1, flags);   // recv one character at a time
		if (n <= 0 || onebuf[0] == '\n')
		{
			break;
		}
		buf[i++] = onebuf[0]; 
		len--;
	}

	return i+1;  // return bytes received
}

// Ensures all len bytes are sent into the socket.
int send_all(int sockfd, const char* buf, size_t len, int flags)
{
	size_t n = 0;
	const char* p = buf;
	while (len)
	{
		n = send(sockfd, buf, len, flags);
		if (n <= 0)
		{
			break;
		}

		p += n;
		len -= n;
	}
	return n;
}

// Sends a message prefaced by length. Returns 0 if successful, -1 otherwise.
int send_msg_with_len(int sockfd, uint16_t msglen, const char outbuf[])
{
	uint16_t msglen_cpy;
	int n;

	// send message length
	msglen_cpy = htons(msglen);   // don't include newline
	n = send(sockfd, &msglen_cpy, sizeof(uint16_t), 0);
	printf("\t%d bytes sent\n", n);

	// send message
	msglen = strlen(outbuf);
	n = send_all(sockfd, outbuf, msglen, 0);
	printf("\t%d bytes sent\n", n);

	return (n < msglen) ? -1 : 0;
}

// Receives a message prefaced by length. Returns 0 if successful, -1 otherwise.
int recv_msg_with_len(int sockfd, char msg_dest[])
{
	uint16_t msglen;
	int n;

	// recv message length
	n = recv(sockfd, &msglen, sizeof(uint16_t), 0);
	msglen = ntohs(msglen);
	printf("Expecting message of length %d bytes...\n", msglen);

	// recv message
	n = recv_all(sockfd, msg_dest, msglen+1, 0); 
	msg_dest[n-1] = 0;

	return (n < msglen) ? -1 : 0;
}