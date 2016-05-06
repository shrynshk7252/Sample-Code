#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/SkeletonInstance.h"
#include "PrimeEngine/Scene/Mesh.h"
#include "PrimeEngine/Scene/MeshInstance.h"

#include "PrimeEngine/Networking/EventManager.h"
#include "PrimeEngine/Networking/GhostManager.h"
#include "PrimeEngine/Networking/Client/ClientNetworkManager.h"
#include "PlayableCharacter.h"
#include "PlayableCharacterMovementSM.h"

#include "PrimeEngine/Scene/PhysicsComponent.h"
#include "PrimeEngine/Scene/PhysicsManager.h"

#include "PlayableCharacterAnimationSM.h"
#include "PlayableCharacterBehaviorSM.h"
#include "ZombiePrisonerBehaviorSM.h"

#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Events/StandardKeyboardEvents.h"
#include "PrimeEngine/Events/StandardIOSEvents.h"
#include "PrimeEngine/Events/StandardGameEvents.h"
#include "PrimeEngine/Events/EventQueueManager.h"
#include "PrimeEngine/Events/StandardControllerEvents.h"
#include "PrimeEngine/GameObjectModel/DefaultGameControls/DefaultGameControls.h"
#include "CharacterControl/CharacterControlContext.h"
#include "CharacterControl/Events/Events.h"
#include "PrimeEngine/GameObjectModel/GameObjectManager.h"

#include "PrimeEngine/Particles/Particles/ParticleManager.h"
#include "PrimeEngine/Particles/Particles/ParticleEmitter.h"

//
#include "PrimeEngine/Particles/Particles/EnergyParticle.h"
#include "PrimeEngine/Particles/Particles/SmokeParticle.h"
#include "PrimeEngine/Particles/Particles/FireballParticle.h"
#include "PrimeEngine/Particles/Particles/ExplosionParticle.h"

using namespace PE;
using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

// Arkane Control Values
#define Analog_To_Digital_Trigger_Distance 0.5f
static float Debug_Fly_Speed = 8.0f; //Units per second
#define Debug_Rotate_Speed 2.0f //Radians per second
#define Player_Keyboard_Rotate_Speed 20.0f //Radians per second

namespace CharacterControl{
namespace Components {

	PE_IMPLEMENT_CLASS1(PlayableCharacterGameControls, PE::Components::Component);

	void PlayableCharacterGameControls::addDefaultComponents()
	{
		Component::addDefaultComponents();

		PE_REGISTER_EVENT_HANDLER(Event_UPDATE, PlayableCharacterGameControls::do_UPDATE);
	}

	void PlayableCharacterGameControls::do_UPDATE(PE::Events::Event *pEvt)
	{
		// Process input events (XBOX360 buttons, triggers...)
		PE::Handle iqh = PE::Events::EventQueueManager::Instance()->getEventQueueHandle("input");

		// Process input event -> game event conversion
		while (!iqh.getObject<PE::Events::EventQueue>()->empty())
		{
			PE::Events::Event *pInputEvt = iqh.getObject<PE::Events::EventQueue>()->getFront();
			m_frameTime = ((Event_UPDATE*)(pEvt))->m_frameTime;
			// Have DefaultGameControls translate the input event to GameEvents
			handleKeyboardDebugInputEvents(pInputEvt);
			handleControllerDebugInputEvents(pInputEvt);

			iqh.getObject<PE::Events::EventQueue>()->destroyFront();
		}

		// Events are destoryed by destroyFront() but this is called every frame just in case
		iqh.getObject<PE::Events::EventQueue>()->destroy();
	}

