#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"

#include "PlayableCharacterMovementSM.h"
#include "PlayableCharacterAnimationSM.h"
#include "PlayableCharacter.h"
#include "PrimeEngine/Scene/RootSceneNode.h"


using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

namespace CharacterControl{

// Events sent by behavior state machine (or other high level state machines)
// these are events that specify where a soldier should move
namespace Events{

PE_IMPLEMENT_CLASS1(PlayableCharacterMovementSM_Event_MOVE_TO, Event);

PlayableCharacterMovementSM_Event_MOVE_TO::PlayableCharacterMovementSM_Event_MOVE_TO(Vector3 targetPos /* = Vector3 */)
: m_targetPosition(targetPos), m_running(false)
{ }

PE_IMPLEMENT_CLASS1(PlayableCharacterMovementSM_Event_STOP, Event);


PE_IMPLEMENT_CLASS1(PlayableCharacterMovementSM_Event_TARGET_REACHED, Event);
}

namespace Components{

PE_IMPLEMENT_CLASS1(PlayableCharacterMovementSM, Component);


PlayableCharacterMovementSM::PlayableCharacterMovementSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself) 
: Component(context, arena, hMyself)
, m_state(STANDING)
{}

SceneNode *PlayableCharacterMovementSM::getParentsSceneNode()
{
	PE::Handle hParent = getFirstParentByType<Component>();
	if (hParent.isValid())
	{
		// see if parent has scene node component
		return hParent.getObject<Component>()->getFirstComponent<SceneNode>();
		
	}
	return NULL;
}

void PlayableCharacterMovementSM::addDefaultComponents()
{
	Component::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(PlayableCharacterMovementSM_Event_MOVE_TO, PlayableCharacterMovementSM::do_PlayableCharacterMovementSM_Event_MOVE_TO);
	PE_REGISTER_EVENT_HANDLER(PlayableCharacterMovementSM_Event_STOP, PlayableCharacterMovementSM::do_PlayableCharacterMovementSM_Event_STOP);
	PE_REGISTER_EVENT_HANDLER(Event_UPDATE, PlayableCharacterMovementSM::do_UPDATE);
}

void PlayableCharacterMovementSM::do_PlayableCharacterMovementSM_Event_MOVE_TO(PE::Events::Event *pEvt)
{
	PlayableCharacterMovementSM_Event_MOVE_TO *pRealEvt = (PlayableCharacterMovementSM_Event_MOVE_TO *)(pEvt);
	
	// change state of this state machine
	m_state = WALKING_TO_TARGET;
	m_targetPostion = pRealEvt->m_targetPosition;

}

void PlayableCharacterMovementSM::do_PlayableCharacterMovementSM_Event_STOP(PE::Events::Event *pEvt)
{
	/*Events::TargetAnimSM_Event_STOP Evt;

	Target *pSol = getFirstParentByTypePtr<Target>();
	pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&Evt);*/
}
float time=0;
void PlayableCharacterMovementSM::do_UPDATE(PE::Events::Event *pEvt)
{
	PE::Events::Event_UPDATE *pRealEvt = (PE::Events::Event_UPDATE *)(pEvt);
	time+=pRealEvt->m_frameTime;
	SceneNode *pSN1 = getParentsSceneNode();
	/*if(time<1 && !pSN1->m_client)
	{
		if(pSN1->m_state==1)
		{
			PE::Handle h("PlayableCharacterAnimSM_Event_WALK", sizeof(PlayableCharacterAnimSM_Event_WALK));
			Events::PlayableCharacterAnimSM_Event_WALK *pOutEvt = new(h) PlayableCharacterAnimSM_Event_WALK();
	
			PlayableCharacter *pSol = getFirstParentByTypePtr<PlayableCharacter>();
			pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

			h.release();
		}
		else if(pSN1->m_state==0)
		{
			PE::Handle h("PlayableCharacterAnimSM_Event_STOP", sizeof(PlayableCharacterAnimSM_Event_STOP));
			Events::PlayableCharacterAnimSM_Event_STOP *pOutEvt = new(h) PlayableCharacterAnimSM_Event_STOP();
	
			PlayableCharacter *pSol = getFirstParentByTypePtr<PlayableCharacter>();
			pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

			h.release();
		}
		return;
	}
	time=0;*/
	if(pSN1->m_state==1 && pSN1->m_client)
	{
		PE::Handle h("PlayableCharacterAnimSM_Event_WALK", sizeof(PlayableCharacterAnimSM_Event_WALK));
		Events::PlayableCharacterAnimSM_Event_WALK *pOutEvt = new(h) PlayableCharacterAnimSM_Event_WALK();
	
		PlayableCharacter *pSol = getFirstParentByTypePtr<PlayableCharacter>();
		pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

		// release memory now that event is processed
		h.release();
		pSN1->m_state=0;
	}
	else if(pSN1->m_state==0 && pSN1->m_client)
	{
		PE::Handle h("PlayableCharacterAnimSM_Event_STOP", sizeof(PlayableCharacterAnimSM_Event_STOP));
		Events::PlayableCharacterAnimSM_Event_STOP *pOutEvt = new(h) PlayableCharacterAnimSM_Event_STOP();
	
		PlayableCharacter *pSol = getFirstParentByTypePtr<PlayableCharacter>();
		pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

		// release memory now that event is processed
		h.release();
		pSN1->m_state=0;
		
		pSN1->m_ismoving=false;
		pSN1->m_isrunning=false;
	}
	if(pSN1->m_state==1 && !pSN1->m_client)
	{
		PE::Handle h("PlayableCharacterAnimSM_Event_WALK", sizeof(PlayableCharacterAnimSM_Event_WALK));
		Events::PlayableCharacterAnimSM_Event_WALK *pOutEvt = new(h) PlayableCharacterAnimSM_Event_WALK();
	
		PlayableCharacter *pSol = getFirstParentByTypePtr<PlayableCharacter>();
		pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

		// release memory now that event is processed
		h.release();
		//pSN1->m_state=0;
	}
	else if(pSN1->m_state==0 && !pSN1->m_client)
	{
		PE::Handle h("PlayableCharacterAnimSM_Event_STOP", sizeof(PlayableCharacterAnimSM_Event_STOP));
		Events::PlayableCharacterAnimSM_Event_STOP *pOutEvt = new(h) PlayableCharacterAnimSM_Event_STOP();
	
		PlayableCharacter *pSol = getFirstParentByTypePtr<PlayableCharacter>();
		pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);

		// release memory now that event is processed
		h.release();
		pSN1->m_state=0;
		
		pSN1->m_ismoving=false;
		pSN1->m_isrunning=false;
	}

