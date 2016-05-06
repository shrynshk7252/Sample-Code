#ifndef _PE_PLAYABLE_CHARACTER_BEHAVIOR_SM_H_
#define _PE_PLAYABLE_CHARACTER_BEHAVIOR_SM_H_


#include "PrimeEngine/Events/Component.h"

#include "CharacterControl/Events/Events.h"

namespace CharacterControl{

namespace Components {

// movement state machine talks to associated animation state machine
	struct PlayableCharacterBehaviorSM : public PE::Components::Component
{
	PE_DECLARE_CLASS(PlayableCharacterBehaviorSM);
	
	enum States
	{
		IDLE, // stand in place
		WAITING_FOR_WAYPOINT, // have a name of waypoint to go to, but it has not been loaded yet
		PATROLLING_WAYPOINTS,
		SHOOTING
	};


	PlayableCharacterBehaviorSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, PE::Handle hMovementSM);

	void start();

	//////////////////////////////////////////////////////////////////////////
	// Component API and Event handlers
	//////////////////////////////////////////////////////////////////////////
	//
	virtual void addDefaultComponents();
	//
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayableCharacterMovementSM_Event_TARGET_REACHED)
	virtual void do_PlayableCharacterMovementSM_Event_TARGET_REACHED(PE::Events::Event *pEvt);
	//
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE)
	virtual void do_UPDATE(PE::Events::Event *pEvt);

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PRE_RENDER_needsRC)
	void do_PRE_RENDER_needsRC(PE::Events::Event *pEvt);

	PE::Handle m_hMovementSM;

	bool m_havePatrolWayPoint;
	char m_curPatrolWayPoint[32];
	float time;
	States m_state;
	bool active;
	bool startTankTimer;
	
};

};
};


#endif