	void PlayableCharacterGameControls::handleKeyboardDebugInputEvents(Event *pEvt)
	{
		m_pQueueManager = PE::Events::EventQueueManager::Instance();
		
		if (PE::Events::Event_KEY_A_HELD::GetClassId() == pEvt->getClassId())
		{
						

			PE::Handle h("EVENT", sizeof(Events::Event_PlayableCharacter_Throttle));
			Events::Event_PlayableCharacter_Throttle *flyCameraEvt = new(h) Events::Event_PlayableCharacter_Throttle ;
		
			Vector3 relativeMovement(-1.0f,0.0f,0.0f);
			flyCameraEvt->m_relativeMove = relativeMovement * Debug_Fly_Speed * m_frameTime;
			m_pQueueManager->add(h, QT_GENERAL);

		}
		else if (Event_KEY_S_HELD::GetClassId() == pEvt->getClassId())
		{
			PE::Handle h("EVENT", sizeof(Events::Event_PlayableCharacter_Throttle));
			Events::Event_PlayableCharacter_Throttle *flyCameraEvt = new(h) Events::Event_PlayableCharacter_Throttle ;
		
			Vector3 relativeMovement(0.0f,0.0f,-1.0f);
			flyCameraEvt->m_relativeMove = relativeMovement * Debug_Fly_Speed * m_frameTime;
			m_pQueueManager->add(h, QT_GENERAL);
		}
		
		else if (Event_KEY_D_HELD::GetClassId() == pEvt->getClassId())
		{
			PE::Handle h("EVENT", sizeof(Events::Event_PlayableCharacter_Throttle));
			Events::Event_PlayableCharacter_Throttle *flyCameraEvt = new(h) Events::Event_PlayableCharacter_Throttle ;
		
			Vector3 relativeMovement(1.0f,0.0f,0.0f);
			flyCameraEvt->m_relativeMove = relativeMovement * Debug_Fly_Speed * m_frameTime;
			m_pQueueManager->add(h, QT_GENERAL);
		}
		else if (Event_KEY_W_HELD::GetClassId() == pEvt->getClassId())
		{
			PE::Handle h("EVENT", sizeof(Events::Event_PlayableCharacter_Throttle));
			Events::Event_PlayableCharacter_Throttle *flyCameraEvt = new(h) Events::Event_PlayableCharacter_Throttle ;
		
			Vector3 relativeMovement(0.0f,0.0f,1.0f);
			flyCameraEvt->m_relativeMove = relativeMovement * Debug_Fly_Speed * m_frameTime;
			m_pQueueManager->add(h, QT_GENERAL);
			
		}
		else if (Event_KEY_L_HELD::GetClassId() == pEvt->getClassId())
		{
			if(RootSceneNode::Instance()->PSN1->m_client)
			{
				RootSceneNode::Instance()->PSN1->weap_type++;
				RootSceneNode::Instance()->PSN1->weap_type=RootSceneNode::Instance()->PSN1->weap_type%2;
			}
			else
			{
				RootSceneNode::Instance()->PSN2->weap_type++;
				RootSceneNode::Instance()->PSN2->weap_type=RootSceneNode::Instance()->PSN2->weap_type%2;
			}

			PE::Handle h("EVENT", sizeof(Events::Event_PlayableCharacter_Revive));
			Events::Event_PlayableCharacter_Revive *reviveEvent = new(h)Events::Event_PlayableCharacter_Revive;

			reviveEvent->m_frametime = m_frameTime;
			m_pQueueManager->add(h, QT_GENERAL);

		}
		
		else
		{
			Component::handleEvent(pEvt);
		}
	}

