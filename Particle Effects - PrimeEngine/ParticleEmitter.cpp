// APIAbstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <iostream>

// Inter-Engine includes
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/RootSceneNode.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Scene/MeshManager.h"
#include "PrimeEngine/Scene/MeshInstance.h"


// Sibling/Children includes
#include "ParticleMesh.h"
#include "ParticleEmitter.h"
#include "ParticleSceneNode.h"



namespace PE
{
namespace Components
{
	PE_IMPLEMENT_CLASS1(ParticleEmitter, SceneNode);//Component);

	//// CONSTRUCTOR ////////////////////////////////////////////
	ParticleEmitter::ParticleEmitter(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
		: SceneNode(context, arena, hMyself), s_myHandle(hMyself)
		, SecondsUntilEmission(0.f)
		, bIsDeactivated(false)
	{
		//the below are default values so we don't do processing until properties have been set
		MaxNumOfParticles = 0;	
	}

	void ParticleEmitter::addDefaultComponents()
	{
		Component::addDefaultComponents();
		//event registration
		PE_REGISTER_EVENT_HANDLER(Events::Event_PRE_GATHER_DRAWCALLS, ParticleEmitter::do_PRE_GATHER_DRAWCALLS);
		PE_REGISTER_EVENT_HANDLER(PE::Events::Event_UPDATE, ParticleEmitter::do_UPDATE);

		//now create initialize, register and add ParticleMesh
		m_hMyParticleMesh = PE::Handle("ParticleMesh", sizeof(ParticleMesh));
		ParticleMesh* pParticleMesh = new(m_hMyParticleMesh) ParticleMesh(*m_pContext, m_arena, m_hMyParticleMesh);
		pParticleMesh->addDefaultComponents();
		m_pContext->getMeshManager()->registerAsset(m_hMyParticleMesh);
		addComponent(m_hMyParticleMesh);
		m_pMyParticleMesh = pParticleMesh; //for mesh
		m_pMyParticleMesh->particleArray = &m_ActiveParticles; //for particle array
	}

//*************//
//** SETTERS **//
//*************//
	void ParticleEmitter::SetEmitterProperties(	PrimitiveTypes::Int32 		In_MaxParticles, 			/** Max number of particles emmiter can have alive at any moment */
												PrimitiveTypes::Float32		In_EmissionsPerSecond,
												PrimitiveTypes::Int32 		In_EmitterLifetime,			/** Number of Sectonds to live */ /** @todo make framerate independent */
												PrimitiveTypes::Float32		In_StretchScale,			/** how much the particles are stretched in their direction of motion */
												PrimitiveTypes::Float32		In_VelocityStretchScale,	/** how much the particles are stretched in their direction of motion proportional to their speed*/
												Vector3						In_EmitterVelocity
											   )
	{
		//EMITTER PROPERTIES
		MaxNumOfParticles		= In_MaxParticles; 	
		EmissionsPerSecond		= In_EmissionsPerSecond;
		EmitterLifetime			= In_EmitterLifetime;
		StretchScale			= In_StretchScale;
		VelocityStretchScale	= In_VelocityStretchScale;
		EmitterVelocity			= In_EmitterVelocity;
	}

	void ParticleEmitter::SetEmitterVariability(PrimitiveTypes::Float32		In_ThetaVariability)
	{
		ThetaVariability			= In_ThetaVariability;
	}

	void ParticleEmitter::SetEmitterID(PrimitiveTypes::Int32 In_ID)
	{
		m_ID = In_ID;
	}

	void ParticleEmitter::SetEmitterTransform(Matrix4x4& m_base_in)
	{
		memcpy(&m_base, &m_base_in, sizeof(Matrix4x4) );
	}
	
	void ParticleEmitter::SetBaseParticle(Particle& particle)
	{
		memcpy(&m_pBaseParticle, &particle, sizeof(Particle) );
	}

  //****************//
 //** INITIALIZE **//
//****************//
	void ParticleEmitter::InitializeParticleEmitter()
	{
		m_ActiveParticles.constructFromCapacity(MaxNumOfParticles);
		m_DeactivatingParticleIndices.constructFromCapacity(MaxNumOfParticles);
		
		//now set up the mesh correctly. Currently. Usede lines 60 through 68  from text scene node
		
		//set the m_base transform we are connected to
		//now create, initialize and add mesh instance
		m_hMyParticleMeshInstance = PE::Handle("MeshInstance", sizeof(MeshInstance));
		MeshInstance* pInstance = new(m_hMyParticleMeshInstance) MeshInstance(*m_pContext, m_arena, m_hMyParticleMeshInstance);
		pInstance->addDefaultComponents();
		pInstance->initFromRegisteredAsset(m_hMyParticleMesh); //initialize the instance from the original asset 
		addComponent(m_hMyParticleMeshInstance);
		//m_pContext->getGPUScreen()->AcquireRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
		m_pMyParticleMesh->loadFromFile_needsRC(m_pBaseParticle, m_pContext->m_gameThreadThreadOwnershipMask);
	}
  //************//
 //** EVENTS **//
//************//
	void ParticleEmitter::do_UPDATE(PE::Events::Event *pEvt)
	{
		PE::Events::Event_UPDATE *pRealEvt = (PE::Events::Event_UPDATE *)(pEvt);
		float frameTime = pRealEvt->m_frameTime;
		Update(frameTime);
		//ParticleMesh* pParticleMesh = m_hMyParticleMesh.getObject<ParticleMesh>();
		//Handle hPositionBuffer = pParticleMesh->m_hPositionBufferCPU;
		//PositionBufferCPU* pPB = hPositionBuffer.getObject<PositionBufferCPU>();
	}
	void ParticleEmitter::do_PRE_GATHER_DRAWCALLS(Events::Event *pEvt)
	{
		
	}
	
	void ParticleEmitter::PostPreDraw(int &ThreadOwnershipMask)
	{
		// call to get the mesh geometry updated to the gpu buffer
		ParticleMesh* pParticleMesh = m_hMyParticleMesh.getObject<ParticleMesh>();
		pParticleMesh->PostPreDraw(ThreadOwnershipMask);
	}

//************//
//** UPDATE **//
//************//
	void ParticleEmitter::Update(float TimeStep)
	{	
		//update emitter properties
		EmitterTimeLived += TimeStep;
		Vector3 EmitterPosition = m_base.getPos();
		EmitterPosition  += EmitterVelocity * TimeStep;
		m_base.setPos(EmitterPosition);

		//update particles
		updateParticles(TimeStep);
		generateNewParticles(TimeStep);
		inactivateDeactivatingParticles();
	}

	void ParticleEmitter::updateParticles(PrimitiveTypes::Float32 TimeStep)
	{
		//first, LOOP update all currently active particles (for all active)
		for ( int i = 0; i < m_ActiveParticles.m_size; i++ )
		{
			Particle p = m_ActiveParticles[i];
			
			p.Update(TimeStep);

			m_ActiveParticles[i] = p;
			//if particle is outdated kill it
			if ( (p.TimeLived) > (p.Lifetime) )
			{
				m_DeactivatingParticleIndices.add(i);
			}
			Vector3 length=p.Position-m_base.getPos();
			if(length.length()>8)
			{
				//m_DeactivatingParticleIndices.add(i);
			}
		}
	}

	void ParticleEmitter::generateNewParticles(PrimitiveTypes::Float32 TimeStep)
	{
		/** @todo calculate random number of particles to be added */
		PrimitiveTypes::Int32 NumParticlesToAdd = CalcutateNumParticlesToAdd(TimeStep);
		PrimitiveTypes::Int32 ParticlesAdded = 0;
		PrimitiveTypes::Int32 NumDeactivatingParticles = m_DeactivatingParticleIndices.m_size;
		//first use all the deactivating particle
		while ( (NumDeactivatingParticles > 0) &&  (ParticlesAdded < NumParticlesToAdd) )
		{
			int index = m_DeactivatingParticleIndices[ParticlesAdded];
			GenerateNewParticleData(m_ActiveParticles[index]);
			--NumDeactivatingParticles;
			++ParticlesAdded;
		}
		//now remove those particles from the deactivating 
		m_DeactivatingParticleIndices.remove(0,ParticlesAdded);

		while ( ParticlesAdded < NumParticlesToAdd  )
		{
			Particle p;
			GenerateNewParticleData(p);
			m_ActiveParticles.add(p);
			++ParticlesAdded;
		}
		
	}

	PrimitiveTypes::Int32 ParticleEmitter::CalcutateNumParticlesToAdd(PrimitiveTypes::Float32 TimeStep)
	{
		PrimitiveTypes::Int32 numParticlesToAdd = 0;
		PrimitiveTypes::Float32 FrameTime = TimeStep;
		PrimitiveTypes::Float32 TimePerParticle = ( 1.f/ EmissionsPerSecond);
		while( FrameTime > 0.f)
		{
			if (FrameTime > SecondsUntilEmission)
			{
				FrameTime -= SecondsUntilEmission;
				numParticlesToAdd += 1;
				SecondsUntilEmission = TimePerParticle;
			}
			else  // decrement time until emission
			{
				SecondsUntilEmission -= FrameTime;
				FrameTime = 0.0f;// we used up all of our frame time
			}
		}
		/*PrimitiveTypes::Int32 variability = (MaxEmissionsPerFrame - MinEmissionsPerFrame);
		numParticlesToAdd = rand() % (variability + 1);
		numParticlesToAdd += MinEmissionsPerFrame;*/
		// see if there is space to add new particles
		PrimitiveTypes::Int32 numFreeing = m_DeactivatingParticleIndices.m_size;
		PrimitiveTypes::Int32 SpaceInBuffer = (m_ActiveParticles.m_capacity - m_ActiveParticles.m_size) + numFreeing;
		numParticlesToAdd = (SpaceInBuffer < numParticlesToAdd)? SpaceInBuffer:numParticlesToAdd;

		return numParticlesToAdd;
	}

	void ParticleEmitter::inactivateDeactivatingParticles()
	{
		
		//we have to deactivate from bahighest index to front because of how array remove works
		int numToDeactivate = m_DeactivatingParticleIndices.m_size;
		for ( int index = (numToDeactivate-1); index >= 0; --index)
		{
			int deactivatingParticleIndex = m_DeactivatingParticleIndices[index];
			m_ActiveParticles.removeUnordered(deactivatingParticleIndex);
		}
		m_DeactivatingParticleIndices.clear();
	}
//************************************//
//** PARTICLE GENERATORS/ACTIVATION **//
//************************************//

	void ParticleEmitter::GenerateNewParticleData(Particle& particle)
	{
		//set to normal
		memcpy(&particle, &m_pBaseParticle, sizeof(Particle) );
		particle.Position += m_base.getPos();
		//then add vairability
		particle.Position;
		particle.Velocity = GenerateNewParticleVelocity(particle);
		particle.Acceleration;
		particle.AngularVelocity;
		particle.Color;
		particle.DeltaColor;
		particle.Flags;
		particle.Lifetime;
		particle.TimeLived;
		particle.Width;
		particle.Height;
		
	}

  //***********************************//
 //** PARTICLE ATTRIBUTE GENERATORS **//
//***********************************//
		void ParticleEmitter::GenerateNewParticlePosition(Particle& particle)
		{

		}

		void ParticleEmitter::GenerateNewParticleSize(Particle& particle)
		{
		
		}

		void ParticleEmitter::GenerateNewParticleWidth(Particle& particle)
		{
		
		}

		void ParticleEmitter::GenerateNewParticleHeight(Particle& particle)
		{
		
		}

		Vector3 ParticleEmitter::GenerateNewParticleVelocity(Particle& particle)
		{
			Vector3 VelocityDirection = m_base.getN();
			VelocityDirection.normalize();

			// now get our random theta and phi
			PrimitiveTypes::Float32 Theta	= 0.f;
			PrimitiveTypes::Float32 Phi		= 0.f;
			// random float between 0 and the max theta variability
			Theta = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/ThetaVariability));
			// random float between 0 and 360
			PrimitiveTypes::Float32 MaxPhiVariability = 360.f;
			Phi = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/MaxPhiVariability));
			//convert to radians
			Theta = M_PI * (Theta / 180.f);
			Theta = abs(Theta);
			Phi = M_PI * (Phi / 180.f);
			Phi = abs(Phi);

			//Get the transform axis
			Vector3 nVector = m_base.getN();
			//nVector.normalize();
			Vector3 uVector = m_base.getU();
			//uVector.normalize();
			Vector3 vVector = m_base.getV();
			//vVector.normalize();
			//Get the transform axis
			Vector3 n_Vector = m_worldTransform.getN();//mgetN();
			//nVector.normalize();
			Vector3 u_Vector = m_worldTransform.getU();
			//uVector.normalize();
			Vector3 v_Vector = m_worldTransform.getV();

			
			//contribution from u
			Vector3 uContrib = sin(Phi) * uVector;
			//contribution from v
			Vector3 vContrib = cos(Phi) * vVector;

			Vector3 planarComponent = (uContrib+vContrib);//.normalize();
			planarComponent.normalize();

			VelocityDirection = (nVector * cos(Theta)) + (planarComponent * sin(Theta));

			//contribution from n
			Vector3 nContrib = Vector3(0,0,0);



			return VelocityDirection * particle.Speed;
		}

		void ParticleEmitter::GenerateNewParticleAngularVelocity(Particle& particle)
		{
			
		}

		void ParticleEmitter::GenerateNewParticleColor(Particle& particle)
		{
		
		}

		void ParticleEmitter::GenerateNewParticleDeltaColor(Particle& particle)
		{
		
		}

	//******************//
	//** KILL EMITTER **//
	//******************//
	void ParticleEmitter::DeactivateEmitter()
	{
		bIsDeactivated = true;
		EmissionsPerSecond = 0.f;
	}
	void ParticleEmitter::KillEmitter()
	{
		/** @todo implement killEmitter*/
		setEnabled(false);
		m_pMyParticleMesh->setEnabled(false);
	}
	//void ParticleEmitter::set_emit_instantaneously(	int emission_start_time,
	//												int num_to_emit,
	//												int min_emit_per_burst,
	//												int max_emissions_per_frame
	//											  )
	//{

	//}

	//void ParticleEmitter::set_emit_continuously(int emission_start_time,
	//											int emission_rate,
	//											int emission_duration
	//										   )
	//{

	//}

};/* end namespace Components */
};/* end namespace PE */