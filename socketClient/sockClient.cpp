#include <iostream>
#include <winsock.h>
//#pragma comment(lib, "Ws2_32.lib")
using namespace std;
#define PORT 9909

int nClientSocket;
struct sockaddr_in srv;

int main()
{
	int nRet = 0;

	//Initialize the WSA variables
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws) < 0)
	{
		cout << endl << "WSAStartup failed";
		WSACleanup();
		return (EXIT_FAILURE);
	}

	//Initialize the socket
	nClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //socketID
	if (nClientSocket < 0)
	{
		cout << endl << "socket call failed";
		WSACleanup();
		return (EXIT_FAILURE);
	}

	//Initialize the environment for sockaddr structure
	srv.sin_family = AF_INET;
	srv.sin_port = htons(PORT);
	srv.sin_addr.s_addr = inet_addr("127.0.0.1"); //for client you need to specify a particular IP address
	memset(&(srv.sin_zero), 0, 8);

	nRet = connect(nClientSocket, (struct sockaddr*)&srv, sizeof(srv));
	if (nRet < 0)
	{
		cout << endl << "connect failed";
		WSACleanup();
		return (EXIT_FAILURE);
	}
	else
	{
		cout << endl << "Connected to the Server";
		//Talk to the client
		char buff[255] = { 0, };
		recv(nClientSocket, buff, 255, 0); //recv is a blocking call. it will wait until it receives something
		cout << endl << "Just press any key to see the message received from the server";
		getchar();
		cout << endl << buff;
		cout << endl << "Now send your messages to server:";
		while (1)
		{
			fgets(buff, 256, stdin);
			send(nClientSocket, buff, 256, 0);
			cout << endl << "Press any key to get the esponse from server..";
			getchar();
			recv(nClientSocket, buff, 256, 0);
			cout << endl << buff << endl << "Now send next message:";
		}
	}

	return 0;
}