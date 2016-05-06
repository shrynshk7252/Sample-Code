#ifndef __PrimeEngineGhostTransmissionData_H__
#define __PrimeEngineGhostTransmissionData_H__

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
#include "PrimeEngine/Networking/GhostComponent.h"

#include "PrimeEngine/Utils/Networkable.h"
#include "Packet.h"

// Sibling/Children includes


namespace PE {
	namespace Components
	{
		struct Component;
	};
	namespace Events
	{
		struct Event;
	};

	struct GhostTransmissionData
	{
		bool m_isGuaranteed;
		int m_size;
		int m_orderId;
		int m_StateMask;
		char m_payload[PE_MAX_EVENT_PAYLOAD];
		PE::Components::GhostComponent *gh;
		
	};

	struct GhostReceptionData
	{
		Components::Component *m_pTargetComponent;
	};

}; // namespace PE
#endif
