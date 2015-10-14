// TODO: 
// - "check the return value of all system calls"
// - add print statements 
// - "there should be a function that reads a string that is prefixed by length"

#include "header.h"

struct sockaddr_in serv_addr, cli_addr, tmp_addr;  // struct containing addr
struct hostent *hp;   							   // defines host comp

int sock, comsock;  // sock = server sock, comsock = client communication socket
int servPort;       // server port number
int res, len, n;    // help variables

struct User db[DB_SIZE];

// Populates name-id database
void db_populate()
{
	db[0].name = "joanne";
	db[0].id = "0123";
	db[0].password = "iforgot";

	db[1].name = "jordan";
	db[1].id = "4567";
	db[1].password = "mypassword";

	db[2].name = "cammi";
	db[2].id = "8910";
	db[2].password = "changeme";
}

// Looks up name-id pair in database. Returns index if found, -1 otherwise.
int db_lookup_nameid(const char* name, const char* id)
{
	int i;
	printf("Looking up Name: '%s' ID: '%s'\n", name, id);
	for (i = 0; i < DB_SIZE; i++)
	{
		printf("\tComparing to '%s' and '%s'\n", db[i].name, db[i].id);
		if (!strcmp(db[i].name, name) && !strcmp(db[i].id, id))
		{
			return i; 
		}
	}
	return -1;
}

// Checks if password matches name-id pair. Returns nonzero if match, zero if not match
int db_lookup_password(int index, const char* password)
{
	printf("\tComparing %s with actual password %s\n", password, db[index].password);
	return !strcmp(db[index].password, password);
}

// Initialize the server.
int server_init(char* argv[])
{
	struct in_addr **addr_list; // janky way to get host addr

	printf("==================== server init ======================\n"); 

	printf("Retrieving hostname and IP...\n");
	hp = gethostbyname("localhost");
	if (!hp)
	{
		printf("Error: Unknown host.");
		exit(1);
	}
	addr_list = (struct in_addr**)hp->h_addr_list;
	printf("\thostname=localhost  IP=%s\n", inet_ntoa(*addr_list[0]));

	printf("Creating a TCP socket...\n");
	
	// domain=>IPv4, type=>stream, protocol=>OS choose TCP for stream socket
	sock = socket(AF_INET, SOCK_STREAM, 0); 
	if (sock < 0)
	{
		printf("Error: Failed to create socket.\n");
		exit(2);
	}

	printf("Populating serv_addr with IP and Port#...\n");
	serv_addr.sin_family = AF_INET;                  // for TCP
   	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // serv IP address  

   	// assign port from command line arg
   	serv_addr.sin_port = htons((uint16_t) strtoul(argv[1], NULL, 0));

	printf("Binding socket to serv_addr info...\n"); 
	res = bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (res < 0)
	{
		printf("Error: Failed to bind.\n");
		exit(3);
	}

	printf("Finding kernel assigned Port#...\n");
	len = sizeof(tmp_addr);
	// get current addr to where sock is bound
	res = getsockname(sock, (struct sockaddr *)&tmp_addr, &len);
   	if (res < 0)
   	{
		printf("Error: getsockname() failed.\n");
		exit(4);
   	}

   	servPort = ntohs(tmp_addr.sin_port);
   	printf("\tPort=%d\n", servPort);

   	printf("Populating name-id database...\n");
   	db_populate();
   	
   	printf("=============== server init complete ==================\n");
}

// MAIN
main(int argc, char *argv[])
{
	char* msg;
	int n, found_pair, is_valid;  
	uint16_t passlen, finalmsglen;
	char outbuf[MAX], id[MAX], name[MAX], password[MAX];   // Cammi

	if (argc < 2)
	{
		printf("Usage: ./server serverPort\n");
		exit(1);
	}

	// initialize server
	server_init(argv);

	// while still serving
	while(1)   // do not quit until Ctrl-C
	{
		// listen at serverPort 
   		listen(sock, QUEUE_MAX);
		printf("\n\nServer is listening...\n");

		// try to accept a client connection 
		len = sizeof(cli_addr);
		comsock = accept(sock, (struct sockaddr *)&cli_addr, &len);
		if (comsock < 0)
		{
			printf("Error: Failed to accept connection.\n");
			exit(1);
		}

		printf("Server accepted connection from\n");
		printf("-----------------------------------------------\n");
     	printf("\tIP=%s   port=%d\n", 
     		(char*)inet_ntoa(cli_addr.sin_addr),
            ntohs(cli_addr.sin_port));
     	printf("-----------------------------------------------\n");

     	// while still talking to client
     	while(1)
     	{
	     	// send welcome msg as newline terminated string
	     	printf("Creating and sending welcome msg...");
	     	msg = "Welcome";

	     	bzero(outbuf, MAX);  
	     	strncpy(outbuf, msg, strlen(msg));
	     	outbuf[strlen(msg)] = '\n';

	     	n = send_all(comsock, outbuf, strlen(outbuf), 0);  
	     	printf(" %d bytes sent\n", n);

			// recv client name and print it
			n = recv_all(comsock, name, MAX-1, 0);
			name[n-1] = 0;   // null terminate for printing
			printf("Name: %s\n", name);

			// recv client ID and print it
			n = recv_all(comsock, id, MAX-1, 0);
			id[n-1] = 0;   // null terminate for printing
			printf("ID: %s\n", id);

			// found name-id pair => Success
			found_pair = db_lookup_nameid(name, id);
			msg = (found_pair >= 0) ? "Success" : "Failure";

			// send name-id Success/Failure message
	     	bzero(outbuf, MAX); 
			strncpy(outbuf, msg, strlen(msg));
			outbuf[strlen(msg)] = '\n';

			printf("Sending %s message...", msg);
			n = send_all(comsock, outbuf, strlen(outbuf), 0);  
			printf(" %d bytes sent\n", n);

			// pair not found => close socket and go back to listening
			if (found_pair < 0)
			{
				printf("Name-ID verification failed. Closing connection...\n");
				close(comsock);
				break;  
			}

			// pair found => carry on
			
			// receive passlen and password
			recv_msg_with_len(comsock, password);
			printf("Password: %s\n", password);

			// check if password valid
			is_valid = db_lookup_password(found_pair, password);

			// if password is valid for previously received ID/name
			bzero(outbuf, MAX);
			if (is_valid)
			{
			 	sprintf(outbuf, "Congratulations, %s; you've just revealed the password for %s to the world!", name, id);
			}
			// if password is invalid
			else
			{
				msg = "Password incorrect.";
				strncpy(outbuf, msg, strlen(msg));;
			}
			outbuf[strlen(outbuf)] = '\n';
			finalmsglen = strlen(outbuf)-1; // don't count newline

			// send finalmsglen and final_msg
			printf("Sending final message of %hu bytes...\n", finalmsglen);
			send_msg_with_len(comsock, finalmsglen, outbuf);

			printf("Final message successfully sent! Closing connection...");

			// close connection
			close(comsock);
			break;
     	}
	}

    return 0;
}
