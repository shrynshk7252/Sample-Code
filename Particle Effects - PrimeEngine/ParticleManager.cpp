/* APIAbstraction */
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"
/* Outer-Engine includes */
/* Inter-Engine includes */
#include "PrimeEngine/Lua/LuaEnvironment.h"
/* Sibling and Children includes */
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "PrimeEngine/Scene/RootSceneNode.h"
#include"PrimeEngine/Particles/Particles/EnergyParticle.h"
#include"PrimeEngine/Scene/PhysicsManager.h"
/* namespace stuff */
namespace PE
{
namespace Components
{
	using namespace PE::Events;

	PE_IMPLEMENT_CLASS1(ParticleManager, SceneNode);

  //*****************************//
 //** STATIC MEMBER VARIABLES **//
//*****************************//
	Handle ParticleManager::s_myHandle;
	int ParticleManager::NumEmittersSpawned = 0;
	int ParticleManager::MaxNumEmitters = 2000;
  //***************************//
 //** SINGLETON CONSTRUCTOR **//
//***************************//
	void ParticleManager::Construct(PE::GameContext &context, PE::MemoryArena arena)
	{
		Handle handle("ParticleManager", sizeof(ParticleManager));
		ParticleManager* pParticleManager = new(handle) ParticleManager(context, arena, handle);
		pParticleManager->addDefaultComponents();
		SetInstanceHandle(handle);
		RootSceneNode::Instance()->addComponent(handle);
	}

	ParticleManager::ParticleManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
	: Component(context, arena, hMyself)
	{
		
	}

	void ParticleManager::addDefaultComponents()
	{
		Component::addDefaultComponents();
	
		m_hParticleEmitters.constructFromCapacity(MaxNumEmitters); //@todo make allocation for array more dynamic
		m_DeactivatingEmitterIndices.constructFromCapacity(MaxNumEmitters);
		m_hDeactivatingParticleEmitters.constructFromCapacity(MaxNumEmitters);

		PE_REGISTER_EVENT_HANDLER(Events::Event_PRE_GATHER_DRAWCALLS, ParticleEmitter::do_PRE_GATHER_DRAWCALLS);
		PE_REGISTER_EVENT_HANDLER(Events::Event_UPDATE, ParticleManager::do_UPDATE);
	}

  //*************//
 //** UTILITY **//
//*************//
	bool ParticleManager::DoesExist(Handle hEmitter)
	{
		return m_hParticleEmitters.isInArray(hEmitter); //Note, this onlf checks if the emitter handle is valid, not if it is an act
	}
  //************//
 //** EVENTS **//
//************//
	void ParticleManager::do_UPDATE(PE::Events::Event *pEvt)
	{
		PE::Events::Event_UPDATE *pRealEvt = (PE::Events::Event_UPDATE *)(pEvt);
		float frameTime = pRealEvt->m_frameTime;
		Update(frameTime);
	}

