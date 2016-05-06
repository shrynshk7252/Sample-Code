#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "GhostManager.h"

// Outer-Engine includes

// Inter-Engine includes

#include "../Lua/LuaEnvironment.h"

// additional lua includes needed
extern "C"
{
#include "../../luasocket_dist/src/socket.h"
#include "../../luasocket_dist/src/inet.h"
};

#include "../../../GlobalConfig/GlobalConfig.h"

#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Networking/NetworkManager.h"

#include "PrimeEngine/Scene/DebugRenderer.h"

#include "StreamManager.h"
#include "CharacterControl/ClientGameObjectManagerAddon.h"
#include "CharacterControl/CharacterControlContext.h"
#include "PrimeEngine/Networking/Server/ServerNetworkManager.h"
#include "PrimeEngine/GameObjectModel/GameObjectManager.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "CharacterControl/Events/Events.h"


// Sibling/Children includes
using namespace PE::Events;
using namespace CharacterControl;
using namespace CharacterControl::Components;


namespace PE {
	namespace Components {

		PE_IMPLEMENT_CLASS1(GhostManager, Component);

		GhostManager::GhostManager(PE::GameContext &context, PE::MemoryArena arena, PE::NetworkContext &netContext, Handle hMyself)
			: Component(context, arena, hMyself)
			, m_transmitterNextGhostOrderId(1) // start at 1 since id = 0 is not ordered
			, m_transmitterNumGhostsNotAcked(0)

			// receiver
			, m_receiverFirstGhostOrderId(1) // start at 1 since id = 0 is not ordered
		{
			m_pNetContext = &netContext;

			memset(&m_receivedGhosts[0], 0, sizeof(m_receivedGhosts));
		}

		void GhostManager::addDefaultComponents()
		{
			Component::addDefaultComponents();
			PE_REGISTER_EVENT_HANDLER(PE::Events::Event_UPDATE, GhostManager::do_UPDATE);
		}

		int GhostManager::haveGhostsToSend()
		{
			return m_GhostObjectsToSend.size() > 0;
		}

		int GhostManager::fillInNextPacket(char *pDataStream, TransmissionRecord *pRecord, int packetSizeAllocated, bool &out_usefulDataSent, bool &out_wantToSendMore)
		{
			
			if (m_pNetContext->m_clientId >= 0)
			{
				//PEINFO("\FillingPacket on Server/Host from Client: %i", m_pNetContext->m_clientId);
			}
			else
			{
				//PEINFO("\Filling Packet on Client in GMan: %i", m_pNetContext->m_clientId);

			}
			out_usefulDataSent = false;
			out_wantToSendMore = false;

			int GhostsToSend = haveGhostsToSend();
			assert(GhostsToSend);

			int GhostsReallySent = 0;

			int size = 0;
			size += StreamManager::WriteInt32(m_GhostObjectsToSend.size(), &pDataStream[size]);

			int sizeLeft = packetSizeAllocated - size;

			for (int i = 0; i < m_GhostObjectsToSend.size();i++)
			{
				int iGht = i;
				assert(iGht < (int)(m_GhostObjectsToSend.size()));
				GhostTransmissionData &ght = m_GhostObjectsToSend[iGht];

				if (ght.m_size > sizeLeft)
				{
					// can't fit this event, break out
					// note this code can be optimized to include next events that can potentailly fit in
					out_wantToSendMore = true;
					break;
				}

				// store this to be able to resolve which events were delivered or dropped on transmittion notification
				// todo: optimize to use pointers and store data somewhere else
				pRecord->m_sentGhostObjects.push_back(ght);


				memcpy(&pDataStream[size], &ght.m_payload[0], ght.m_size);
				size += ght.m_size;
				sizeLeft = packetSizeAllocated - size;

				GhostsReallySent++;
				m_transmitterNumGhostsNotAcked++;

				//Reset State Mask!
				ght.gh->stateMask = 0;
				
			}

			if (GhostsReallySent > 0)
			{
			
				m_GhostObjectsToSend.erase(m_GhostObjectsToSend.begin(), m_GhostObjectsToSend.begin() + GhostsReallySent);
			}

			//write real value into the beginning of event chunk
			StreamManager::WriteInt32(GhostsReallySent, &pDataStream[0]);

			// we are sending useful data only if we are sending events
			out_usefulDataSent = GhostsReallySent > 0;

			//PEINFO("Size of GhotsObjectsToSend: %i",m_GhostObjectsToSend.size());

			return size;
		}

