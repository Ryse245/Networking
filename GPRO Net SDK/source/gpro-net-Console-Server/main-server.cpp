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

//#include "gpro-net/gpro-net-common/gpro-net-gamestate.h"

//#define MAX_CLIENTS 10
//#define SERVER_PORT 60000
/*
enum GameMessages
{
	ID_GAME_MESSAGE_1 = ID_USER_PACKET_ENUM + 1,
	ID_TEXT_CHAT
};
*/

enum class PlayerTurn
{
	PLAYER_ONE,
	PLAYER_TWO
};

class GameRoom
{
public:
	int CreateRoom(RakNet::RakString playerUName, RakNet::SystemAddress playerAddress);
	int JoinRoom(RakNet::RakString playerUName, RakNet::SystemAddress playerAddress);
	void StartGame(RakNet::RakPeerInterface* peer);
	int UpdateGame(int xPos, int yPos);

	bool GetCreatedRoom() {	return createdRoom;	}
	bool GetStartGame() { return StartGame; }
	//void SetCreatedRoom(bool room) { createdRoom = room; }

	RakNet::RakString playerNames[2];
	RakNet::RakString spectatorNames[8];
	RakNet::SystemAddress playerAdresses[2];
	//Not great, but whatever
	gpro_battleship playerOne;
	gpro_battleship playerTwo;
private:

	bool createdRoom = false;
	bool startGame = false;
	PlayerTurn currentTurn = PlayerTurn::PLAYER_ONE;
};