	void PlayableCharacterGameControls::handleControllerDebugInputEvents(Event *pEvt)
	{

		if (Event_ANALOG_L_THUMB_MOVE::GetClassId() == pEvt->getClassId())
		{

			Event_ANALOG_L_THUMB_MOVE *pRealEvent = (Event_ANALOG_L_THUMB_MOVE*)(pEvt);
			if(pRealEvent->m_absPosition.getX()!=0 && pRealEvent->m_absPosition.getY()!=0)
			{
				PE::Handle h("EVENT", sizeof(Events::Event_PlayableCharacter_Throttle));

				Events::Event_PlayableCharacter_Throttle  *flyCameraEvt = new(h) Events::Event_PlayableCharacter_Throttle  ;
				Vector3 relativeMovement(pRealEvent->m_absPosition.getX(), 0.0f, pRealEvent->m_absPosition.getY());//Flip Y and Z axis
				flyCameraEvt->m_relativeMove = relativeMovement * Debug_Fly_Speed * m_frameTime;
				m_pQueueManager->add(h, QT_GENERAL);
			}
		}
		else if (Event_ANALOG_R_THUMB_MOVE::GetClassId() == pEvt->getClassId())
		{
			Event_ANALOG_L_THUMB_MOVE *pRealEvent = (Event_ANALOG_L_THUMB_MOVE*)(pEvt);

			PE::Handle h("EVENT", sizeof(Events::Event_PlayableCharacter_Turn));

			Events::Event_PlayableCharacter_Turn  *flyCameraEvt = new(h) Events::Event_PlayableCharacter_Turn  ;
			Vector3 relativeMovement(pRealEvent->m_absPosition.getX(), 0.0f, pRealEvent->m_absPosition.getY());//Flip Y and Z axis
			flyCameraEvt->m_relativeMove = relativeMovement * Debug_Fly_Speed * m_frameTime;
			m_pQueueManager->add(h, QT_GENERAL);
		}
		else if (Event_PAD_N_DOWN::GetClassId() == pEvt->getClassId())
		{
		}
		else if (Event_PAD_N_HELD::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_PAD_N_UP::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_PAD_S_DOWN::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_PAD_S_HELD::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_PAD_S_UP::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_PAD_W_DOWN::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_PAD_W_HELD::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_PAD_W_UP::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_PAD_E_DOWN::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_PAD_E_HELD::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_PAD_E_UP::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_BUTTON_A_HELD::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_BUTTON_Y_DOWN::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_BUTTON_A_UP::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_BUTTON_B_UP::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_BUTTON_X_UP::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_BUTTON_Y_UP::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_ANALOG_L_TRIGGER_MOVE::GetClassId() == pEvt->getClassId())
		{

		}
		else if (Event_ANALOG_R_TRIGGER_MOVE::GetClassId() == pEvt->getClassId())
		{
			Event_ANALOG_R_TRIGGER_MOVE *pRealEvent = (Event_ANALOG_R_TRIGGER_MOVE*)(pEvt);
			if(pRealEvent->m_absPosition>0)
				RootSceneNode::Instance()->LBdown=true;
		}
		else if (Event_BUTTON_L_SHOULDER_DOWN::GetClassId() == pEvt->getClassId())
		{
			
		}
		else
		
		{
			Component::handleEvent(pEvt);
		}
	}

PE_IMPLEMENT_CLASS1(PlayableCharacter, GhostComponent);

PlayableCharacter::PlayableCharacter(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, Event_CreatePlayableCharacter *pEvt,float networkPingInterval) : GhostComponent(context, arena, hMyself), m_active(0), m_time(0), m_overriden(false)
{
	maxTankTimer = 10.0f;
	currTankTimer = 0.0f;
	m_playerHealth = 100.0f;
	m_playerDown = false;
	m_playerReviveTime = 10.0f;
	m_currReviveTime = 0.0f;

	kills = 0;

	//Ghost Component Stuff!
	//GhostId = numInstance!
	playerReady = false;
	stateMask = 0;
	gtype = 0;
	gactive = false;
	 
	time = 0;
	new_gun=false;
	cd_time=0;

	damage[0] = 100;
	damage[1] = 50;
	damage[2] = 75;
	damage[3] = 150;

	fr[0] = .5;
	fr[1] = .75;
	fr[2] = .30;
	fr[3] = .4;
	
	range[0] = 20;
	range[1] = 8;
	range[2] = 25;
	range[3] = 40;

	m_activate = false;
	m_pContext->getGPUScreen()->AcquireRenderContextOwnership(pEvt->m_threadOwnershipMask);

	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pMainSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pMainSN->addDefaultComponents();

	pMainSN->m_base.setPos(pEvt->m_pos);
	pMainSN->m_base.setU(pEvt->m_u);
	pMainSN->m_base.setV(pEvt->m_v);
	pMainSN->m_base.setN(pEvt->m_n);
	


	if(RootSceneNode::Instance()->m_playernumber==0)
	{
		
		pMainSN->m_isplayer=true;
		pMainSN->m_state=0;
		
		RootSceneNode::Instance()->m_playernumber=1;
		RootSceneNode::Instance()->PSN1=pMainSN;
	}
	else
	{
		pMainSN->m_isplayer=true;
		pMainSN->m_state=0;
		RootSceneNode::Instance()->PSN2=pMainSN;
	}
	
	pMainSN->radius=1;
	pMainSN->mass=5;

	PE::Handle hPM("Physics component", sizeof(PhysicsComponent));
	PhysicsComponent *pMainPM= new(hPM) PhysicsComponent(*m_pContext, m_arena, hPM);

	pMainPM->SN = pMainSN;
	pMainPM->isStatic=false;
	pMainPM->type=0;
	strcpy(pMainPM->name,pEvt->m_meshFilename);
	
	PhysicsManager::PC[PhysicsManager::top]=pMainPM;
	PhysicsManager::top=PhysicsManager::top+1;	
	
	RootSceneNode::Instance()->addComponent(hSN);
	// add the scene node as component of soldier without any handlers. this is just data driven way to locate scnenode for soldier's components
	{
		static int allowedEvts[] = {0};
		addComponent(hSN, &allowedEvts[0]);
	}

	int numskins = 1; // 8
	for (int iSkin = 0; iSkin < numskins; ++iSkin)
	{
		float z = (iSkin / 4) * 1.5f;
		float x = (iSkin % 4) * 1.5f;
		PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
		SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
		pSN->addDefaultComponents();

		pSN->m_base.setPos(Vector3(x, 0, z));
		

		// Attach Tank!
		PE::Handle hTankSN("SCENE_NODE", sizeof(SceneNode));
		SceneNode *pTankSN = new(hTankSN)SceneNode(*m_pContext, m_arena, hTankSN);
		pTankSN->addDefaultComponents();

		pSN->addComponent(hTankSN);
		PE::Handle hTankMeshInstance("TankMeshInstance", sizeof(MeshInstance));
		MeshInstance *pTankMeshInstance = new(hTankMeshInstance)MeshInstance(*m_pContext, m_arena, hTankMeshInstance);

		pTankMeshInstance->addDefaultComponents();
		pTankMeshInstance->initFromFile("kingtiger.x_main_mesh.mesha", "Default", pEvt->m_threadOwnershipMask);
		pTankSN->addComponent(hTankMeshInstance);

		//Scale Tank:
		pTankSN->m_base.importScale(.00f, .00f, .00f);

		tankSN = pTankSN;
		
		//End Attach Tank

		// rotation scene node to rotate soldier properly, since soldier from Maya is facing wrong direction
		PE::Handle hRotateSN("SCENE_NODE", sizeof(SceneNode));
		SceneNode *pRotateSN = new(hRotateSN) SceneNode(*m_pContext, m_arena, hRotateSN);
		pRotateSN->addDefaultComponents();

		pSN->addComponent(hRotateSN);
		playerSN = pRotateSN;
		//pRotateSN->m_base.turnLeft(3.1415f);

		PE::Handle hSoldierAnimSM("PlayableCharacterAnimationSM", sizeof(PlayableCharacterAnimationSM));
		PlayableCharacterAnimationSM *pSoldierAnimSM = new(hSoldierAnimSM) PlayableCharacterAnimationSM(*m_pContext, m_arena, hSoldierAnimSM);
		pSoldierAnimSM->addDefaultComponents();

		pSoldierAnimSM->m_debugAnimIdOffset = 0;// rand() % 3;

		PE::Handle hSkeletonInstance("SkeletonInstance", sizeof(SkeletonInstance));
		SkeletonInstance *pSkelInst = new(hSkeletonInstance) SkeletonInstance(*m_pContext, m_arena, hSkeletonInstance, 
			hSoldierAnimSM);
		pSkelInst->addDefaultComponents();

		pSkelInst->initFromFiles("Exo-T-Pose_Hips.skela", "TestPackage", pEvt->m_threadOwnershipMask);

		pSkelInst->setAnimSet("Exo-Idle-Aiming_Hips.animseta", "TestPackage");
		pSkelInst->setAnimSet("Exo-Walk_Hips.animseta", "TestPackage");
		pSkelInst->setAnimSet("Exo-GunPlay_Hips.animseta", "TestPackage");
		
		//Exo Body
		PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
		MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
		pMeshInstance->addDefaultComponents();

		pMeshInstance->initFromFile("EXO_Body.mesha", "TestPackage", pEvt->m_threadOwnershipMask);

		pSkelInst->addComponent(hMeshInstance);

		//Exo Eyes
		pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
		pMeshInstance->addDefaultComponents();

		pMeshInstance->initFromFile("EXO_Eyes.mesha", "TestPackage", pEvt->m_threadOwnershipMask);

		pSkelInst->addComponent(hMeshInstance);

		//Exo Eyes
		pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
		pMeshInstance->addDefaultComponents();

		pMeshInstance->initFromFile("EXO_Eyes.mesha", "TestPackage", pEvt->m_threadOwnershipMask);

		pSkelInst->addComponent(hMeshInstance);

		//Exo Suit
		pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
		pMeshInstance->addDefaultComponents();

		pMeshInstance->initFromFile("Exo_Suit.mesha", "TestPackage", pEvt->m_threadOwnershipMask);

		pSkelInst->addComponent(hMeshInstance);

		//Exo BrowsLashes
		pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
		pMeshInstance->addDefaultComponents();

		pMeshInstance->initFromFile("EXO_BrowsLashes.mesha", "TestPackage", pEvt->m_threadOwnershipMask);

		pSkelInst->addComponent(hMeshInstance);

		//Exo Caruncula
		pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
		pMeshInstance->addDefaultComponents();

		pMeshInstance->initFromFile("EXO_Caruncula.mesha", "TestPackage", pEvt->m_threadOwnershipMask);

		pSkelInst->addComponent(hMeshInstance);

		//Exo EyesSpec
		pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
		pMeshInstance->addDefaultComponents();

		pMeshInstance->initFromFile("EXO_EyesSpec.mesha", "TestPackage", pEvt->m_threadOwnershipMask);

		pSkelInst->addComponent(hMeshInstance);

		//Exo HeadMask
		pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
		pMeshInstance->addDefaultComponents();

		pMeshInstance->initFromFile("EXO_HeadMask.mesha", "TestPackage", pEvt->m_threadOwnershipMask);

		pSkelInst->addComponent(hMeshInstance);

		//Exo Teeth
		pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);
		pMeshInstance->addDefaultComponents();

		pMeshInstance->initFromFile("EXO_Teeth.mesha", "TestPackage", pEvt->m_threadOwnershipMask);

		pSkelInst->addComponent(hMeshInstance);

		// add skin to scene node
		pRotateSN->addComponent(hSkeletonInstance);

#if !APIABSTRACTION_D3D11
		{
			PE::Handle hMyGunMesh = PE::Handle("MeshInstance", sizeof(MeshInstance));
			MeshInstance *pGunMeshInstance = new(hMyGunMesh) MeshInstance(*m_pContext, m_arena, hMyGunMesh);

			pGunMeshInstance->addDefaultComponents();
			pGunMeshInstance->initFromFile("m98.x_m98main_mesh.mesha", "CharacterControl", pEvt->m_threadOwnershipMask);

			// create a scene node for gun attached to a joint

			PE::Handle hMyGunSN = PE::Handle("SCENE_NODE", sizeof(JointSceneNode));
			JointSceneNode *pGunSN = new(hMyGunSN) JointSceneNode(*m_pContext, m_arena, hMyGunSN, 76);
			pGunSN->m_base.importScale(100,100,100);
			//pGunSN->m_base.moveForward(-1);
			//pGunSN->m_base.moveDown(1);
			//pGunSN->m_base.turnAboutAxis((22/28),Vector3(1,0,0));
			pGunSN->m_base.turnAboutAxis((22/7),Vector3(0,1,0));
			float angle = (22 * 270)/(7 * 180);
			pGunSN->m_base.turnAboutAxis(angle,Vector3(1,0,0));
			angle = (22 * 315)/(7 * 180);
			pGunSN->m_base.turnAboutAxis(angle,Vector3(0,0,1));
			pGunSN->addDefaultComponents();

			// add gun to joint
			pGunSN->addComponent(hMyGunMesh);

			// add gun scene node to the skin
			pSkelInst->addComponent(hMyGunSN);
		}
#endif

		pMainSN->addComponent(hSN);
	}

	
	m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(pEvt->m_threadOwnershipMask);
#if 1

	
	// add movement state machine to soldier npc
    PE::Handle hPlayableCharacterMovementSM("PlayableCharacterMovementSM", sizeof(PlayableCharacterMovementSM));
	PlayableCharacterMovementSM *pPlayableCharacterMovementSM = new(hPlayableCharacterMovementSM) PlayableCharacterMovementSM(*m_pContext, m_arena, hPlayableCharacterMovementSM);
	pPlayableCharacterMovementSM->addDefaultComponents();

