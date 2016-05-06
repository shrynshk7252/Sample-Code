#ifndef _PE_PLAYABLE_CHARACTER_ANIMATION_SM_H_
#define _PE_PLAYABLE_CHARACTER_ANIMATION_SM_H_


#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Scene/DefaultAnimationSM.h"


#include "CharacterControl/Events/Events.h"

namespace CharacterControl{

// events that can be sent to this state machine
namespace Events
{

// sent by movement state machine when a soldier has to stop
struct PlayableCharacterAnimSM_Event_STOP : public PE::Events::Event {
	PE_DECLARE_CLASS(PlayableCharacterAnimSM_Event_STOP);

	PlayableCharacterAnimSM_Event_STOP() {}
};

// sent by movement state machine when a soldier has to walk
struct PlayableCharacterAnimSM_Event_WALK : public PE::Events::Event {
	PE_DECLARE_CLASS(PlayableCharacterAnimSM_Event_WALK);

	PlayableCharacterAnimSM_Event_WALK() {}
};

// todo add events for shooting (sent by weapons state machine
struct PlayableCharacterAnimSM_Event_PUNCH : public PE::Events::Event {
	PE_DECLARE_CLASS(PlayableCharacterAnimSM_Event_PUNCH);

	PlayableCharacterAnimSM_Event_PUNCH() {}
};

struct PlayableCharacterAnimSM_Event_RUN : public PE::Events::Event {
	PE_DECLARE_CLASS(PlayableCharacterAnimSM_Event_RUN);

	PlayableCharacterAnimSM_Event_RUN() {}
};

struct PlayableCharacterAnimSM_Event_JUMP : public PE::Events::Event {
	PE_DECLARE_CLASS(PlayableCharacterAnimSM_Event_JUMP);

	PlayableCharacterAnimSM_Event_JUMP() {}
};

struct PlayableCharacterAnimSM_Event_KICK : public PE::Events::Event {
	PE_DECLARE_CLASS(PlayableCharacterAnimSM_Event_KICK);

	PlayableCharacterAnimSM_Event_KICK() {}
};

struct PlayableCharacterAnimSM_Event_HITREACTION : public PE::Events::Event {
	PE_DECLARE_CLASS(PlayableCharacterAnimSM_Event_HITREACTION);

	PlayableCharacterAnimSM_Event_HITREACTION() {}
};
struct PlayableCharacterAnimSM_Event_HITREACTION2 : public PE::Events::Event {
	PE_DECLARE_CLASS(PlayableCharacterAnimSM_Event_HITREACTION2);

	PlayableCharacterAnimSM_Event_HITREACTION2() {}
};

};

namespace Components {

struct PlayableCharacterAnimationSM : public PE::Components::DefaultAnimationSM
{
	PE_DECLARE_CLASS(PlayableCharacterAnimationSM);
	
	enum AnimId
	{
		NONE = -1,
		STAND = 0,
		WALK = 1,
		RUN = 0,
		JUMP = 0,
		KICK = 0,
		STAND_AIM = 0,
		STAND_SHOOT = 0,
	};

	PlayableCharacterAnimationSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself);

	// event handling
	virtual void addDefaultComponents();
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayableCharacterAnimSM_Event_STOP)
	virtual void do_PlayableCharacterAnimSM_Event_STOP(PE::Events::Event *pEvt);
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayableCharacterAnimSM_Event_WALK)
	virtual void do_PlayableCharacterAnimSM_Event_WALK(PE::Events::Event *pEvt);
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayableCharacterAnimSM_Event_PUNCH)
	virtual void do_PlayableCharacterAnimSM_Event_PUNCH(PE::Events::Event *pEvt);
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayableCharacterAnimSM_Event_RUN)
	virtual void do_PlayableCharacterAnimSM_Event_RUN(PE::Events::Event *pEvt);
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayableCharacterAnimSM_Event_HITREACTION)
	virtual void do_PlayableCharacterAnimSM_Event_HITREACTION(PE::Events::Event *pEvt);
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayableCharacterAnimSM_Event_HITREACTION2)
	virtual void do_PlayableCharacterAnimSM_Event_HITREACTION2(PE::Events::Event *pEvt);
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayableCharacterAnimSM_Event_JUMP)
	virtual void do_PlayableCharacterAnimSM_Event_JUMP(PE::Events::Event *pEvt);
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayableCharacterAnimSM_Event_KICK)
	virtual void do_PlayableCharacterAnimSM_Event_KICK(PE::Events::Event *pEvt);


	AnimId m_curId;
};

};
};


#endif