int main(void)
{
	const unsigned short MAX_CLIENTS = 10;
	const unsigned short SERVER_PORT = 7777;
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;
	int gameIndex = 0;
	RakNet::SocketDescriptor sd(SERVER_PORT, 0);
	peer->Startup(MAX_CLIENTS, &sd, 1);
	peer->SetMaximumIncomingConnections(MAX_CLIENTS);

	RakNet::SystemAddress clientAddresses[MAX_CLIENTS];
	RakNet::RakString usernames[MAX_CLIENTS];
	GameRoom possibleRooms[MAX_CLIENTS];	//I guess in case everyone just wants to sit in their own room?
	
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
				//printf("%s\n", rs.C_String());
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (packet->systemAddress == clientAddresses[i])	//Client already "logged on"
					{
						RakNet::BitStream bsOut;
						bsOut.Write((RakNet::MessageID)ID_TEXT_CHAT);
						bsOut.Write(rs);
						peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);

						printf("%s\n", rs.C_String());

						/*
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
						*/
						break;
					}
					else if (packet->systemAddress != clientAddresses[i] && clientAddresses[i]==RakNet::UNASSIGNED_SYSTEM_ADDRESS )	//New client
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
						bsOut.Write("Welcome to the lobby!\nType '/update' to get chat messages and '/getRooms' to get a list of lobbies to join\n");
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
				//newOrder.bsID = (RakNet::MessageID)ID_BATTLESHIP;
				//bsOut.Write(newOrder, sizeof(newOrder));

				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
				break;
			}
			case ID_SETUP_BOARD:
			{
				BattleShipSetup boardSetup;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(boardSetup);
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (packet->systemAddress == possibleRooms[i].playerAdresses[0])	//If the packet system address is the same as the player address at index 0 = If the placket address == player one of room
					{
						BoardSetup(&boardSetup, possibleRooms[i], 0);
						break;
					}
					else if (packet->systemAddress == possibleRooms[i].playerAdresses[1])	//Packet address matches player 2 address
					{
						BoardSetup(&boardSetup, possibleRooms[i], 1);
						break;
					}
				}
				break;
			}
			case ID_BATTLESHIP:
			{
				//PASS IN STRUCT WITH COORDINATES
				//RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				//bsIn.Read(rs);
				for (int i = 0; i < MAX_CLIENTS; i++)
				{

				}
				break;
			}
			case ID_GET_ROOMS:
			{
				RakNet::BitStream bsOut;
				RakNet::RakString roomList = "Rooms:\n";
				bsOut.Write((RakNet::MessageID)ID_TEXT_CHAT);
				for (__int64 i = 0; i < MAX_CLIENTS; i++)
				{
					if (!possibleRooms[i].playerNames->IsEmpty())
					{
						roomList.AppendBytes((char*)i, sizeof((char*)i));	//Room index
						roomList.AppendBytes(". Player(s): ", sizeof(". Player(s): "));	//For readability when printing EX: 1. Player(s):
						//for (int j = 0; j < 2; j++)
						//{
							roomList.AppendBytes(possibleRooms[i].playerNames->C_String(), sizeof(possibleRooms[i].playerNames->C_String()));	//Not sure if this works yet
						//}	//Final print should be "0. Player(s): username1
						roomList.AppendBytes("\n", sizeof("\n"));
					}
				}
				if (roomList == "Rooms:\n")	//if no rooms were added to print
				{
					roomList.AppendBytes("No rooms available. Create one by typing '/create'\n", sizeof("No rooms available. Create one by typing '/create'\n"));

				}
				bsOut.Write(roomList);
				peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
				break;
			}
			case ID_CREATE_ROOM:
			{
				//Assign client to new empty room
				//RakNet::RakString rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				//bsIn.Read(rs);
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (!possibleRooms[i].GetCreatedRoom())
					{
						for (int j = 0; j < MAX_CLIENTS; j++)
						{
							if (clientAddresses[j] == packet->systemAddress)
							{
								possibleRooms[i].CreateRoom(usernames[j], packet->systemAddress);
								//possibleRooms[i].playerNames[0] = usernames[j];
								break;
							}
						}
						//possibleRooms[i].SetCreatedRoom(true);
						break;
					}
				}
				break;
			}
			case ID_JOIN_ROOM:
			{
				char rs;
				RakNet::BitStream bsIn(packet->data, packet->length, false);
				bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
				bsIn.Read(rs);
				int requestedRoom = rs - '0';
				//Read in requested room and assign player
				if (possibleRooms[requestedRoom].GetCreatedRoom() == true)
				{
					for (int i = 0; i < MAX_CLIENTS; i++)
					{
						if (clientAddresses[i]==packet->systemAddress)
						{
							possibleRooms[requestedRoom].JoinRoom(usernames[i], packet->systemAddress);
							break;
						}
					}
				}
				break;
			}
			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}
		}
	}
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (possibleRooms[i].GetStartGame()==false)
		{
			possibleRooms[i].StartGame(peer);
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

int GameRoom::CreateRoom(RakNet::RakString playerUName, RakNet::SystemAddress playerAddress)
{
	playerNames[0] = playerUName;
	playerAdresses[0] = playerAddress;
	//Player now has "created" a lobby and joined it
	createdRoom = true;
	return 0;
}

int GameRoom::JoinRoom(RakNet::RakString playerUName, RakNet::SystemAddress playerAddress)
{
	if (playerNames[1].IsEmpty())
	{
		playerNames[1] = playerUName;
		playerAdresses[1] = playerAddress;
	}
	else
	{
		for (int i = 0; i < sizeof(spectatorNames); i++)
		{
			if (spectatorNames[i].IsEmpty())
			{
				spectatorNames[i] = playerUName;
			}
		}
	}
	return 0;
}

void GameRoom::StartGame(RakNet::RakPeerInterface* peer)
{
	if (!playerNames[0].IsEmpty() && !playerNames[1].IsEmpty())
	{
		//Resets boards
		gpro_battleship_reset(playerOne);
		gpro_battleship_reset(playerTwo);
		//Both players in game room, start game
		for (int i = 0; i < 10; ++i)
		{
			for (int j = 0; j < 10; ++j)
			{
				playerOne[i][j] = gpro_battleship_open;	//Mark every space on both boards as open
				playerTwo[i][j] = gpro_battleship_open;
			}
		}

		//SET BOARDS
		//Send request for each players' board
		RakNet::BitStream bsOut;
		bsOut.Write((RakNet::MessageID)ID_SETUP_BOARD);
		peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, playerAdresses[0], false);
		peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, playerAdresses[1], false);


		startGame = true;
	}
	else
	{
		printf("Not enough players to start.\n");
	}
}