	// add it to soldier NPC
	addComponent(hPlayableCharacterMovementSM);

	// add behavior state machine ot soldier npc
    PE::Handle hPlayableCharacterBheaviorSM("PlayableCharacterBehaviorSM", sizeof(PlayableCharacterBehaviorSM));
	PlayableCharacterBehaviorSM *pPlayableCharacterBehaviorSM = new(hPlayableCharacterBheaviorSM) PlayableCharacterBehaviorSM(*m_pContext, m_arena, hPlayableCharacterBheaviorSM, hPlayableCharacterMovementSM);
	pPlayableCharacterBehaviorSM->addDefaultComponents();

	// add it to soldier NPC
	addComponent(hPlayableCharacterBheaviorSM);

	StringOps::writeToString(pEvt->m_patrolWayPoint, pPlayableCharacterBehaviorSM->m_curPatrolWayPoint, 32);
	pPlayableCharacterBehaviorSM->m_havePatrolWayPoint = StringOps::length(pPlayableCharacterBehaviorSM->m_curPatrolWayPoint) > 0;

	// start the soldier
	pPlayableCharacterBehaviorSM->start();
	

#endif
}
void PlayableCharacter::activate()
{
	// Net to put this somewhere else!
	gactive = true;


	cantakedamage = true;

	maxTankTimer = 10.0f;
	currTankTimer = 0.0f;
	m_playerHealth = 100.0f;
	m_playerDown = false;
	m_playerReviveTime = 10.0f;
	m_currReviveTime = 0.0f;

	// Set Health Circle
	UITexture temp = UISystem::Instance()->UITextureMap["Loading"];
	Handle &h = temp.m_TextSNHandle;
	TextSceneNode *pTextSN = 0;
	pTextSN = h.getObject<TextSceneNode>();
	UISystem::Instance()->changeUITextureColor("Loading", 2, pTextSN->m_color, m_playerHealth/100.0f); //This is how to change color Dynamically
	//

	//turnInto(PlayerMorphObject::Tank);
	m_active = true;
	PE_REGISTER_EVENT_HANDLER(Event_PlayableCharacter_Throttle, PlayableCharacter::do_PlayableCharacter_Throttle);
	PE_REGISTER_EVENT_HANDLER(Event_PlayableCharacter_Turn, PlayableCharacter::do_PlayableCharacter_Turn);
	PE_REGISTER_EVENT_HANDLER(Event_PlayableCharacter_Revive, PlayableCharacter::do_PlayableCharacter_Revive);
	PE_REGISTER_EVENT_HANDLER(Event_PlayableCharacter_Shoot, PlayableCharacter::do_PlayableCharacter_Shoot);

#if 0
	// add movement state machine to soldier npc
    PE::Handle hPlayableCharacterMovementSM("PlayableCharacterMovementSM", sizeof(PlayableCharacterMovementSM));
	PlayableCharacterMovementSM *pPlayableCharacterMovementSM = new(hPlayableCharacterMovementSM) PlayableCharacterMovementSM(*m_pContext, m_arena, hPlayableCharacterMovementSM);
	pPlayableCharacterMovementSM->addDefaultComponents();

	// add it to soldier NPC
	addComponent(hPlayableCharacterMovementSM);

	// add behavior state machine ot soldier npc
    PE::Handle hPlayableCharacterBheaviorSM("PlayableCharacterBehaviorSM", sizeof(PlayableCharacterBehaviorSM));
	PlayableCharacterBehaviorSM *pPlayableCharacterBehaviorSM = new(hPlayableCharacterBheaviorSM) PlayableCharacterBehaviorSM(*m_pContext, m_arena, hPlayableCharacterBheaviorSM, hPlayableCharacterMovementSM);
	pPlayableCharacterBehaviorSM->addDefaultComponents();

	// add it to soldier NPC
	addComponent(hPlayableCharacterBheaviorSM);

	// start the soldier
	pPlayableCharacterBehaviorSM->start();
#endif
	PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
	if (!hFisrtSN.isValid())
	{
		assert(!"wrong setup. must have scene node referenced");
		return;
	}

	SceneNode *charSN = hFisrtSN.getObject<SceneNode>();
	p_base = charSN->m_base;
	charSN->m_client=true;
	m_activate = true;
	//Put this in activate
	m_pContext->getDefaultGameControls()->setEnabled(false);
	m_pContext->get<CharacterControlContext>()->getPlayableCharacterGameControls()->setEnabled(true);
	
	PE::Handle hCamera("Camera", sizeof(Camera));
	Camera *pCamera = new(hCamera) Camera(*m_pContext, m_arena, hCamera, RootSceneNode::InstanceHandle());
	pCamera->addDefaultComponents();

	CameraManager::Instance()->setCamera(CameraManager::DEBUG_CAM, hCamera);

	CameraManager::Instance()->selectActiveCamera(CameraManager::DEBUG_CAM);
	
	CameraSceneNode *pCamera_Top = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
	
	float x, y, z;
	x = 0;
	y = 12;
	z = -2;//-5;

	if(charSN->m_isplayer)
	{
		pCamera_Top->m_base.setPos(Vector3(x, y, z));
		pCamera_Top->m_base.turnDown(PrimitiveTypes::Constants::c_Pi_F32/2);
		Vector3 AtoB = Vector3(x,y,z);
		AtoB.normalize();
		float a = acos(AtoB.dotProduct(Vector3(0,1,0)));
		pCamera_Top->m_base.turnDown(-a);
	}

	//Need a Place for this
	for (int ix = 0; ix < RootSceneNode::Instance()->Zomcount; ix++)
	{
		PrisonZombieBehaviorSM *pPZBehaviorSM = RootSceneNode::Instance()->Zom[ix];
		pPZBehaviorSM->start();
	}
}

void PlayableCharacter::overrideTransform(Matrix4x4 &t)
{
	m_overriden = true;
	
	m_transformOverride = t;
}

void PlayableCharacter::do_UPDATE(PE::Events::Event *pEvt)
{

	PE::Events::Event_UPDATE *pRealEvt = (PE::Events::Event_UPDATE *)(pEvt);
	PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
	if (!hFisrtSN.isValid())
	{
		assert(!"wrong setup. must have scene node referenced");
		return;
	}
	SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();

	if (m_active)
	{
		
		Vector3 a = p_base.getPos();
		Vector3 b = pFirstSN->m_base.getPos();

		static float inc = 0;

		int delayTime = 0.0f;
		inc += pRealEvt->m_frameTime;
		if (inc >= delayTime)
		{
			if (p_base.getPos().m_x != pFirstSN->m_base.getPos().m_x || p_base.getPos().m_z != pFirstSN->m_base.getPos().m_z || p_base.getN().m_x != pFirstSN->m_base.getN().m_x || p_base.getN().m_z != pFirstSN->m_base.getN().m_z)
			{
				stateMask = 1;
				p_base=pFirstSN->m_base;
			}
			//stateMask = 1;
			inc = 0;
		}
		//End Delay

		gm_base = pFirstSN->m_base;

		//Show PlayableCharacter Status:
		//UISystem::Instance()->changeUITextureText("FPSLabel", PEString::s_buf, m_pContext->m_gameThreadThreadOwnershipMask);

		if (!m_playerDown)
		{
			//m_currReviveTime += pRealEvt->m_frameTime;
			m_currReviveTime = 0.0f;
			//changeHealthBy(-.3f);
		}
		else		
		{
			// When Player is in Range of Another. We change that players health by this much!
			m_currReviveTime += pRealEvt->m_frameTime;
			changeHealthBy(((m_currReviveTime / m_playerReviveTime)*100.0f) - m_playerHealth);

		}
	}

	//countdown for gun change?
	if(new_gun)
	{
		cd_time += pRealEvt->m_frameTime;
	}
	if(cd_time>20)
	{
		new_gun=false;
		cd_time=0;
		pFirstSN->weap_type=0;
	}

	time+=pRealEvt->m_frameTime;
	m_time += pRealEvt->m_frameTime;
	m_networkPingTimer += pRealEvt->m_frameTime;
	
	if (m_overriden)
	{
		if(pFirstSN->m_base.getPos().m_x != m_transformOverride.getPos().m_x || pFirstSN->m_base.getPos().m_y != m_transformOverride.getPos().m_y || pFirstSN->m_base.getPos().m_z !=m_transformOverride.getPos().m_z )
		{
			pFirstSN->m_state=1;
		}

		
		//pFirstSN->m_base = m_transformOverride;
		pFirstSN->m_base.setPos((pFirstSN->m_base.getPos() + .09f*(m_transformOverride.getPos() - pFirstSN->m_base.getPos())));
		//m_overriden = false;
	}
	

	if (m_networkPingTimer > m_networkPingInterval && RootSceneNode::Instance()->shoot==true)
	{
		RootSceneNode::Instance()->shoot=false;
		// send client authoritative position event
		//CharacterControl::Events::Event_PlayableCharacter_Shoot evt(*m_pContext);
		//evt.m_frametime=0;

		//ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
		//pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&evt, m_pContext->getGameObjectManager(), true);
		//pNetworkManager->getNetworkContext().getGhostManager()->scheduleGhost(&evt, m_pContext->getGameObjectManager(), true);

		CharacterControl::Events::Event_Shoot_C_to_S evt(*m_pContext);
		evt.m_transform=pFirstSN->m_base;

		ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
		pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&evt, m_pContext->getGameObjectManager(), true);

		m_networkPingTimer = 0.0f;
	}
}

