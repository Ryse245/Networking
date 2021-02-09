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

	main-client.c/.cpp
	Main source for console client application.
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

//#define SERVER_PORT 60000


#pragma pack(push)
#pragma pack(1)
struct GameMessage1
{
	//if timestamp, time ID and time
	//char timeID;	//ID_TIMESTAMP
	//RakNet::Time time; //using RakNet::GetTime()
	//id: char
	char msgID;
	//message
	char msg[512];
	//char* msg;
};
#pragma pack(pop)

struct GameState
{
	RakNet::RakPeerInterface* peer;
};

void handleInputLocal(GameState* state, char* msg, bool* init)
{
	if (*init!=true)
	{
		printf("Enter username and hit 'enter' to log on \n");
		fgets(msg, 512, stdin);
		//printf("Your username is %s \n",msg);	//SET USERNAME ON SERVER, SEND TIME + MSG TO SERVER
		*init = true;
	}
	else
	{
		printf("Type message\n");
		fgets(msg, 512, stdin);
	}
	//Keyboard, controller, etc
}

void handleRemoteInput(GameState* state, bool* connect)
{
	RakNet::RakPeerInterface* peer = state->peer;
	RakNet::Packet* packet;

	//recieve packets, merge with local input
	//for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
	while (packet = peer->Receive())
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

		//handle actual message data here
		switch (msg)
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
			//bsOut.Write((RakNet::MessageID)ID_TIMESTAMP);
			//bsOut.Write((RakNet::Time)RakNet::GetTime());
			
			char test[] = { "Hello World" };

			bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
			bsOut.Write(test);
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);	//Last variable is broadcast, send to all AND, if you have a system address AND TRUE, it'll send to all EXCEPT the address
			//peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);	//For server, packet systemAddress should be replaced with sender IP
			//peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RAKNET::UNASSIGNED_SYSTEM_ADRESS, true);	//For server, send to all

			*connect = true;
			/*
			GameMessage1 msg = {
				//ID_TIMESTAMP,
				//RakNet::GetTime(),
				(char)ID_GAME_MESSAGE_1,
				"Hello World"
			};
			peer->Send((char*)&msg, sizeof(msg), HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			*/
		}
		break;
		case ID_CONNECTION_ATTEMPT_FAILED:
			printf("Failed to connect.\n");
			break;
		case ID_NEW_INCOMING_CONNECTION:
			printf("A connection is incoming.\n");
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			printf("The server is full.\n");
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			printf("We have been disconnected.\n");
			*connect = false;
			break;
		case ID_CONNECTION_LOST:
			printf("Connection lost.\n");
			*connect = false;
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
			printf("Text message recieved \n");
			RakNet::RakString rs;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			bsIn.Read(rs);
			printf("%s\n", rs.C_String());
			break;
		}
		case ID_USERNAMES_REQUEST:
		{ 
			RakNet::RakString rs;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			bsIn.Read(rs);
			printf("%s", rs.C_String());
			break;
		}

		default:
			printf("Message with identifier %i has arrived.\n", msg);
			break;
		}
		//Done with curent packet
		peer->DeallocatePacket(packet);
	}
}

void handleUpdate(GameState* state)
{
	//figure out what the state actually is
}

void handleOutputRemote(const GameState* state, char* message)
{
	//package and send state changes to server
	
	RakNet::RakPeerInterface* peer = state->peer;
	RakNet::BitStream bsOut;

	if (strcmp(message,"/getUsers\n")==0)
	{
		//send request for all usernames
		bsOut.Write((RakNet::MessageID)ID_USERNAMES_REQUEST);
		peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, peer->GetGUIDFromIndex(0), false);
	}
	else
	{
		bsOut.Write((RakNet::MessageID)ID_TEXT_CHAT);
		bsOut.Write(message);
		peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, peer->GetGUIDFromIndex(0), false);	//JANK
	}

	if (sizeof message > 0)
	{
		memset(message, 0, sizeof message);
	}
	
}

void handleOutputLocal(const GameState* state)
{
	//rendering, display
}

int main(void)
{
	const unsigned short SERVER_PORT = 7777;
	const char SERVER_IP[] = "172.16.2.61";	//get fron VDI

	GameState gs[1] = {0};

	char message[512];
	bool initialized = false;
	bool connected = false;

	gs->peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::SocketDescriptor sd;
	gs->peer->Startup(1, &sd, 1);
	gs->peer->SetMaximumIncomingConnections(0);
	gs->peer->Connect(SERVER_IP, SERVER_PORT, 0, 0);

	//game loop
	while (1)
	{
		//input
		handleInputLocal(gs, message, &initialized);
		//recieve and merge
		handleRemoteInput(gs, &connected);
		//update
		handleUpdate(gs);
		//package and send
		
		if (connected==true)
		{
			handleOutputRemote(gs, message);
		}
		//output
		handleOutputLocal(gs);
	}

	return 0;
}

/*
int main(int const argc, char const* const argv[])
{


	printf("\n\n");
	system("pause");
}
*/