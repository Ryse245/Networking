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

	gpro-net-RakNet-Client.cpp
	Source for RakNet client management.
*/

#include "gpro-net/gpro-net-client/gpro-net-RakNet-Client.hpp"


namespace gproNet
{
	cRakNetClient::cRakNetClient()
		: index(-1)
		// Manages 16 of total 128 pots
		//	"Range of owned pots" = [i*16,(i+1)*16) OR [i*16,(i+1)*16-1] (if i = 0, range is 0-15)
		//		in a vector/matrix of objs
	{
		RakNet::SocketDescriptor sd;
		char SERVER_IP[16] = "127.0.0.1";

		peer->Startup(1, &sd, 1);
		peer->SetMaximumIncomingConnections(0);
		peer->Connect(SERVER_IP, SET_GPRO_SERVER_PORT, 0, 0);
	}

	cRakNetClient::~cRakNetClient()
	{
		peer->CloseConnection(server, true, 0, IMMEDIATE_PRIORITY);
		peer->Shutdown(0, 0, IMMEDIATE_PRIORITY);
	}

	bool cRakNetClient::ProcessMessage(RakNet::BitStream& bitstream, RakNet::SystemAddress const sender, RakNet::Time const dtSendToReceive, RakNet::MessageID const msgID)
	{
		if (cRakNetManager::ProcessMessage(bitstream, sender, dtSendToReceive, msgID))
			return true;

		// client-specific messages
		switch (msgID)
		{
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			//printf("Another client has disconnected.\n");
			break;
		case ID_REMOTE_CONNECTION_LOST:
			//printf("Another client has lost the connection.\n");
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION:
			//printf("Another client has connected.\n");
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			//printf("The server is full.\n");
			return true;
		case ID_DISCONNECTION_NOTIFICATION:
			//printf("We have been disconnected.\n");
			return true;
		case ID_CONNECTION_LOST:
			//printf("Connection lost.\n");
			return true;

		case ID_CONNECTION_REQUEST_ACCEPTED:
		{
			// set server info
			server = sender;

			// client connects to server, send greeting
			RakNet::BitStream bitstream_w;
			WriteTest(bitstream_w, "Hello server from client");
			peer->Send(&bitstream_w, MEDIUM_PRIORITY, UNRELIABLE_SEQUENCED, 0, sender, false);
		}	return true;

			// index receipt
		case ID_GPRO_MESSAGE_COMMON_BEGIN:
		{
			// read our index relative to server
			if (sender == server && index < 0)
				bitstream.Read(index);
		}	return true;
		case ID_GPRO_MESSAGE_AGENT_UPDATE:
		{
			//What the other peer sends as state
			sAgent agent, tmp;
			sAgent::Read(bitstream, agent);

			//algorithm
			//	->check distance between ours and server's
			//	IF WITHIN THRESHOLD
			//	->kinematic - position
			//		->change in time
			//	->1st order (Euler) - velocity
			//	->lerp between our copy and server info
			//		->bias/perference (local vs server)
			//	IF TOO FAR
			//	->if distance too far, snap ours to server's
			//		->or just lerp with higher preference for server
			agent = tmp;
			if (1)
			{
				//simulate
			}

			//update our own state
			agents[agent.ownerID][agent.agentID] = agent;
		}	return true;
			// test message
		case ID_GPRO_MESSAGE_COMMON_END:
		{
			// client receives greeting, just print it
			ReadTest(bitstream);
		}	return true;

		}
		return false;
	}
}