		int GhostManager::receiveNextPacket(char *pDataStream)
		{
			int read = 0;
			PrimitiveTypes::Int32 numGhosts;
			if (m_pNetContext->m_clientId >= 0)
			{
				//PEINFO("\nRecieved on Server/Host from Client: %i", m_pNetContext->m_clientId);

			}
			else
			{
				//PEINFO("\nRecieved on Client in GMan: %i", m_pNetContext->m_clientId);

			}
			//PEINFO("Recieved: ");

			read += StreamManager::ReadInt32(&pDataStream[read], numGhosts);
			//PEINFO("NumGhosts: %i",numGhosts);

			for (int i = 0; i < numGhosts; ++i)
			{
				//Read Status:
				PrimitiveTypes::Int32 gstatus;
				read += StreamManager::ReadInt32(&pDataStream[read], gstatus);

				//Read Type:
				PrimitiveTypes::Int32 gtype;
				read += StreamManager::ReadInt32(&pDataStream[read], gtype);

				//Read mainClientID:
				PrimitiveTypes::Int32 mainClientID;
				read += StreamManager::ReadInt32(&pDataStream[read], mainClientID);

				//Read GhostID
				PrimitiveTypes::Int32 gID;
				read += StreamManager::ReadInt32(&pDataStream[read], gID);

				//Should be unpack!

				//Read StateMask
				PrimitiveTypes::Int32 stateMask;
				read += StreamManager::ReadInt32(&pDataStream[read], stateMask);

				//Read Data
				Matrix4x4 data;
				read += StreamManager::ReadMatrix4x4(&pDataStream[read], data);

				//Receiving on Server
				if (m_pNetContext->m_clientId != -1)
				{
					//When server recieves, update then send all!
					//Update list and dictionary!
					//PEINFO("Received on Server :");

					//Loop through Dictionary and update!
					if (mainClientID == 0)
					{
						//Dont Update.
						//PEINFO("Data from Client0 DONT USE");
					}
					else //Data from Differnet Client received on Server!
					{
						//Add to dictionary!
						if (gstatus == 0)
						{
							if (gtype == 0)
							{
								PEINFO("Create new Player entry to ghostDictionary");
								RootSceneNode::Instance()->createPlayerOnServer(1,gID,mainClientID);

							}
							//Never Receiving any Zombie info from other clients to server!

							
						}
						else if (gstatus == 1)
						{
							PEINFO("Delete/Disable new entry to ghostDictionary");
						}
						else //update the ghostcomponent in the ghostDic!
						{
							if (gtype == 0)
							{
								PEINFO("Update Player entry to ghostDictionary");
								//Since only two players we know player2 is in index 0, if not we would have to iterate through ghostDic
								RootSceneNode::Instance()->ghostDictionary[0]->useMatrix(&data);

							}
							
						}
					}

				}
				
				//Receiving on Client
				//Dont want to update on client 0! Server receiveing does that!
				if ((m_pNetContext->m_clientId == -1) && (RootSceneNode::Instance()->m_networkClientNum != 0) )
				{
					if (mainClientID == RootSceneNode::Instance()->m_networkClientNum)
					{
						PEINFO("Recieved on Client: Data for myself: Do not use!");
					}
					else
					{
						if (gstatus == 0) 
						{
							//PEINFO("Create new entry to ghostDictionary");
							if (gtype == 0)
							{
								PEINFO("Create new Player entry to ghostDictionary");
								RootSceneNode::Instance()->ghostDictionary[RootSceneNode::Instance()->ghostDictionarySize] = RootSceneNode::Instance()->players[0];
								RootSceneNode::Instance()->ghostDictionarySize++;
								RootSceneNode::Instance()->ghostDictionary[0]->useMatrix(&data);

							}
							
						}
						else if (gstatus == 1)
						{
							PEINFO("Delete/Disable new entry to ghostDictionary");
						}
						else
						{
							if (gtype == 0)
							{
								//PEINFO("Update Player to ghostDictionary");
								RootSceneNode::Instance()->ghostDictionary[RootSceneNode::Instance()->zombiesGC_Size]->useMatrix(&data);
							}
							else if (gtype == 1)
							{
								//PEINFO("Update Zom to ghostDictionary");
								//Server Sends Zombies id starting from 1
								RootSceneNode::Instance()->ghostDictionary[gID-1]->useMatrix(&data);

							}

						}
					}
					
				}
			

				
				


				//Received on Server
				if (m_pNetContext->m_clientId != -1)
				{
					//PEINFO("Server Received so it sends to all other connections...");
					ServerNetworkManager *pNM = (ServerNetworkManager *)(m_pContext->getNetworkManager());

					//Send ghostDictionary and ghostUpdateList!
					pNM->scheduleGhostToAllExcept(NULL, m_pContext->getGameObjectManager(), m_pNetContext->m_clientId);
				}
			}
			
			return read;
			
		}