		void ParticleManager::do_PRE_GATHER_DRAWCALLS(Events::Event *pEvt)
	{
		
	}
  //************//
 //** UPDATE **//
//************//
	void ParticleManager::Update(float FrameTime)
	{
		for (int i = 0; i < m_hParticleEmitters.m_size; i++)
		{
			Handle PEhandle = m_hParticleEmitters[i];
			if ( PEhandle.isValid() )
			{
				ParticleEmitter* particleEmitter = m_hParticleEmitters[i].getObject<ParticleEmitter>();
				Vector3 position=particleEmitter->m_base.getPos();
				if(position.m_y<0 && particleEmitter->EmitterVelocity.length()!=0)
				{
					particleEmitter->EmitterVelocity=Vector3(0,0,0);
					particleEmitter->EmissionsPerSecond=0;
					DeactivateEmitter(PEhandle);
				}
				
				for(int count=0;count<PhysicsManager::Instance()->top;count++)
				{
					SceneNode *P1 = PhysicsManager::Instance()->PC[count]->SN;
					Vector3 Length=position-P1->m_base.getPos();
					if(Length.length()<P1->radius && !P1->m_isplayer)
					{
						particleEmitter->EmitterVelocity=Vector3(0,0,0);
						particleEmitter->EmissionsPerSecond=0;
						DeactivateEmitter(PEhandle);
					}
				}
				if ( particleEmitter->EmitterTimeLived >= particleEmitter->EmitterLifetime )
				{
					//m_DeactivatingEmitterIndices.add(i);
					DeactivateEmitter(PEhandle);
				}
			}
		}
		//set up deactivating emitters
		int numDeactivatingIndices = m_DeactivatingEmitterIndices.m_size;
		for (int index = numDeactivatingIndices-1; index >= 0; index--)
		{
			PrimitiveTypes::Int32 indexOfInactiveEmitter = m_DeactivatingEmitterIndices[index];
			DeactivateEmitter(m_hParticleEmitters[index]);
		}
		m_DeactivatingEmitterIndices.clear();

		//if emitter is done deactivating kill it, else  draw
		int NumDeactivatingParticleEmitters = m_hDeactivatingParticleEmitters.m_size;
		for ( int i = 0; i < NumDeactivatingParticleEmitters; ++i)
		{
			Handle hDeactivatingParticleEmitter = m_hDeactivatingParticleEmitters[i];
			ParticleEmitter* pDeactivatingParticleEmitter = hDeactivatingParticleEmitter.getObject<ParticleEmitter>();
			bool bHasActiveParticles = (pDeactivatingParticleEmitter->GetNumActiveParticles() > 0 );
			if (!bHasActiveParticles)
			{
				m_hDeactivatingParticleEmitters.removeUnordered(i);
				NumDeactivatingParticleEmitters = m_hDeactivatingParticleEmitters.m_size;
				removeComponent(hDeactivatingParticleEmitter);
				pDeactivatingParticleEmitter->KillEmitter();

			}
		}
	}

	void ParticleManager::PostPreDraw(int &ThreadOwnershipMask)
	{
		// For each emmitter I need to call their PostPreDraw so they can update their GPU buffers
		// since only the cpu buffers have been updated
		int NumActiveParticleEmitters = m_hParticleEmitters.m_size;
		for ( int i = 0; i < NumActiveParticleEmitters; ++i)
		{
			Handle hParticleEmitter = m_hParticleEmitters[i];
			ParticleEmitter* pParticleEmitter = hParticleEmitter.getObject<ParticleEmitter>();
			pParticleEmitter->PostPreDraw(ThreadOwnershipMask);
		}
		// we also need to finish drawing particles for the deactivating emitters
		int NumDeactivatingParticleEmitters = m_hDeactivatingParticleEmitters.m_size;
		for ( int i = 0; i < NumDeactivatingParticleEmitters; ++i)
		{
			Handle hDeactivatingParticleEmitter = m_hDeactivatingParticleEmitters[i];
			ParticleEmitter* pParticleEmitter = hDeactivatingParticleEmitter.getObject<ParticleEmitter>();
			pParticleEmitter->PostPreDraw(ThreadOwnershipMask);
		}
	}
  //************************//
 //** EMITTER MANAGEMENT **//
//************************//
	void ParticleManager::AddEmitter(ParticleEmitter& particleEmitter)
	{
		Handle hPE = particleEmitter.GetHandle();
		bool bEmitterAlredyAdded = DoesExist(hPE);
		PEASSERT(bEmitterAlredyAdded, "ERROR: YOU ARE TRYING TO ADD AN EMITTER THAT HAS ALREADY BEEN ADDED");
		particleEmitter.SetEmitterID(NumEmittersSpawned++);
		addComponent(hPE);
		m_hParticleEmitters.add(hPE);
	}

	bool ParticleManager::DeactivateEmitter(const Handle& EmitterHandle)
	{
		//Here we are removeing the particle emitter from the manager. It still exists in memory tho.
		int index = m_hParticleEmitters.indexOf(EmitterHandle);
		if(index>0)
		{
			m_hParticleEmitters.removeUnordered(index);
		
			m_hDeactivatingParticleEmitters.add(EmitterHandle);
			Handle h = EmitterHandle;
			ParticleEmitter* pPE = h.getObject<ParticleEmitter>();
			pPE->DeactivateEmitter();
		}
		//now also remove it as a comportent
		return true;

	}
};/* end namespace Components */
};/* end namespace PE */