//virtual int useMatrix(Matrix4x4 m_base){ assert(!"This function is not overridden by this class and should not be called! Make sure to implement override of this function in current class"); return 0; }

void PlayableCharacter::useMatrix(Matrix4x4 *m_b)
{
	
	overrideTransform(*m_b);
	
}
void PlayableCharacter::do_PlayableCharacter_Throttle(PE::Events::Event *pEvt)
{

	//Send Ready up Event!
	PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
	SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();
	CharacterControl::Events::Event_MovePC_C_to_S evt(*m_pContext);
	evt.m_transform = pFirstSN->m_base;
	evt.isReady = true;
	playerReady = true;
	ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
	pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&evt, m_pContext->getGameObjectManager(), true);

	if (!m_playerDown)
	{
		Event_PlayableCharacter_Throttle *pRealEvent = (Event_PlayableCharacter_Throttle *)(pEvt);

		
		pFirstSN->m_state = 1;
		Vector3 pos = pFirstSN->m_base.getPos();

		pos.m_x += pRealEvent->m_relativeMove.getX();
		pos.m_z += pRealEvent->m_relativeMove.getZ();

		pFirstSN->m_base.setPos(pos);

	}
}
void PlayableCharacter::do_PlayableCharacter_Turn(PE::Events::Event *pEvt)
{
	if (!m_playerDown)
	{
		Event_PlayableCharacter_Throttle *pRealEvent = (Event_PlayableCharacter_Throttle *)(pEvt);

		PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
		if (!hFisrtSN.isValid())
		{
			assert(!"wrong setup. must have scene node referenced");
			return;
		}
		SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();
		if(pRealEvent->m_relativeMove.getX()==0 && pRealEvent->m_relativeMove.getZ()==0)
			return;
		pFirstSN->m_base.turnInDirection(Vector3(pRealEvent->m_relativeMove.getX(), 0.0f, pRealEvent->m_relativeMove.getZ()),3.1415f);
	}
}
void PlayableCharacter::do_PlayableCharacter_Revive(PE::Events::Event *pEvt)
{
	Event_PlayableCharacter_Revive *pRealEvent = (Event_PlayableCharacter_Revive *)(pEvt);
	/*if (m_playerCanRevive)
	{
		if (m_reviveSecs > 3.0f)
		{
			m_reviveSecs = 0.0f;
		}
		UITexture temp = UISystem::Instance()->UITextureMap["Loading"];
		Handle &h = temp.m_TextSNHandle;
		TextSceneNode *pTextSN = 0;
		pTextSN = h.getObject<TextSceneNode>();
		m_reviveSecs += pRealEvent->m_frametime;
		float percentage = m_reviveSecs / 3.0f;
		PEINFO("%f", percentage);
		UISystem::Instance()->changeUITextureColor("Loading", 2, pTextSN->m_color, percentage); //This is how to change color Dynamically

		//PE::Handle hCharSceneNode = pChar->getFirstComponentHandle<PE::Components::SceneNode>();
		//SceneNode *pFirstSN = hCharSceneNode.getObject<SceneNode>();
	}*/
	
	

}

