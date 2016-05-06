#ifndef _CHARACTER_CONTROL_PLAYABLE_CHARACTER_
#define _CHARACTER_CONTROL_PLAYABLE_CHARACTER_

#include "PrimeEngine/Events/Component.h"
#include "CharacterControl/Events/Events.h"
#include "PrimeEngine/Scene/TextSceneNode.h"
#include "PrimeEngine/UISystem.h"
#include "PrimeEngine/Scene/SceneNode.h"

#include "PrimeEngine/Scene/RootSceneNode.h"
#include "PrimeEngine/Networking/GhostComponent.h"

//#include "PrimeEngine/Scene/SceneNode.h"

namespace PE {
	namespace Events{
		struct EventQueueManager;
	}
}

namespace CharacterControl{
namespace Components {

	enum PlayerMorphObject{
		Human,
		Tank
	};

	struct PlayableCharacterGameControls : public PE::Components::Component 
	{
		PE_DECLARE_CLASS(PlayableCharacterGameControls);
	public:

		PlayableCharacterGameControls(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself)
			: PE::Components::Component(context, arena, hMyself)
		{
			}

		virtual ~PlayableCharacterGameControls(){}
		// Component ------------------------------------------------------------

		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
		virtual void do_UPDATE(PE::Events::Event *pEvt);

		virtual void addDefaultComponents();

		//Methods----------------
		void handleKeyboardDebugInputEvents(PE::Events::Event *pEvt);
		void handleControllerDebugInputEvents(PE::Events::Event *pEvt);
		
		PE::Events::EventQueueManager *m_pQueueManager;

		PrimitiveTypes::Float32 m_frameTime;
	};



struct PlayableCharacter : /*public PE::Components::Component,*/  public PE::Components::GhostComponent
{
	PE_DECLARE_CLASS(PlayableCharacter);

	PlayableCharacter(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, Events::Event_CreatePlayableCharacter *pEvt,float networkPingInterval);
	
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayableCharacter_Throttle);
	virtual void do_PlayableCharacter_Throttle(PE::Events::Event *pEvt);

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayableCharacter_Turn);
	virtual void do_PlayableCharacter_Turn(PE::Events::Event *pEvt);
	
	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayableCharacter_Revive);
	virtual void do_PlayableCharacter_Revive(PE::Events::Event *pEvt);

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PlayableCharacter_Shoot);
	virtual void do_PlayableCharacter_Shoot(PE::Events::Event *pEvt);

	PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
    virtual void do_UPDATE(PE::Events::Event *pEvt);

	virtual void addDefaultComponents();
	void overrideTransform(Matrix4x4 &t);
	void activate();
	void ar();
	void shotgun();
	virtual void useMatrix(Matrix4x4 *m_b);


	// Tank and other morph objects
	void turnInto(PlayerMorphObject obj);
	PlayerMorphObject playerMorphState;
	float maxTankTimer;
	float currTankTimer;


	char m_name[32];
	bool m_active;
	float m_time;
	bool m_overriden;
	Matrix4x4 m_transformOverride;
	Matrix4x4 p_base;
	float m_networkPingTimer;
	float m_networkPingInterval;
	bool m_activate;
	float time;

	// Total Seconds pressing Revive
	bool m_playerCanRevive;

	// Player Health
	float m_playerHealth;
	bool m_playerDown;
	float m_playerReviveTime;
	float m_currReviveTime;
	bool new_gun;
	float cd_time;
	void changeHealthBy(float amount);
	int kills;

	PE::Components::SceneNode* tankSN;
	PE::Components::SceneNode* playerSN;

	float damage[4];// = {100,60,30};
	float fr[4];
	float range[4];
	bool cantakedamage;
	bool isReady;
};
}; // namespace Components
}; // namespace CharacterControl
#endif