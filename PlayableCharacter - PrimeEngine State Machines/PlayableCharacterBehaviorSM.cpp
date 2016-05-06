#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/DebugRenderer.h"
#include "CharacterControl/ClientGameObjectManagerAddon.h"
#include "CharacterControl/CharacterControlContext.h"
#include "PlayableCharacterMovementSM.h"
#include "PlayableCharacterBehaviorSM.h"
#include "PlayableCharacterAnimationSM.h"
#include "PlayableCharacter.h"

#include "PrimeEngine/Networking/EventManager.h"
#include "PrimeEngine/Networking/GhostManager.h"
#include "PrimeEngine/Networking/Client/ClientNetworkManager.h"

#include "PrimeEngine/Scene/SceneNode.h"
#include "PrimeEngine/Render/IRenderer.h"
#include "PrimeEngine/Scene/RootSceneNode.h"
#include "PrimeEngine/Scene/PhysicsManager.h"
#include "PrimeEngine/GameObjectModel/DefaultGameControls/DefaultGameControls.h"
#include "PrimeEngine/Particles/Particles/ParticleManager.h"
#include "PrimeEngine/Particles/Particles/ParticleEmitter.h"

//
#include "PrimeEngine/Particles/Particles/EnergyParticle.h"
#include "PrimeEngine/Particles/Particles/SmokeParticle.h"
#include "PrimeEngine/Particles/Particles/FireballParticle.h"
#include "PrimeEngine/Particles/Particles/ExplosionParticle.h"

#include "PrimeEngine/Render/D3D9Renderer.h"
#include "PrimeEngine/APIAbstraction/Texture/Texture.h"
//

using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

