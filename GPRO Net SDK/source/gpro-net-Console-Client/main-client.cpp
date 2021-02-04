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

enum GameMessages
{
	ID_GAME_MESSAGE_1 = ID_USER_PACKET_ENUM + 1
};

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

void handleInputLocal(GameState* state)
{
	//Keyboard, controller, etc
}

void handleRemoteInput(GameState* state)
{
	RakNet::RakPeerInterface* peer = state->peer;
	RakNet::Packet* packet;

	//recieve packets, merge with local input
	//for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
	while (packet = peer->Receive())
	{
		RakNet::MessageID msg = packet->data[0];
		/*if (msg == ID_TIMESTAMP)
		{
			//handle time
			//1. Bitstream
			//2. Skip msg byte
			//3. Read time
			//4. Read new msg byte
		}*/
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
			
			bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
			bsOut.Write("Hello world");
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			
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
		case ID_NEW_INCOMING_CONNECTION:
			printf("A connection is incoming.\n");
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			printf("The server is full.\n");
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			printf("We have been disconnected.\n");
			break;
		case ID_CONNECTION_LOST:
			printf("Connection lost.\n");
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

		default:
			printf("Message with identifier %i has arrived.\n", packet->data[0]);
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

void handleOutputRemote(const GameState* state)
{
	//package and send state changes to server
}

void handleOutputLocal(const GameState* state)
{
	//rendering, display
}

int main(void)
{
	const unsigned short SERVER_PORT = 7777;
	const char SERVER_IP[] = "172.16.2.186";	//get fron VDI

	GameState gs[1] = {0};

	gs->peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::SocketDescriptor sd;
	gs->peer->Startup(1, &sd, 1);
	gs->peer->SetMaximumIncomingConnections(0);
	gs->peer->Connect(SERVER_IP, SERVER_PORT, 0, 0);

	//game loop
	while (1)
	{
		//input
		handleInputLocal(gs);
		//recieve and merge
		handleRemoteInput(gs);
		//update
		handleUpdate(gs);
		//package and send
		handleOutputRemote(gs);
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