void PlayableCharacter::shotgun()
{
	Matrix4x4 base;

	PE::Handle hCharSceneNode = getFirstComponentHandle<PE::Components::SceneNode>();
	SceneNode *charSN = hCharSceneNode.getObject<SceneNode>();

	Matrix4x4 m_base = charSN->m_base;
	m_base.turnLeft(20*3.1415/180);
	for(int i=0;i<5;i++)
	{
		Vector3 Position = m_base.getPos();
		Vector3 Direction = m_base.getN();
		Vector3 result=PhysicsManager::Instance()->PrePhysicsUpdate(charSN, range[charSN->weap_type],damage[charSN->weap_type],m_pContext);
		if(result.m_x==0 && result.m_y==50 && result.m_z==0)
		{
			Vector3 target = Position;
			Vector3 pos = Position + Direction*8 + Vector3(0,1,0);
			Vector3 color(1.0f, 0.0f, 0);
			Vector3 linepts[] = {pos, color, target, color};
					
			DebugRenderer::Instance()->createLineMesh(true, base,  &linepts[0].m_x, 2, 10);// send event while the array is on the stack
				
			int a=0; 
		}
		else
		{
			Vector3 target = Position;
			Vector3 pos = result;
			Vector3 color(0.0f, 0.0f, 1.0);
			Vector3 linepts[] = {pos, color, target, color};
					
			DebugRenderer::Instance()->createLineMesh(true, base,  &linepts[0].m_x, 2, 10);
		}
		m_base.turnRight(10*3.1415/180);
	}
					
}
void PlayableCharacter::ar()
{
	
	Matrix4x4 base;
	PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
	if (!hFisrtSN.isValid())
	{
		assert(!"wrong setup. must have scene node referenced");
		return;
	}
	SceneNode *charSN = hFisrtSN.getObject<SceneNode>();

	Vector3 Position = charSN->m_base.getPos();
	Vector3 Direction = charSN->m_base.getN();
	Vector3 result=PhysicsManager::Instance()->PrePhysicsUpdate(charSN, range[charSN->weap_type],damage[charSN->weap_type], m_pContext);
	if(result.m_x==0 && result.m_y==50 && result.m_z==0)
	{
		Vector3 target = Position;
		Vector3 pos = Position + Direction*20 + Vector3(0,1,0);
		Vector3 color(1.0f, 0.0f, 0);
		Vector3 linepts[] = {pos, color, target, color};
					
		DebugRenderer::Instance()->createLineMesh(true, base,  &linepts[0].m_x, 2, 10);// send event while the array is on the stack
				
		int a=0; 
	}
	else
	{
		Vector3 target = Position;
		Vector3 pos = result;
		Vector3 color(0.0f, 0.0f, 1.0);
		Vector3 linepts[] = {pos, color, target, color};
					
		DebugRenderer::Instance()->createLineMesh(true, base,  &linepts[0].m_x, 2, 10);
	}
}