namespace CharacterControl{

namespace Components{

PE_IMPLEMENT_CLASS1(PlayableCharacterBehaviorSM, Component);

PlayableCharacterBehaviorSM::PlayableCharacterBehaviorSM(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself, PE::Handle hMovementSM) 
: Component(context, arena, hMyself)
, m_hMovementSM(hMovementSM)
{
	//m_pContext->getGPUScreen()->AcquireRenderContextOwnership(context.m_gameThreadThreadOwnershipMask);
	//CameraSceneNode *camtemp = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
	//float d = camtemp->m_near;
	//	float H = 2 * d*(tan(camtemp->fov / 2.0f)); //Horizontal Field of View
	//	float W = H*camtemp->aspectRatio;
	//m_pContext->getGPUScreen()->Construct(context,W,H);
}

void PlayableCharacterBehaviorSM::start()
{
	startTankTimer = false;
	active = false;
	time=0;
	//m_pContext->getDefaultGameControls()->setEnabled(false);

	/*PlayableCharacter *pChar = getFirstParentByTypePtr<PlayableCharacter>();
	PE::Handle hCharacterSceneNode = pChar->getFirstComponentHandle<PE::Components::SceneNode>();
	SceneNode *charSN = hCharacterSceneNode.getObject<SceneNode>();

	PE::Handle hCamera("Camera", sizeof(Camera));
	Camera *pCamera = new(hCamera) Camera(*m_pContext, m_arena, hCamera, RootSceneNode::InstanceHandle());
	pCamera->addDefaultComponents();
	CameraManager::Instance()->setCamera(CameraManager::DEBUG_CAM, hCamera);

	CameraManager::Instance()->selectActiveCamera(CameraManager::DEBUG_CAM);
	
	CameraSceneNode *pCamera_Top = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
	float x, y, z;
	x = 0;
	y = 15;
	z = 0;

	if(charSN->m_isplayer)
	{
		pCamera_Top->m_base.setPos(Vector3(x, y, z));
		pCamera_Top->m_base.turnDown(1.57);
	}

	*/
	if (m_havePatrolWayPoint)
	{
		m_state = WAITING_FOR_WAYPOINT; // will update on next do_UPDATE()
	}
	else
	{
		m_state = IDLE; // stand in place

		PE::Handle h("PlayableCharacterMovementSM_Event_STOP", sizeof(PlayableCharacterMovementSM_Event_STOP));
		PlayableCharacterMovementSM_Event_STOP *pEvt = new(h) PlayableCharacterMovementSM_Event_STOP();

		m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
		// release memory now that event is processed
		h.release();
		
	}	
}

void PlayableCharacterBehaviorSM::addDefaultComponents()
{
	Component::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(PlayableCharacterMovementSM_Event_TARGET_REACHED, PlayableCharacterBehaviorSM::do_PlayableCharacterMovementSM_Event_TARGET_REACHED);
	PE_REGISTER_EVENT_HANDLER(Event_UPDATE, PlayableCharacterBehaviorSM::do_UPDATE);

	PE_REGISTER_EVENT_HANDLER(Event_PRE_RENDER_needsRC, PlayableCharacterBehaviorSM::do_PRE_RENDER_needsRC);
}

// sent by movement state machine whenever it reaches current PlayableCharacter 
void PlayableCharacterBehaviorSM::do_PlayableCharacterMovementSM_Event_TARGET_REACHED(PE::Events::Event *pEvt)
{
	PEINFO("PlayableCharacterBehaviorSM::do_PlayableCharacterMovementSM_Event_TARGET_REACHED\n");
	if (m_state == PATROLLING_WAYPOINTS)
	{
		ClientGameObjectManagerAddon *pGameObjectManagerAddon = (ClientGameObjectManagerAddon *)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon());
		if (pGameObjectManagerAddon)
		{
			// search for waypoint object
			WayPoint *pWP = pGameObjectManagerAddon->getWayPoint(m_curPatrolWayPoint);
			//This is my design
			char *arr[3],*temp;
			char string[32];
			int count=0;
			strcpy(string,pWP->m_nextWayPointName);
			temp=strtok (string,",");
			while(temp != NULL)
			{
				arr[count]= temp;
				count++;
				temp=strtok (NULL,",");
			}
			if (pWP && StringOps::length(pWP->m_nextWayPointName) > 0)
			{
				//this is my design
				if(count==1)
					pWP = pGameObjectManagerAddon->getWayPoint(pWP->m_nextWayPointName);
				else
				{
					int nextwaypoint=rand()%(count);
					pWP = pGameObjectManagerAddon->getWayPoint(arr[nextwaypoint]);
				}
				//end of my design
				// have next waypoint to go to
				//pWP = pGameObjectManagerAddon->getWayPoint(pWP->m_nextWayPointName);
				if (pWP)
				{
					StringOps::writeToString(pWP->m_name, m_curPatrolWayPoint, 32);

					m_state = PATROLLING_WAYPOINTS;
					PE::Handle h("PlayableCharacterMovementSM_Event_MOVE_TO", sizeof(PlayableCharacterMovementSM_Event_MOVE_TO));
					Events::PlayableCharacterMovementSM_Event_MOVE_TO *pEvt = new(h) PlayableCharacterMovementSM_Event_MOVE_TO(pWP->m_base.getPos());

					m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
					// release memory now that event is processed
					h.release();
				}
			}
			else
			{
				m_state = IDLE;
				// no need to send the event. movement state machine will automatically send event to animation state machine to play idle animation
			}
		}
	}
}

// this event is executed when thread has RC
void PlayableCharacterBehaviorSM::do_PRE_RENDER_needsRC(PE::Events::Event *pEvt)
{
	Event_PRE_RENDER_needsRC *pRealEvent = (Event_PRE_RENDER_needsRC *)(pEvt);
	
	//m_pContext->getGPUScreen()->AcquireRenderContextOwnership(pRealEvent->m_threadOwnershipMask);

	if (m_havePatrolWayPoint)
	{
		char buf[80], temp_buf[500];
		sprintf(buf, "Patrol Waypoint: %s",m_curPatrolWayPoint);
		
		
		PlayableCharacter *pSol = getFirstParentByTypePtr<PlayableCharacter>();
		PE::Handle hSoldierSceneNode = pSol->getFirstComponentHandle<PE::Components::SceneNode>();
		Matrix4x4 base = hSoldierSceneNode.getObject<PE::Components::SceneNode>()->m_worldTransform;
		
		//DebugRenderer::Instance()->createTextMesh(
			//buf, false, false, true, false, 0,
			//base.getPos(), 0.01f, pRealEvent->m_threadOwnershipMask);
		
		{
			//we can also construct points ourself
			bool sent = false;
			ClientGameObjectManagerAddon *pGameObjectManagerAddon = (ClientGameObjectManagerAddon *)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon());
			if (pGameObjectManagerAddon)
			{
				WayPoint *pWP = pGameObjectManagerAddon->getWayPoint(m_curPatrolWayPoint);
				if (pWP)
				{
					Vector3 target = pWP->m_base.getPos();
					Vector3 pos = base.getPos();
					Vector3 color(1.0f, 1.0f, 0);
					Vector3 linepts[] = {pos, color, target, color};
					
					//DebugRenderer::Instance()->createLineMesh(true, base,  &linepts[0].m_x, 2, 0);// send event while the array is on the stack
					sent = true;
				}
			}
			if (!sent)
			{// if for whatever reason we didnt retrieve waypoint info, send the event with transform only
				//DebugRenderer::Instance()->createLineMesh(true, base, NULL, 0, 0);// send event while the array is on the stack
			}
		}
	}
}