		//If sending a status of 0, change the status to 2 for update!
		void GhostManager::scheduleGhost()
		{
			if (haveGhostsToSend() >= PE_MAX_EVENT_JAM)
			{
				assert(!"Sending too many events have to drop, need throttling mechanism here");
				return;
			}
			if (m_pNetContext->m_clientId >= 0)
			{
				//PEINFO("\Scheduling on Server/Host from Client: %i", m_pNetContext->m_clientId);
			}
			else
			{
				//PEINFO("\Scheduling on Client in GMan: %i", m_pNetContext->m_clientId);
			}

			if (m_pNetContext->m_clientId != -1) //On Server!
			{
				//Send all Ghosts on Server
				for (PrimitiveTypes::UInt32 i = 0; i < RootSceneNode::Instance()->ghostUpdateListSize; i++)
				{
					//Push TransmissionData to Objects to Send

					if (RootSceneNode::Instance()->ghostUpdateList[i]->stateMask != 0)
					{
						m_GhostObjectsToSend.push_back(GhostTransmissionData());
						GhostTransmissionData &back = m_GhostObjectsToSend.back();

						back.gh = RootSceneNode::Instance()->ghostUpdateList[i];
						int dataSize = 0;

						//GC_Status
						dataSize += StreamManager::WriteInt32(back.gh->gstatus, &back.m_payload[dataSize]);

						//gType
						dataSize += StreamManager::WriteInt32(back.gh->gtype, &back.m_payload[dataSize]);

						//Main ClientID
						dataSize += StreamManager::WriteInt32(back.gh->m_mainClientID, &back.m_payload[dataSize]);

						//GhostID
						dataSize += StreamManager::WriteInt32(back.gh->m_ghostID, &back.m_payload[dataSize]);

						//Should call pack!
						//StateMask
						dataSize += StreamManager::WriteInt32(back.gh->stateMask, &back.m_payload[dataSize]);

						//Data
						dataSize += StreamManager::WriteMatrix4x4(back.gh->gm_base, &back.m_payload[dataSize]);

						back.m_size = dataSize;
					}

					

				}

			}

			else //Sending on Client
			{
				for (PrimitiveTypes::UInt32 i = 0; i < RootSceneNode::Instance()->ghostUpdateListSize; i++) // fast array traversal (increasing ptr)
				{
					GhostComponent *temp = RootSceneNode::Instance()->ghostUpdateList[i];

					//Dirty Mask!
					// IF dirty or new data : Send!
					if ((temp->stateMask != 0) || (temp->gstatus == 0))
					{
						
						m_GhostObjectsToSend.push_back(GhostTransmissionData());
						GhostTransmissionData &back = m_GhostObjectsToSend.back();

						back.gh = temp;
						int dataSize = 0;

						//GC_Status
						dataSize += StreamManager::WriteInt32(temp->gstatus, &back.m_payload[dataSize]);
						if (temp->gstatus == 0)
						{
							temp->gstatus = 2; //update from now on!
						}
						//gType
						dataSize += StreamManager::WriteInt32(temp->gtype, &back.m_payload[dataSize]);

						//MainClientID
						dataSize += StreamManager::WriteInt32(temp->m_mainClientID, &back.m_payload[dataSize]);

						//GhostID
						dataSize += StreamManager::WriteInt32(temp->m_ghostID, &back.m_payload[dataSize]);

						// Should call Pack!
						//StateMask
						dataSize += StreamManager::WriteInt32(temp->stateMask, &back.m_payload[dataSize]);

						//Data 
						dataSize += StreamManager::WriteMatrix4x4(temp->gm_base, &back.m_payload[dataSize]);


						back.m_size = dataSize;

					}
				}

			}

		}