void PlayableCharacter::do_PlayableCharacter_Shoot(PE::Events::Event *pEvt)
{
	PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
	if (!hFisrtSN.isValid())
	{
		assert(!"wrong setup. must have scene node referenced");
		return;
	}
	SceneNode *charSN = hFisrtSN.getObject<SceneNode>();

	if(charSN->weap_type==0 || charSN->weap_type==2 || charSN->weap_type==3 )
		ar();
	else if(charSN->weap_type==1)
		shotgun();

	
}

void PlayableCharacter::changeHealthBy(float amount)
{
	if(cantakedamage)
	{
		m_playerHealth += amount;
		if (m_playerHealth >= 100.0f)
		{
			m_playerHealth = 100.0f;
			if (m_playerDown)
			{
				m_playerDown = false;
				PE::Handle hFirstCharBSM = getFirstComponentHandle<PlayableCharacterBehaviorSM>();
				PlayableCharacterBehaviorSM *pCharBSM = hFirstCharBSM.getObject<PlayableCharacterBehaviorSM>();
				pCharBSM->setEnabled(true);
			}
		}
		if (m_playerHealth < 0.0f)
		{
			m_playerDown = true;
			m_playerHealth = 0.0f;

			PE::Handle hFirstCharBSM = getFirstComponentHandle<PlayableCharacterBehaviorSM>();
			PlayableCharacterBehaviorSM *pCharBSM = hFirstCharBSM.getObject<PlayableCharacterBehaviorSM>();
			pCharBSM->setEnabled(false);
		}
		// Set Health Circle
		UITexture temp = UISystem::Instance()->UITextureMap["Loading"];
		Handle &h = temp.m_TextSNHandle;
		TextSceneNode *pTextSN = 0;
		pTextSN = h.getObject<TextSceneNode>();
		UISystem::Instance()->changeUITextureColor("Loading", 2, pTextSN->m_color, m_playerHealth / 100.0f); //This is how to change color Dynamically
	}
}


