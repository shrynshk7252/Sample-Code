#ifndef __PrimeEngineGhostManager_H__
#define __PrimeEngineGhostManager_H__

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>
#include <vector>
#include <deque>

// Inter-Engine includes

#include "../Events/Component.h"

extern "C"
{
#include "../../luasocket_dist/src/socket.h"
};

#include "PrimeEngine/Networking/NetworkContext.h"
#include "PrimeEngine/Utils/Networkable.h"
#include "PrimeEngine/Math/Matrix4x4.h"

// Sibling/Children includes
#include "Packet.h"


// Sibling/Children includes

namespace PE{
	namespace Components {

		struct GhostManager : public Component
		{
			static const int PE_EVENT_SLIDING_WINDOW = 64;

			PE_DECLARE_CLASS(GhostManager);

			// Constructor -------------------------------------------------------------
			// same
			GhostManager(PE::GameContext &context, PE::MemoryArena arena, PE::NetworkContext &netContext, Handle hMyself);

			virtual ~GhostManager(){}

			// component
			virtual void addDefaultComponents();

			// Methods -------------------------------------------------------
			/// called by stream manager to see how many events to send
			int haveGhostsToSend();

			/// called by gameplay code to schedule event transmission to client(s)
			void scheduleGhost();
			void scheduleListandDic();

			/// called by StreamManager to put queued up ghostObjects in packet
			int fillInNextPacket(char *pDataStream, TransmissionRecord *pRecord, int packetSizeAllocated, bool &out_usefulDataSent, bool &out_wantToSendMore);

			int receiveNextPacket(char *pDataStream);
			// Individual events -------------------------------------------------------

			// this method will set up some global gpu constants like game time, frame time
			// it will also set light source gpu constants

			std::deque<GhostTransmissionData> m_GhostObjectsToSend;
			
			// transmitter
			int m_transmitterNextGhostOrderId;
			int m_transmitterNumGhostsNotAcked; //= number of ghost objects stored in TransmissionRecords

			// receiver
			int m_receiverFirstGhostOrderId; // evtOrderId of first element in m_receivedEvents
			
			GhostReceptionData m_receivedGhosts[PE_EVENT_SLIDING_WINDOW];
			
			PE::NetworkContext *m_pNetContext;

			
		public:
			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
			virtual void do_UPDATE(PE::Events::Event *pEvt);

		};

	}; // namespace Components
}; // namespace PE

#endif // file guard