void PlayableCharacterBehaviorSM::do_UPDATE(PE::Events::Event *pEvt)
{
	
	//PEINFO("FrameRate: %f", ((Event_UPDATE*)(pEvt))->m_frameTime);
	PlayableCharacter *pChar = getFirstParentByTypePtr<PlayableCharacter>();
	PE::Events::Event_UPDATE *pRealEvt = (PE::Events::Event_UPDATE *)(pEvt);
	time+=pRealEvt->m_frameTime;
	
	if (startTankTimer)
	{
		std::string textureToUSE = "TankPowerUpP1";

	
		PlayableCharacter *pChar = getFirstParentByTypePtr<PlayableCharacter>();

		PE::Handle hCharSceneNode = pChar->getFirstComponentHandle<PE::Components::SceneNode>();
		SceneNode *charSN = hCharSceneNode.getObject<SceneNode>();

		if (charSN == RootSceneNode::Instance()->PSN2)
		{
			textureToUSE = "TankPowerUpP2";

		}

		

		float percent = 1 - (pChar->currTankTimer / pChar->maxTankTimer);
		UITexture temp = UISystem::Instance()->UITextureMap[textureToUSE];
		PE::Handle &h = temp.m_TextSNHandle;
		TextSceneNode *pTextSN = 0;
		pTextSN = h.getObject<TextSceneNode>();
		UISystem::Instance()->changeUITextureColor(textureToUSE.c_str(), 0, pTextSN->m_color, percent); //This is how to change color Dynamically

		pChar->currTankTimer += ((Event_UPDATE*)(pEvt))->m_frameTime;
		if (pChar->currTankTimer >= pChar->maxTankTimer)
		{
			pChar->turnInto(PlayerMorphObject::Human);
			startTankTimer = false;
			pChar->currTankTimer = 0.0f;
			UISystem::Instance()->changeUITextureColor(textureToUSE.c_str(), 0, pTextSN->m_color, 0.0f); //This is how to change color Dynamically

		}
		//Check which player it is!


	}
	

	//Only if activated!
	if(active == false)
	{
		PlayableCharacter *pC = getFirstParentByTypePtr<PlayableCharacter>();
		if(pC->m_activate == true)
			active = true;
	}
	else 
	{
		CameraSceneNode *camtemp = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
		PlayableCharacter *pChar = getFirstParentByTypePtr<PlayableCharacter>();

		PE::Handle hCharSceneNode = pChar->getFirstComponentHandle<PE::Components::SceneNode>();
		SceneNode *charSN = hCharSceneNode.getObject<SceneNode>();
		charSN->m_ispunching=true;
		Vector3 a=charSN->m_base.getPos();
		if(charSN->m_isplayer)
		{
			if(RootSceneNode::Instance()->LBdown && time>(pChar->fr[charSN->weap_type])) //.5 should be replaced by firerate
			{
				time=0;
				PE::Handle h("EVENT", sizeof(Events::Event_PlayableCharacter_Shoot));

				Events::Event_PlayableCharacter_Shoot  *flyCameraEvt = new(h) Events::Event_PlayableCharacter_Shoot;
				flyCameraEvt->m_frametime = 0;

				PE::Events::EventQueueManager::Instance()->add(h, QT_GENERAL);

				

				ParticleManager* pParticleManager = ParticleManager::Instance();
				RootSceneNode::Instance()->LBdown=false;
				RootSceneNode::Instance()->shoot=true;
				Matrix4x4 m_base = charSN->m_base;
				if(charSN->weap_type==1)
					m_base.turnLeft(20*3.1415/180);
					
				PE::Handle &hEmitter = PE::Handle("ParticleEmitter", sizeof(ParticleEmitter));
				m_pContext->getGPUScreen()->AcquireRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
				ParticleEmitter *pParticleEmitter = new(hEmitter) ParticleEmitter(*m_pContext, m_arena, hEmitter);
				pParticleEmitter->addDefaultComponents();
					
				//set emitter properties
				PrimitiveTypes::Int32 		MaxNumParticles		= 5;	/** max number of particle is the emitter */
				PrimitiveTypes::Float32		EmissionsPerSecond	= 100;	/** Base number of particle emissions per second */
				PrimitiveTypes::Int32 		EmitterLifetime		= 2;	/** Number of Sectonds to live */ /** @todo make framerate independent */
				PrimitiveTypes::Float32		StretchScale		= 0;	/** how much the particles are stretched in their direction of motion */
				PrimitiveTypes::Float32		VelocityStretchScale= 0;	/** how much the particles are stretched in their direction of motion proportional to their speed*/
				Vector3						EmitterVelocity		= m_base.getN()*200;
				pParticleEmitter->SetEmitterProperties(	MaxNumParticles, 				
														EmissionsPerSecond,
														EmitterLifetime,
														StretchScale,
														VelocityStretchScale,
														EmitterVelocity		);
				Matrix4x4 EmitterTransform;
				////set emitter transform
				//EmitterTransform.loadIdentity();
				//EmitterTransform.setPos(Vector3(2,2,2));
				//EmitterTransform.turnAboutAxis( M_PI/4, EmitterTransform.getU());
				EmitterTransform.loadIdentity();
				EmitterTransform = m_base;
				//EmitterTransform.turnDown(-90*M_PI/180);
				//EmitterTransform.turnAboutAxis( -M_PI/4, EmitterTransform.getU());
				pParticleEmitter->SetEmitterTransform(EmitterTransform);
				//setting parameters for emmitter emission variaton
				PrimitiveTypes::Float32 ThetaVariability = 0.f; /* in degrees */
				pParticleEmitter->SetEmitterVariability( ThetaVariability );
				//set emitter particle data
				//kinematics
				EnergyParticle EmitterParticle;
				EmitterParticle.Position = Vector3(0, 0, 0);
				EmitterParticle.Speed = 200.0f;
				EmitterParticle.Acceleration = Vector3(0, 0.f, 0);
				EmitterParticle.AngularVelocity = Vector4(0,0,0);
				//appearance
				EmitterParticle.Color = Vector4(1,0,0,0);
				EmitterParticle.DeltaColor = Vector4(-.4,.4,.4,0);
				EmitterParticle.Width = .5f;
				EmitterParticle.Height = .5f;
				//duration
				EmitterParticle.Lifetime = 1.f; 
				EmitterParticle.TimeLived = 0;
				EmitterParticle.Flags = 0x0;
				EmitterParticle.AnimationCycles=1;
				EmitterParticle.NumColumns=1;
				EmitterParticle.NumRows=1;
				pParticleEmitter->SetBaseParticle(EmitterParticle);
				// now that emitter stats are set we can initialize the emitter
				pParticleEmitter->InitializeParticleEmitter();//@todo make so only manager can call this. This is so the manager can check that we have space for the emitter */

				if(charSN->weap_type==1)
				{
					m_base.turnRight(10*3.1415/180);
					PE::Handle &hEmitter2 = PE::Handle("ParticleEmitter", sizeof(ParticleEmitter));
					ParticleEmitter *pParticleEmitter2 = new(hEmitter2) ParticleEmitter(*m_pContext, m_arena, hEmitter2);
					pParticleEmitter2->addDefaultComponents();
					MaxNumParticles		= 5;	/** max number of particle is the emitter */
					EmissionsPerSecond	= 100;	/** Base number of particle emissions per second */
					EmitterLifetime		= 2;	/** Number of Sectonds to live */ /** @todo make framerate independent */
					StretchScale		= 0;	/** how much the particles are stretched in their direction of motion */
					VelocityStretchScale= 0;	/** how much the particles are stretched in their direction of motion proportional to their speed*/
					EmitterVelocity		= m_base.getN()*200;
					pParticleEmitter2->SetEmitterProperties(MaxNumParticles, EmissionsPerSecond, EmitterLifetime, StretchScale, VelocityStretchScale, EmitterVelocity);
					EmitterTransform = m_base;
					pParticleEmitter2->SetEmitterTransform(EmitterTransform);
					pParticleEmitter2->SetEmitterVariability( ThetaVariability );
					pParticleEmitter2->SetBaseParticle(EmitterParticle);
					pParticleEmitter2->InitializeParticleEmitter();
					
					
					m_base.turnRight(10*3.1415/180);
					PE::Handle &hEmitter3 = PE::Handle("ParticleEmitter", sizeof(ParticleEmitter));
					ParticleEmitter *pParticleEmitter3 = new(hEmitter3) ParticleEmitter(*m_pContext, m_arena, hEmitter3);
					pParticleEmitter3->addDefaultComponents();
					MaxNumParticles		= 5;	/** max number of particle is the emitter */
					EmissionsPerSecond	= 100;	/** Base number of particle emissions per second */
					EmitterLifetime		= 2;	/** Number of Sectonds to live */ /** @todo make framerate independent */
					StretchScale		= 0;	/** how much the particles are stretched in their direction of motion */
					VelocityStretchScale= 0;	/** how much the particles are stretched in their direction of motion proportional to their speed*/
					EmitterVelocity		= m_base.getN()*200;
					pParticleEmitter3->SetEmitterProperties(MaxNumParticles, EmissionsPerSecond, EmitterLifetime, StretchScale, VelocityStretchScale, EmitterVelocity);
					EmitterTransform = m_base;
					pParticleEmitter3->SetEmitterTransform(EmitterTransform);
					pParticleEmitter3->SetEmitterVariability( ThetaVariability );
					pParticleEmitter3->SetBaseParticle(EmitterParticle);
					pParticleEmitter3->InitializeParticleEmitter();
					
					
					m_base.turnRight(10*3.1415/180);
					PE::Handle &hEmitter4 = PE::Handle("ParticleEmitter", sizeof(ParticleEmitter));
					ParticleEmitter *pParticleEmitter4 = new(hEmitter4) ParticleEmitter(*m_pContext, m_arena, hEmitter4);
					pParticleEmitter4->addDefaultComponents();
					MaxNumParticles		= 5;	/** max number of particle is the emitter */
					EmissionsPerSecond	= 100;	/** Base number of particle emissions per second */
					EmitterLifetime		= 2;	/** Number of Sectonds to live */ /** @todo make framerate independent */
					StretchScale		= 0;	/** how much the particles are stretched in their direction of motion */
					VelocityStretchScale= 0;	/** how much the particles are stretched in their direction of motion proportional to their speed*/
					EmitterVelocity		= m_base.getN()*200;
					pParticleEmitter4->SetEmitterProperties(MaxNumParticles, EmissionsPerSecond, EmitterLifetime, StretchScale, VelocityStretchScale, EmitterVelocity);
					EmitterTransform = m_base;
					pParticleEmitter4->SetEmitterTransform(EmitterTransform);
					pParticleEmitter4->SetEmitterVariability( ThetaVariability );
					pParticleEmitter4->SetBaseParticle(EmitterParticle);
					pParticleEmitter4->InitializeParticleEmitter();
					
					
					m_base.turnRight(10*3.1415/180);
					PE::Handle &hEmitter5 = PE::Handle("ParticleEmitter", sizeof(ParticleEmitter));
					ParticleEmitter *pParticleEmitter5 = new(hEmitter5) ParticleEmitter(*m_pContext, m_arena, hEmitter5);
					pParticleEmitter5->addDefaultComponents();
					MaxNumParticles		= 5;	/** max number of particle is the emitter */
					EmissionsPerSecond	= 100;	/** Base number of particle emissions per second */
					EmitterLifetime		= 2;	/** Number of Sectonds to live */ /** @todo make framerate independent */
					StretchScale		= 0;	/** how much the particles are stretched in their direction of motion */
					VelocityStretchScale= 0;	/** how much the particles are stretched in their direction of motion proportional to their speed*/
					EmitterVelocity		= m_base.getN()*200;
					pParticleEmitter5->SetEmitterProperties(MaxNumParticles, EmissionsPerSecond, EmitterLifetime, StretchScale, VelocityStretchScale, EmitterVelocity);
					EmitterTransform = m_base;
					pParticleEmitter5->SetEmitterTransform(EmitterTransform);
					pParticleEmitter5->SetEmitterVariability( ThetaVariability );
					pParticleEmitter5->SetBaseParticle(EmitterParticle);
					pParticleEmitter5->InitializeParticleEmitter();
					
					pParticleManager->AddEmitter(*pParticleEmitter2);
					pParticleManager->AddEmitter(*pParticleEmitter3);
					pParticleManager->AddEmitter(*pParticleEmitter4);
					pParticleManager->AddEmitter(*pParticleEmitter5);
						
				}
				pParticleManager->AddEmitter(*pParticleEmitter);

					//Shrey God Mode

				m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
			}
		}
					
	

		//Character screen location.
		Matrix4x4 charTemp;
		charTemp = camtemp->m_viewToProjectedTransform * camtemp->m_worldToViewTransform *charSN->m_worldTransform;
		//charTemp = charTemp *charSN->m_worldTransform;
		Vector2 PlayerScreenPos;
		PlayerScreenPos.m_x = charTemp.getPos().getX() / charTemp.getPos().getZ();
		PlayerScreenPos.m_y = charTemp.getPos().getY() / charTemp.getPos().getZ();

		//PEINFO("Character Location:  %f, %f", PlayerScreenPos.m_x, PlayerScreenPos.m_y);
	
		/*if (abs(PlayerScreenPos.m_x) >= .6f || abs(PlayerScreenPos.m_y) >= .6f)
		{
			PEINFO("Move in direction:");
		}*/

		// Movement Implementation Iteration 1.
		/*float moveSpeed = .05f;
		float boxDim = .1f;
		if ((PlayerScreenPos.m_x) >= boxDim)
		{
			//Move Cam to the right
			camtemp->m_base.moveRight(moveSpeed);
		}
		if ((PlayerScreenPos.m_x) <= -boxDim)
		{
			//Move Cam to the left
			camtemp->m_base.moveLeft(moveSpeed);
		}
		if ((PlayerScreenPos.m_y) >= boxDim)
		{
			//Move Cam to the up
			camtemp->m_base.moveUp(moveSpeed);
		}
		if ((PlayerScreenPos.m_y) <= -boxDim)
		{
			//Move Cam to the down
			camtemp->m_base.moveDown(moveSpeed);
		}*/
		
		//Linear Interpolation between PlayerPos and CamPos
		Vector3 playerPosTemp = charSN->m_base.getPos()+Vector3(0,0,-2);
		playerPosTemp.m_y = camtemp->m_base.getPos().getY();
		camtemp->m_base.setPos((camtemp->m_base.getPos() + .02f*(playerPosTemp - camtemp->m_base.getPos())));
		

		//Vector3 between = RootSceneNode::Instance()->PSN2->m_base.getPos() - RootSceneNode::Instance()->PSN1->m_base.getPos();
		//float length = between.length();
		//between.normalize();
		//Vector3 newpos = RootSceneNode::Instance()->PSN1->m_base.getPos() + ((length / 2)*between);
		//Vector3 playerPosTemp = /*charSN->m_base.getPos()*/newpos + Vector3(0, 0, 0);

		//playerPosTemp.m_y = (length) > 12 ? length : 12;
		//camtemp->m_base.setPos((camtemp->m_base.getPos() + .04f*(playerPosTemp - camtemp->m_base.getPos())));

		//Raycast Turn:
		//CameraSceneNode *camtemp = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();

		float d = camtemp->m_near;
		float H = 2 * d*(tan(camtemp->fov / 2.0f)); //Horizontal Field of View
		float W = H*camtemp->aspectRatio;

		Vector3 Center = Vector3(camtemp->m_base.getPos().m_x + (camtemp->m_base.getN().m_x * d), camtemp->m_base.getPos().m_y + (camtemp->m_base.getN().m_y * d), camtemp->m_base.getPos().m_z + (camtemp->m_base.getN().m_z * d));

		Vector3 BottomLeft = Vector3(
			Center.m_x - (camtemp->m_base.getU().m_x * (W / 2)) - (camtemp->m_base.getV().m_x * (H / 2)),
			Center.m_y - (camtemp->m_base.getU().m_y * (W / 2)) - (camtemp->m_base.getV().m_y * (H / 2)),
			Center.m_z - (camtemp->m_base.getU().m_z * (W / 2)) - (camtemp->m_base.getV().m_z * (H / 2))
			);

		Vector3 BottomRight = Vector3(
			Center.m_x + (camtemp->m_base.getU().m_x * (W / 2)) - (camtemp->m_base.getV().m_x * (H / 2)),
			Center.m_y + (camtemp->m_base.getU().m_y * (W / 2)) - (camtemp->m_base.getV().m_y * (H / 2)),
			Center.m_z + (camtemp->m_base.getU().m_z * (W / 2)) - (camtemp->m_base.getV().m_z * (H / 2))
			);

		Vector3 TopLeft = Vector3(
			Center.m_x - (camtemp->m_base.getU().m_x * (W / 2)) + (camtemp->m_base.getV().m_x * (H / 2)),
			Center.m_y - (camtemp->m_base.getU().m_y * (W / 2)) + (camtemp->m_base.getV().m_y * (H / 2)),
			Center.m_z - (camtemp->m_base.getU().m_z * (W / 2)) + (camtemp->m_base.getV().m_z * (H / 2))
			);

		Vector3 TopRight = Vector3(
			Center.m_x + (camtemp->m_base.getU().m_x * (W / 2)) + (camtemp->m_base.getV().m_x * (H / 2)),
			Center.m_y + (camtemp->m_base.getU().m_y * (W / 2)) + (camtemp->m_base.getV().m_y * (H / 2)),
			Center.m_z + (camtemp->m_base.getU().m_z * (W / 2)) + (camtemp->m_base.getV().m_z * (H / 2))
			);



		//RootSceneNode::Instance()->viewNearBL = BottomLeft;
		//RootSceneNode::Instance()->viewNearBR = BottomRight;
		//RootSceneNode::Instance()->viewNearTL = TopLeft;
		//RootSceneNode::Instance()->viewNearTR = TopRight;

		float imageWidth = (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getWidth());
		float imageHeight = (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getHeight());
		//float ptOnScreenX = RootSceneNode::Instance()->mouse_Click.getX() * imageWidth;
		//float ptOnScreenY = (RootSceneNode::Instance()->mouse_Click.getY()) * imageHeight;
		float ptOnScreenX = RootSceneNode::Instance()->MouseXPos * imageWidth;
		float ptOnScreenY = RootSceneNode::Instance()->MouseYPos * imageHeight;
		Vector3 pixelPos = Vector3(
			BottomLeft.m_x + (camtemp->m_base.getU().m_x * (ptOnScreenX + .5)*(W / imageWidth)) + (camtemp->m_base.getV().m_x * (ptOnScreenY + .5)*(H / imageHeight)),
			BottomLeft.m_y + (camtemp->m_base.getU().m_y * (ptOnScreenX + .5)*(W / imageWidth)) + (camtemp->m_base.getV().m_y * (ptOnScreenY + .5)*(H / imageHeight)),
			BottomLeft.m_z + (camtemp->m_base.getU().m_z * (ptOnScreenX + .5)*(W / imageWidth)) + (camtemp->m_base.getV().m_z * (ptOnScreenY + .5)*(H / imageHeight))

			);
		RootSceneNode::Instance()->ptNear = pixelPos;


		//RootSceneNode::Instance()->actPt1 = Center;
		//RootSceneNode::Instance()->actPt2 = pixelPos;


		//FAR PLANE
		d = camtemp->m_far;
		H = 2 * d*(tan(camtemp->fov / 2.0f)); //Horizontal Field of View
		W = H*camtemp->aspectRatio;



		Center = Vector3(camtemp->m_base.getPos().m_x + (camtemp->m_base.getN().m_x * d), camtemp->m_base.getPos().m_y + (camtemp->m_base.getN().m_y * d), camtemp->m_base.getPos().m_z + (camtemp->m_base.getN().m_z * d));

		BottomLeft = Vector3(
			Center.m_x - (camtemp->m_base.getU().m_x * (W / 2)) - (camtemp->m_base.getV().m_x * (H / 2)),
			Center.m_y - (camtemp->m_base.getU().m_y * (W / 2)) - (camtemp->m_base.getV().m_y * (H / 2)),
			Center.m_z - (camtemp->m_base.getU().m_z * (W / 2)) - (camtemp->m_base.getV().m_z * (H / 2))
			);

		BottomRight = Vector3(
			Center.m_x + (camtemp->m_base.getU().m_x * (W / 2)) - (camtemp->m_base.getV().m_x * (H / 2)),
			Center.m_y + (camtemp->m_base.getU().m_y * (W / 2)) - (camtemp->m_base.getV().m_y * (H / 2)),
			Center.m_z + (camtemp->m_base.getU().m_z * (W / 2)) - (camtemp->m_base.getV().m_z * (H / 2))
			);

		TopLeft = Vector3(
			Center.m_x - (camtemp->m_base.getU().m_x * (W / 2)) + (camtemp->m_base.getV().m_x * (H / 2)),
			Center.m_y - (camtemp->m_base.getU().m_y * (W / 2)) + (camtemp->m_base.getV().m_y * (H / 2)),
			Center.m_z - (camtemp->m_base.getU().m_z * (W / 2)) + (camtemp->m_base.getV().m_z * (H / 2))
			);

		TopRight = Vector3(
			Center.m_x + (camtemp->m_base.getU().m_x * (W / 2)) + (camtemp->m_base.getV().m_x * (H / 2)),
			Center.m_y + (camtemp->m_base.getU().m_y * (W / 2)) + (camtemp->m_base.getV().m_y * (H / 2)),
			Center.m_z + (camtemp->m_base.getU().m_z * (W / 2)) + (camtemp->m_base.getV().m_z * (H / 2))
			);

		imageWidth = (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getWidth());
		imageHeight = (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getHeight());
		//ptOnScreenX = RootSceneNode::Instance()->mouse_Click.getX() * imageWidth;
		//ptOnScreenY = (RootSceneNode::Instance()->mouse_Click.getY()) * imageHeight;
		ptOnScreenX = RootSceneNode::Instance()->MouseXPos * imageWidth;
		ptOnScreenY = RootSceneNode::Instance()->MouseYPos * imageHeight;
		pixelPos = Vector3(
			BottomLeft.m_x + (camtemp->m_base.getU().m_x * (ptOnScreenX + .5)*(W / imageWidth)) + (camtemp->m_base.getV().m_x * (ptOnScreenY + .5)*(H / imageHeight)),
			BottomLeft.m_y + (camtemp->m_base.getU().m_y * (ptOnScreenX + .5)*(W / imageWidth)) + (camtemp->m_base.getV().m_y * (ptOnScreenY + .5)*(H / imageHeight)),
			BottomLeft.m_z + (camtemp->m_base.getU().m_z * (ptOnScreenX + .5)*(W / imageWidth)) + (camtemp->m_base.getV().m_z * (ptOnScreenY + .5)*(H / imageHeight))

			);

		RootSceneNode::Instance()->ptFar = pixelPos;


		//RootSceneNode::Instance()->viewFarBL = BottomLeft;
		//RootSceneNode::Instance()->viewFarBR = BottomRight;
		//RootSceneNode::Instance()->viewFarTL = TopLeft;
		//RootSceneNode::Instance()->viewFarTR = TopRight;

		Vector3 rayOrg = RootSceneNode::Instance()->ptNear;
		Vector3 rayDir = (RootSceneNode::Instance()->ptFar - rayOrg);
		rayDir.normalize();

		//Create Michael Plane Equation from normal
		Vector3 michNorm = charSN->m_base.getV();


		bool intesectPlane = false;

		float denom = michNorm.dotProduct(rayDir);
		float t;
		if (abs(denom) > 0)
		{
			Vector3 p0l0 = charSN->m_base.getPos() - rayOrg;
			t = p0l0.dotProduct(michNorm) / denom;
			intesectPlane = (t >= 0);
		}

		if (intesectPlane)
		{
			//PEINFO("Intersected!");
			Vector3 IntersectPt = rayOrg + t*rayDir;
			RootSceneNode::Instance()->interPt = IntersectPt;
		}
		else
		{
			//PEINFO("No Intersection!");
		}

		//Michael Lookat!
		Vector3 dir = (RootSceneNode::Instance()->interPt - charSN->m_base.getPos());
		dir.normalize();


		// instantaneous turn
	
		charSN->m_base.turnInDirection(dir, 3.1415f);
	}
	if (m_state == WAITING_FOR_WAYPOINT)
	{
		if (m_havePatrolWayPoint)
		{
			ClientGameObjectManagerAddon *pGameObjectManagerAddon = (ClientGameObjectManagerAddon *)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon());
			if (pGameObjectManagerAddon)
			{
				// search for waypoint object
				WayPoint *pWP = pGameObjectManagerAddon->getWayPoint(m_curPatrolWayPoint);
				if (pWP)
				{
					m_state = PATROLLING_WAYPOINTS;
					PE::Handle h("PlayableCharacterMovementSM_Event_MOVE_TO", sizeof(PlayableCharacterMovementSM_Event_MOVE_TO));
					Events::PlayableCharacterMovementSM_Event_MOVE_TO *pEvt = new(h) PlayableCharacterMovementSM_Event_MOVE_TO(pWP->m_base.getPos());

					m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
					// release memory now that event is processed
					h.release();
				}
			}
		}
		
		else 
		{
			// should not happen, but in any case, set state to idle
			m_state = IDLE;

			PE::Handle h("PlayableCharacterMovementSM_Event_STOP", sizeof(PlayableCharacterMovementSM_Event_STOP));
			PlayableCharacterMovementSM_Event_STOP *pEvt = new(h) PlayableCharacterMovementSM_Event_STOP();

			m_hMovementSM.getObject<Component>()->handleEvent(pEvt);
			// release memory now that event is processed
			h.release();
		}
	}
	
}


}}




