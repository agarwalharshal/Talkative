#include <iostream>
#include <winsock.h>
//#pragma comment(lib, "Ws2_32.lib")
using namespace std;
#define PORT 9909

struct sockaddr_in srv;
fd_set fr, fw, fe; //FD_SETSIZE max is 64
int nMaxFd;
int nSocket;
int n = 5; //number of clients to be handled
int nArrClient[5]; //5 = n

void ProcessNewMessage(int nClientSocket)
{
	cout << endl << "Processing the new message for client socket:" << nClientSocket;
	char buff[256 + 1] = { 0, };
	int nRet = recv(nClientSocket, buff, 256, 0);
	if (nRet < 0)
	{
		cout << endl << "Something wrong happened. Closing the connection for client";
		closesocket(nClientSocket);
		for (int nIndex = 0; nIndex < n; nIndex++)
		{
			if (nArrClient[nIndex] == nClientSocket)
			{
				nArrClient[nIndex] = 0;
				break;
			}
		}
	}
	else
	{
		cout << endl << "The message received from client is:" << buff;
		//Send the response to client
		send(nClientSocket, "Processed your request", 23, 0);
		cout << endl << "*******************************************";

	}
}

void ProcessTheNewRequest()
{
	//New connection request
	if (FD_ISSET(nSocket, &fr))
	{
		int nLen = sizeof(struct sockaddr);
		int nClientSocket = accept(nSocket, NULL, &nLen);
		//nSocket is the listener socket using which we are just opening a channel where we are able to receive new requests from the client
		//but we cannot use this socket to communicate over the network with the same client
		//using the accept call we will get a new client socket we will get a new socket id
		//with this socket id nClientSocket, we can communicate with the server
		if (nClientSocket > 0) //nClientSocket has become a read set socket descriptor
		{
			//Put it into the client fd_set
			int nIndex;
			for (nIndex = 0; nIndex < n; nIndex++)
			{
				if (nArrClient[nIndex] == 0)
				{
					nArrClient[nIndex] = nClientSocket;
					send(nClientSocket, "Got the connection done successfully", 37, 0); //37 = The length, in bytes, of the data in buffer pointed to by the buf parameter. //0=flag
					break;
				}
			}
			if (nIndex==n)
			{
				cout << endl << "No space for a new connection";
			}
			else
			{
				for (int nIndex = 0; nIndex < n; nIndex++)
				{
					if (FD_ISSET(nArrClient[nIndex], &fr))
					{
						//Got the new message from the client
						//Just recv the new message
						//Just queue that for new worker of your server to fulfill the request
						ProcessNewMessage(nArrClient[nIndex]);
					}
				}
			}
		}
	}
}

int main()
{
	int nRet = 0;

	//Initialize the WSA variables
	WSADATA ws;
	if(WSAStartup(MAKEWORD(2,2), &ws) < 0)
	{
		cout << endl << "WSA failed to initialize";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
		cout << endl << "WSA initialized";

	//Initialize the socket
	nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //socketID
	if (nSocket < 0)
	{
		cout << endl << "The Socket has NOT opened";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
		cout << endl << "The Socket has opened successfully "<<nSocket;
	
	//Initialize the environment for sockaddr structure
	srv.sin_family = AF_INET;
	srv.sin_port = htons(PORT);
	srv.sin_addr.s_addr = INADDR_ANY; //provide system IP address //if we want to provide ip address then use inet_addr("127.0.0.1")
	memset(&(srv.sin_zero), 0, 8);

	//About the Blocking vs Non blocking sockets
	// by default every socket is a blocking socket that's why commenting these lines out
	//optval = 0 means blocking and !=0 means non blocking
	//u_long optval = 0;
	//nRet = ioctlsocket(nSocket, FIONBIO, &optval);	//function for windows to help we set the socket blocking or non blocking
	//if (nRet != 0)
	//	cout << endl << "ioctlsocket call failed";
	//else
	//	cout << endl << "ioctlsocket call passed";

	//setsockopt
	int nOptVal = 0;
	int nOptLen = sizeof(nOptVal);
	nRet = setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOptVal, nOptLen);
	if (!nRet)
	{
		cout << endl << "The setsockopt call successful.";
	}
	else
	{
		cout << endl << "The setsockopt call failed.";
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	
	//Bind the socket to the local port
	nRet = bind(nSocket, (sockaddr*)&srv, sizeof(sockaddr));
	if (nRet < 0)
	{
		cout << endl << "Failed to bind to local port";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
		cout << endl << "Successfully bind to local";

	//Listen the request from client (queues the requests)
	nRet = listen(nSocket, 5); //backlog->number of active requests the server can pull
	if (nRet < 0)
	{
		cout << endl << "Failed to start listen to local port";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
		cout << endl << "Started listening to local port";

	

	nMaxFd = nSocket;
	struct timeval tv;
	tv.tv_sec = 1; //wait one sec to see if my read write exception socket ready ornot whether they contain anything or not
	tv.tv_usec = 0;

	while (1)
	{
		FD_ZERO(&fr);
		FD_ZERO(&fw);
		FD_ZERO(&fe);

		FD_SET(nSocket, &fr);
		FD_SET(nSocket, &fe);

		for (int nIndex = 0; nIndex < n; nIndex++)
		{
			if (nArrClient[nIndex] != 0)
			{
				FD_SET(nArrClient[nIndex], &fr);
				FD_SET(nArrClient[nIndex], &fe);
			}
		}

		//cout << endl << "Before select call:" << fr.fd_count;

		//Keep waiting for new requests and proceed as per the request
		nRet = select(nMaxFd + 1, &fr, &fw, &fe, &tv); //after the call, clears fr, fw, fe. need to set them again if we want to use them
		if (nRet > 0)
		{
			//when someone connects or communicates with a message over a dedicated connection

			//the socket listening to new clients and then communicating with the same client later are not the same.
			//after connection, we get one more socket to communicate with client
			//we need to send/receive message over the network using that new socket.
			cout << endl << "Data on port.. Processing now..";
			
			//Process the request
			ProcessTheNewRequest();
			//if (FD_ISSET(nSocket, &fe))
			//	cout << endl << "There is an exception.";
			//if (FD_ISSET(nSocket, &fw))
			//	cout << endl << "Ready to write something.";
			//if (FD_ISSET(nSocket, &fr))
			//{
			//	cout << endl << "Ready to read. Something new came at the port";
			//	//Accept the new connection
			//}
			//break;

		}
		else if (nRet == 0)
		{
			//No connection or any communication request made or we can say that none of the socket descriptors are ready
			//cout << endl << "Nothing on port:" << PORT;
		}
		else
		{
			//it failed and your application should show some useful message
			cout << endl << "I failed.."; //if descriptor set outside the loop
			WSACleanup();
			exit(EXIT_FAILURE);
		}
		//cout << endl << "After select call:" << fr.fd_count;
		Sleep(2000);	//2 seconds
	}
	
	

	return 0;
}