	if (m_state == WALKING_TO_TARGET )
	{
		// see if parent has scene node component
		SceneNode *pSN = getParentsSceneNode();
		
		if (pSN)
		{
			Vector3 curPos = pSN->m_base.getPos();
			float dsqr = (m_targetPostion - curPos).lengthSqr();

			bool reached = true;
			if (dsqr > 0.01f)
			{
				// not at the spot yet
				Event_UPDATE *pRealEvt = (Event_UPDATE *)(pEvt);
				float speed = (m_state == WALKING_TO_TARGET ? 1.4f : 3.0f );
				
				//static float speed =  3.0f ;
				
				float allowedDisp = speed * pRealEvt->m_frameTime;

				Vector3 dir = (m_targetPostion - curPos);
				dir.normalize();
				float dist = sqrt(dsqr);
				if (dist > allowedDisp)
				{
					dist = allowedDisp; // can move up to allowedDisp
					reached = false; // not reaching destination yet
				}

				// instantaneous turn
				pSN->m_base.turnInDirection(dir, 3.1415f);
				pSN->m_base.setPos(curPos + dir * dist);
			}

			if (reached)
			{	
				// target has been reached. need to notify all same level state machines (components of parent)
				m_state = STANDING;
				{
					PE::Handle h("PlayableCharacterMovementSM_Event_TARGET_REACHED", sizeof(PlayableCharacterMovementSM_Event_TARGET_REACHED));
					Events::PlayableCharacterMovementSM_Event_TARGET_REACHED *pOutEvt = new(h) PlayableCharacterMovementSM_Event_TARGET_REACHED();

					PE::Handle hParent = getFirstParentByType<Component>();
					if (hParent.isValid())
					{
						hParent.getObject<Component>()->handleEvent(pOutEvt);
					}
					
					// release memory now that event is processed
					h.release();
				}

				if (m_state == STANDING)
				{
					// no one has modified our state based on TARGET_REACHED callback
					// this means we are not going anywhere right now
					// so can send event to animation state machine to stop
					{
						/*Events::TargetAnimSM_Event_STOP evt;
						
						Target *pSol = getFirstParentByTypePtr<Target>();
						pSol->getFirstComponent<PE::Components::SceneNode>()->handleEvent(&evt);*/
					}
				}
			}
		}
	}
}

}}




