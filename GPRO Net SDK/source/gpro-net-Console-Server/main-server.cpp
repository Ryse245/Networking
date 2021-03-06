/*
   Copyright 2021 Daniel S. Buckstein

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/*
	GPRO Net SDK: Networking framework.
	By Daniel S. Buckstein

	main-server.c/.cpp
	Main source for console server application.
*/

#include "gpro-net/gpro-net.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "RakNet/RakPeerInterface.h"

#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakNetTypes.h"  // MessageID
#include "RakNet/GetTime.h"

//#define MAX_CLIENTS 10
//#define SERVER_PORT 60000
/*
enum GameMessages
{
	ID_GAME_MESSAGE_1 = ID_USER_PACKET_ENUM + 1,
	ID_TEXT_CHAT
};
*/
int main(void)
{
	const unsigned short MAX_CLIENTS = 10;
	const unsigned short SERVER_PORT = 7777;
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;

	RakNet::SocketDescriptor sd(SERVER_PORT, 0);
	peer->Startup(MAX_CLIENTS, &sd, 1);
	peer->SetMaximumIncomingConnections(MAX_CLIENTS);

	RakNet::SystemAddress clientAddresses[10];
	RakNet::RakString usernames[10];
	
	printf("Starting the server.\n");
	//fputs("HaHaPenis", logfile);
	//fclose(logfile);
	// We need to let the server accept incoming connections from the clients

	while (1)
	{
		for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			//FOR SERVER READING IN MESSAGE
			RakNet::Time sentTime = 0;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			RakNet::MessageID msg = 0;//packet->data[0];
			//RakNet::RakString rs;
			//bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			bsIn.Read(msg);

			if (msg == ID_TIMESTAMP)	//FOR SERVER READING IN MESSAGE
			{
				//handle time
				//1. Bitstream DONE ABOVE
				//2. Skip msg byte DONE ABOVE
				//3. Read time
				bsIn.Read(sentTime);
				//4. Read new msg byte
				bsIn.Read(msg);
			}
			switch(msg)
			{
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
				printf("Another client has disconnected.\n");
				break;
			case ID_REMOTE_CONNECTION_LOST:
				printf("Another client has lost the connection.\n");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION:
				printf("Another client has connected.\n");
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				printf("Our connection request has been accepted.\n");

				// Use a BitStream to write a custom user message
				// Bitstreams are easier to use than sending casted structures, and handle endian swapping automatically
				RakNet::BitStream bsOut;
				bsOut.Write((RakNet::MessageID)ID_TIMESTAMP);
				bsOut.Write((RakNet::Time)RakNet::GetTime());
				bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
				bsOut.Write("Hello world\n");
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			}
			break;
			case ID_NEW_INCOMING_CONNECTION:
				printf("A connection is incoming.\n");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				printf("The server is full.\n");
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					if (packet->systemAddress == clientAddresses[i])
					{
						usernames[i] = &RakNet::RakString().emptyString;
						clientAddresses[i] = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
					}
				}
				printf("A client has disconnected.\n");
				break;
			case ID_CONNECTION_LOST:
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (packet->systemAddress == clientAddresses[i])
					{
						usernames[i] = &RakNet::RakString().emptyString;
						clientAddresses[i] = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
					}
				}
				printf("A client lost the connection.\n");
				break;

			case ID_GAME_MESSAGE_1:
			{
				RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());
			}
			break;
			case ID_TEXT_CHAT:
			{
				//printf("Text message recieved \n");
				RakNet::RakString rs;
				//RakNet::BitStream bsIn(packet->data, packet->length, false);
				//bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				printf("%s\n", rs.C_String());
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (packet->systemAddress == clientAddresses[i])
					{
						RakNet::BitStream bsOut;
						bsOut.Write((RakNet::MessageID)ID_TEXT_CHAT);
						bsOut.Write(rs);
						peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);
						FILE* logfile = fopen("Z:\\textlog.txt", "a");	//For server, send to all
						rs.AppendBytes(" Username: ", sizeof(" Username: "));
						rs.AppendBytes(usernames[i].C_String(), sizeof(usernames[i].C_String()));
						rs.AppendBytes(" Time: ", sizeof(" Time: "));
						//rs.AppendBytes((char*)((int)sentTime), sizeof(sentTime));
						int a = fputs(rs.C_String(), logfile);
						//a = fputs((char*)((int)sentTime), logfile);
						fprintf(logfile,"%i",(int)sentTime);
						a = fputs("\n", logfile);
						fclose(logfile);
						break;
					}
					else if (packet->systemAddress != clientAddresses[i] && clientAddresses[i]==RakNet::UNASSIGNED_SYSTEM_ADDRESS )
					{
						clientAddresses[i] = packet->systemAddress;
						usernames[i] = rs;
						printf("%s has joined server\n", usernames[i].C_String()); 
						RakNet::BitStream bsOut;
						RakNet::RakString newUser = RakNet::RakString("%s has joined server\n", usernames[i].C_String());
						bsOut.Write((RakNet::MessageID)ID_TEXT_CHAT);
						bsOut.Write(newUser);
						peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);

						bsOut.Reset();
						bsOut.Write((RakNet::MessageID)ID_TEXT_CHAT);
						bsOut.Write("Welcome to the chatroom!");
						peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
						break;
					}
				}

				//printf("%i",a);
				break;
			}
			case ID_USERNAMES_REQUEST:
			{
				RakNet::RakString userNameList = "Users: \n";
				RakNet::BitStream bsOut;
				for (int count = 0; count < MAX_CLIENTS; count++)
				{
					if (usernames[count] != &RakNet::RakString().emptyString)
					{
						userNameList.AppendBytes(usernames[count].C_String(), sizeof(usernames[count]));
						userNameList.AppendBytes("\n", sizeof("\n"));
					}
				}
				bsOut.Write((RakNet::MessageID)ID_USERNAMES_REQUEST);
				bsOut.Write(userNameList);
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
				break;
			}
			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
		}
	}


	RakNet::RakPeerInterface::DestroyInstance(peer);

	return 0;
}

/*
int main(int const argc, char const* const argv[])
{


	printf("\n\n");
	system("pause");
}
*/