void BoardSetup(BattleShipSetup* setup, GameRoom& room, int playerIndex)
{
	//Making the player boards into an array could simplify this, but I don't care at this point
	if (playerIndex == 0)
	{
		for (int j = 0; j < BOAT_SPACES; j++)
		{
			if (j < 2)	//From 0 to 1: Patrol boat (2 spots)
			{
				room.playerOne[setup->xCoords[j] - '0'][setup->yCoords[j] - '0'] += gpro_battleship_ship_p2;
			}
			else if (j < 5)	//From 2 to 4: Submarine (3 spots)
			{
				room.playerOne[setup->xCoords[j] - '0'][setup->yCoords[j] - '0'] += gpro_battleship_ship_s3;
			}
			else if (j < 8)	//From 5 to 7: Destroyer (3 spots)
			{
				//*playerBoard[xCoord[i]][yCoord[i]] += gpro_battleship_ship_d3;
				room.playerOne[setup->xCoords[j] - '0'][setup->yCoords[j] - '0'] += gpro_battleship_ship_d3;
			}
			else if (j < 12) //From 8 to 11: Battleship (4 spots)
			{
				//*playerBoard[xCoord[i]][yCoord[i]] += gpro_battleship_ship_b4;
				room.playerOne[setup->xCoords[j] - '0'][setup->yCoords[j] - '0'] += gpro_battleship_ship_b4;
			}
			else if (j < 17) //From 12 to 16: Carrier (5 spots)
			{
				//*playerBoard[xCoord[i]][yCoord[i]] += gpro_battleship_ship_c5;
				room.playerOne[setup->xCoords[j] - '0'][setup->yCoords[j] - '0'] += gpro_battleship_ship_c5;
			}
		}
	}
	else if (playerIndex == 1)
	{
		for (int j = 0; j < BOAT_SPACES; j++)
		{
			if (j < 2)	//From 0 to 1: Patrol boat (2 spots)
			{
				room.playerTwo[setup->xCoords[j] - '0'][setup->yCoords[j] - '0'] += gpro_battleship_ship_p2;
			}
			else if (j < 5)	//From 2 to 4: Submarine (3 spots)
			{
				room.playerTwo[setup->xCoords[j] - '0'][setup->yCoords[j] - '0'] += gpro_battleship_ship_s3;
			}
			else if (j < 8)	//From 5 to 7: Destroyer (3 spots)
			{
				//*playerBoard[xCoord[i]][yCoord[i]] += gpro_battleship_ship_d3;
				room.playerTwo[setup->xCoords[j] - '0'][setup->yCoords[j] - '0'] += gpro_battleship_ship_d3;
			}
			else if (j < 12) //From 8 to 11: Battleship (4 spots)
			{
				//*playerBoard[xCoord[i]][yCoord[i]] += gpro_battleship_ship_b4;
				room.playerTwo[setup->xCoords[j] - '0'][setup->yCoords[j] - '0'] += gpro_battleship_ship_b4;
			}
			else if (j < 17) //From 12 to 16: Carrier (5 spots)
			{
				//*playerBoard[xCoord[i]][yCoord[i]] += gpro_battleship_ship_c5;
				room.playerTwo[setup->xCoords[j] - '0'][setup->yCoords[j] - '0'] += gpro_battleship_ship_c5;
			}
		}
	}
}

int GameRoom::UpdateGame(int xPos, int yPos)
{
	switch (currentTurn)
	{
	case PlayerTurn::PLAYER_ONE:

		if (playerTwo[xPos][yPos] & gpro_battleship_ship && !(playerTwo[xPos][yPos] & gpro_battleship_damage))	//Second part most likely redundant, but a good idea to check
		{
			//Hit a ship (that we haven't hit before)
			playerTwo[xPos][yPos] += gpro_battleship_damage;
			playerOne[xPos][yPos] += gpro_battleship_hit;
			playerOne[xPos][yPos] -= gpro_battleship_open;
		}
		else
		{
			//Missed
			playerOne[xPos][yPos] += gpro_battleship_miss;
			playerOne[xPos][yPos] -= gpro_battleship_open;
		}

		currentTurn = PlayerTurn::PLAYER_TWO;
		break;
	case PlayerTurn::PLAYER_TWO:

		if (playerOne[xPos][yPos] & gpro_battleship_ship && !(playerOne[xPos][yPos] & gpro_battleship_damage))	//Second part most likely redundant, but a good idea to check
		{
			//Hit a ship (that we haven't hit before)
			playerOne[xPos][yPos] += gpro_battleship_damage;
			playerTwo[xPos][yPos] += gpro_battleship_hit;
			playerTwo[xPos][yPos] -= gpro_battleship_open;

		}
		else
		{
			//Missed
			playerTwo[xPos][yPos] += gpro_battleship_miss;
			playerTwo[xPos][yPos] -= gpro_battleship_open;
		}

		currentTurn = PlayerTurn::PLAYER_ONE;
		break;
	default:
		break;
	}

	int shipCountP1 = 0;
	int shipCountP2 = 0;

	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if ((playerOne[i][j] & gpro_battleship_ship) && !(playerOne[i][j] & gpro_battleship_damage))
			{
				shipCountP1++;
			}
			if ((playerTwo[i][j] & gpro_battleship_ship) && !(playerTwo[i][j] & gpro_battleship_damage))
			{
				shipCountP2++;
			}
		}
	}

	if (shipCountP1 == 0)
	{
		printf("Player Two Wins!\n");
		return 2;
	}
	if (shipCountP2 == 0)
	{
		printf("Player One Wins!\n");
		return 1;
	}

	return 0;
}