		// Send info as new!
		void GhostManager::scheduleListandDic()
		{
			for (PrimitiveTypes::UInt32 i = 0; i < RootSceneNode::Instance()->ghostUpdateListSize; i++) // fast array traversal (increasing ptr)
			{
				//Push TransmissionData to Objects to Send
				m_GhostObjectsToSend.push_back(GhostTransmissionData());
				GhostTransmissionData &back = m_GhostObjectsToSend.back();

				back.gh = RootSceneNode::Instance()->ghostUpdateList[i];
				int dataSize = 0;

				//GC_Status
				dataSize += StreamManager::WriteInt32(0, &back.m_payload[dataSize]);

				//gType
				dataSize += StreamManager::WriteInt32(back.gh->gtype, &back.m_payload[dataSize]);

				//Main ClientID
				dataSize += StreamManager::WriteInt32(back.gh->m_mainClientID, &back.m_payload[dataSize]);

				//GhostID
				dataSize += StreamManager::WriteInt32(back.gh->m_ghostID, &back.m_payload[dataSize]);

				//Should call pack!
				//StateMask
				dataSize += StreamManager::WriteInt32(back.gh->stateMask, &back.m_payload[dataSize]);

				//Data
				dataSize += StreamManager::WriteMatrix4x4(back.gh->gm_base, &back.m_payload[dataSize]);

				back.m_size = dataSize;

			}

			for (PrimitiveTypes::UInt32 i = 0; i < RootSceneNode::Instance()->ghostDictionarySize; i++) // fast array traversal (increasing ptr)
			{
				//Push TransmissionData to Objects to Send
				m_GhostObjectsToSend.push_back(GhostTransmissionData());
				GhostTransmissionData &back = m_GhostObjectsToSend.back();

				back.gh = RootSceneNode::Instance()->ghostDictionary[i];
				int dataSize = 0;

				//GC_Status
				dataSize += StreamManager::WriteInt32(0, &back.m_payload[dataSize]);

				//gType
				dataSize += StreamManager::WriteInt32(back.gh->gtype, &back.m_payload[dataSize]);

				//Main ClientID
				dataSize += StreamManager::WriteInt32(back.gh->m_mainClientID, &back.m_payload[dataSize]);

				//GhostID
				dataSize += StreamManager::WriteInt32(back.gh->m_ghostID, &back.m_payload[dataSize]);

				//Should call pack!
				//StateMask
				dataSize += StreamManager::WriteInt32(back.gh->stateMask, &back.m_payload[dataSize]);

				//Data
				dataSize += StreamManager::WriteMatrix4x4(back.gh->gm_base, &back.m_payload[dataSize]);

				back.m_size = dataSize;

			}
		
		}

		void GhostManager::do_UPDATE(PE::Events::Event *pEvt)
		{
			PE::Events::Event_UPDATE *pRealEvt = (PE::Events::Event_UPDATE *)(pEvt);
			static float inc = 0;
			int delayTime = 0.0f;
			inc += pRealEvt->m_frameTime;
			if (inc >= delayTime)
			{
				if (m_pNetContext->m_clientId != -1) //On Server!
				{
					//PEINFO("GhostUpdateServer");

				}
				else
				{
					//PEINFO("GhostUpdateClient");
					scheduleGhost();
				}
				inc = 0;
			}

		}

		

	}; // namespace Components
}; // namespace PE