void PlayableCharacter::turnInto(PlayerMorphObject obj)
{
	PE::Handle hFisrtSN = getFirstComponentHandle<SceneNode>();
	if (!hFisrtSN.isValid())
	{
		assert(!"wrong setup. must have scene node referenced");
		return;
	}
	SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();

	if (obj == PlayerMorphObject::Human)
	{
		pFirstSN->weap_type=0;
		cantakedamage = true;
		playerMorphState = PlayerMorphObject::Human;
		tankSN->m_base.importScale(0.0f, 0.0f, 0.0f);
		playerSN->m_base.importScale(1.0f, 1.0f, 1.0f);
	}

	else if (obj == PlayerMorphObject::Tank)
	{
		pFirstSN->weap_type=3;

		PE::Handle hFirstCharBSM = getFirstComponentHandle<PlayableCharacterBehaviorSM>();
		PlayableCharacterBehaviorSM *pCharBSM = hFirstCharBSM.getObject<PlayableCharacterBehaviorSM>();
		pCharBSM->startTankTimer = true;
		cantakedamage = false;
		playerMorphState = PlayerMorphObject::Tank;
		tankSN->m_base.importScale(0.5f, 0.5f, 0.5f);
		playerSN->m_base.importScale(0.0f, 0.0f, 0.0f);

		//start Timer!
	}

}


void PlayableCharacter::addDefaultComponents()
{
	Component::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(PE::Events::Event_UPDATE, PlayableCharacter::do_UPDATE);
	// custom methods of this component
}

}; // namespace Components
}; // namespace CharacterControl
