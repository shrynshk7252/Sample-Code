#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/SceneNode.h"
#include "PrimeEngine/Scene/RootSceneNode.h"
#include "PlayableCharacterAnimationSM.h"
#include "PlayableCharacter.h"

using namespace PE::Components;
using namespace PE::Events;

namespace CharacterControl{

namespace Events{
PE_IMPLEMENT_CLASS1(PlayableCharacterAnimSM_Event_STOP, Event);
PE_IMPLEMENT_CLASS1(PlayableCharacterAnimSM_Event_WALK, Event);
PE_IMPLEMENT_CLASS1(PlayableCharacterAnimSM_Event_PUNCH, Event);
PE_IMPLEMENT_CLASS1(PlayableCharacterAnimSM_Event_RUN, Event);
PE_IMPLEMENT_CLASS1(PlayableCharacterAnimSM_Event_HITREACTION, Event);
PE_IMPLEMENT_CLASS1(PlayableCharacterAnimSM_Event_HITREACTION2, Event);
PE_IMPLEMENT_CLASS1(PlayableCharacterAnimSM_Event_JUMP, Event);
PE_IMPLEMENT_CLASS1(PlayableCharacterAnimSM_Event_KICK, Event);
}
namespace Components{

PE_IMPLEMENT_CLASS1(PlayableCharacterAnimationSM, DefaultAnimationSM);

PlayableCharacterAnimationSM::PlayableCharacterAnimationSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself) : DefaultAnimationSM(context, arena, hMyself)
{
	m_curId = NONE;
}

void PlayableCharacterAnimationSM::addDefaultComponents()
{
	DefaultAnimationSM::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(Events::PlayableCharacterAnimSM_Event_STOP, PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_STOP);
	PE_REGISTER_EVENT_HANDLER(Events::PlayableCharacterAnimSM_Event_WALK, PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_WALK);
	PE_REGISTER_EVENT_HANDLER(Events::PlayableCharacterAnimSM_Event_PUNCH, PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_PUNCH);
	PE_REGISTER_EVENT_HANDLER(Events::PlayableCharacterAnimSM_Event_RUN, PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_RUN);
	PE_REGISTER_EVENT_HANDLER(Events::PlayableCharacterAnimSM_Event_JUMP, PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_JUMP);
	PE_REGISTER_EVENT_HANDLER(Events::PlayableCharacterAnimSM_Event_KICK, PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_KICK);
	PE_REGISTER_EVENT_HANDLER(Events::PlayableCharacterAnimSM_Event_HITREACTION, PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_HITREACTION);
	
	PE_REGISTER_EVENT_HANDLER(Events::PlayableCharacterAnimSM_Event_HITREACTION2, PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_HITREACTION2);

}

void PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_STOP(PE::Events::Event *pEvt)
{
	m_curId = PlayableCharacterAnimationSM::STAND;
	setAnimation(0, 0, 0, 0, 1, 1, PE::LOOPING,1);
	//setAnimation(1, 0, 0, 1, 0, 1, PE::LOOPING,1);
}

void PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_WALK(PE::Events::Event *pEvt)
{
	if (m_curId != PlayableCharacterAnimationSM::WALK)
	{
		m_curId = PlayableCharacterAnimationSM::WALK;
		setAnimation(1, 0, 0, 0, 0, 0, PE::ACTIVE|PE::LOOPING,50);
	}
}

void PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_PUNCH(PE::Events::Event *pEvt)
{	
	if (m_curId != PlayableCharacterAnimationSM::STAND_AIM)
	{
		m_curId = PlayableCharacterAnimationSM::STAND_AIM;
		setAnimation(1, 0, 0, 0, 0, 2, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,.5,NULL,true,0,1,69);
		setAnimation(2, 0, 0, 1, 0, 2, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,1,NULL,true,0,70,81);
	}
}

void PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_RUN(PE::Events::Event *pEvt)
{
	if (m_curId != PlayableCharacterAnimationSM::RUN)
	{
		m_curId = PlayableCharacterAnimationSM::RUN;
		if(RootSceneNode::Instance()->PSN1->m_ispunching==true)
		{
			setAnimation(5, 0, 0, 3, 4, 7, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,.5,NULL,true,0,1,69);
			setAnimation(3, 0, 0, 3, 4, 7, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,1,NULL,true,0,70,81);
		}
		else
		{
			setAnimation(0, 0, 0, 3, 4, 7, PE::LOOPING,.5);
			setAnimation(3, 0, 0, 3, 4, 7, PE::LOOPING,1,NULL,true);
		}
		
	}
}

void PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_HITREACTION(PE::Events::Event *pEvt)
{
	if (m_curId != PlayableCharacterAnimationSM::STAND_SHOOT)
	{
		m_curId = PlayableCharacterAnimationSM::STAND_SHOOT;
		
		if(RootSceneNode::Instance()->PSN1->m_ismoving==true)
		{
			if(RootSceneNode::Instance()->PSN1->m_isrunning==true)
			{
				setAnimation(4, 0, 0, 3, 4, 7, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,.5,NULL,true,0,1,69);
				setAnimation(3, 0, 0, 3, 4, 7, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,1,NULL,true,0,70,81);
				/*setAnimation(4, 0, 0, 1, 0, 0, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,.5,NULL,true,0,1,21);
				setAnimation(4, 0, 1, 2, 0, 0, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,.5,NULL,true,0,22,45);
				setAnimation(4, 0, 2, 3, 0, 0, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,.5,NULL,true,0,46,69);
				setAnimation(3, 0, 3, 4, 0, 0, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,1,NULL,true,0,70,81);*/
			}
			else
			{
				setAnimation(4, 0, 0, 3, 4, 7, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,.5,NULL,true,0,1,69);
				setAnimation(0, 0, 0, 3, 4, 7, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,1,NULL,true,0,70,81);
			}
		}
		else
		{
			//setAnimation(1, 0, 0, 3, 4, 7, PE::LOOPING,0.5);
			setAnimation(4, 0, 0, 3, 4, 7, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,1,NULL,true,0,0,81);
			
			setAnimation(4, 0, 0, 3, 4, 7, PE::LOOPING|PE::FADING_AWAY,1,NULL,true);
		}
	}
}

void PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_HITREACTION2(PE::Events::Event *pEvt)
{
	if (m_curId != PlayableCharacterAnimationSM::STAND_SHOOT)
	{
		m_curId = PlayableCharacterAnimationSM::STAND_SHOOT;
		
		setAnimation(1, 0, 0, 3, 4, 7, PE::LOOPING,0);
		setAnimation(4, 0, 0, 3, 4, 7, PE::FADING_AWAY,1);
		if(RootSceneNode::Instance()->PSN2->m_health>=5)
			RootSceneNode::Instance()->PSN2->m_health-=5;
	}
}


void PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_JUMP(PE::Events::Event *pEvt)
{
	if (m_curId != PlayableCharacterAnimationSM::JUMP)
	{
		m_curId = PlayableCharacterAnimationSM::JUMP;
		//setAnimation(1, 0, 0, 3, 4, 7, PE::LOOPING,0);
		//setAnimation(6, 0, 0, 3, 4, 7, PE::FADING_AWAY,1);
		
			setAnimation(6, 0, 0, 3, 4, 7, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,1,NULL,true,0,0,81);
			
			setAnimation(6, 0, 0, 3, 4, 7, PE::LOOPING,1,NULL,true);
	}
}

void PlayableCharacterAnimationSM::do_PlayableCharacterAnimSM_Event_KICK(PE::Events::Event *pEvt)
{
	if (m_curId != PlayableCharacterAnimationSM::KICK)
	{
		m_curId = PlayableCharacterAnimationSM::KICK;
		int chance=rand()%2;
		if(chance==0)
		{
			//setAnimation(1, 0, 0, 3, 4, 7, PE::LOOPING,0);
			setAnimation(7, 0, 0, 3, 4, 7, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,1,NULL,true,0,0,81);
			setAnimation(7, 0, 0, 3, 4, 7, PE::LOOPING|PE::FADING_AWAY,1,NULL,true);
		}
		else
		{
			setAnimation(4, 0, 0, 3, 4, 7, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION,.75,NULL,true,0,1,69);
			setAnimation(7, 0, 0, 3, 4, 7, PE::LOOPING|PE::PARTIAL_BODY_ANIMATION|PE::FADING_AWAY,1,NULL,true,0,70,81);

		}
	}
}


}
}




