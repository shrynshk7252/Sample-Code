
#pragma once

/* API Abstraction */
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

/* Outer-Engine includes */
#include <assert.h>

/* Inter-Engine includes */
#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Scene/SceneNode.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/Utils/Array/Array.h"
#include "PrimeEngine/Events/Event.h"
/* Sibling and Children includes */
#include "ParticleEmitter.h"


/* namespaces */
namespace PE
{
namespace Components
{

	struct ParticleManager : public Component
	{
		PE_DECLARE_CLASS(ParticleManager)

		//// SINGLETON ////////////////////////////////////////////////////////
		static void Construct(PE::GameContext &context, PE::MemoryArena arena);

		static ParticleManager* Instance()
		{
			return s_myHandle.getObject<ParticleManager>();
		}

		static Handle InstanceHandle()
		{
			return s_myHandle;
		}

		static void SetInstanceHandle(const Handle &handle)
		{
			// Singleton
			ParticleManager::s_myHandle = handle;
		}

		//// CONSTRUCTOR //////////////////////////////////////////////////////////////
		ParticleManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself);
		virtual ~ParticleManager(){}
		//// EVENTS //////////////////////////////////////////////////////////////////
		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PRE_GATHER_DRAWCALLS);
		virtual void do_PRE_GATHER_DRAWCALLS(Events::Event *pEvt);
		//// METHODS //////////////////////////////////////////////////////////////////

		/** NOT NEEDED
		 * Initializes the particle manager
		 */
		//void Init();
		/**
		 * Adds a specified particle system to the manager
		 */
		void AddEmitter(ParticleEmitter& particleEmitter);

		//PE::handle
		//construct
		//set emmitter variables
		// initialize emmiter
		//add Emitter

		/**
		 * Removes and deletes the specified particle emitter
		 * @param ParticleEmitter the particle emmiter we want removed from the system
		 * @retval true if the particle emitter was found and removed
		 * @retval false if the particlse was not found and couldn't be removed
		 */
		bool DeactivateEmitter(const Handle& Handle);
		/**
		 * Updates all active particle systems && removes all systems that die after the update
		 */
		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
		virtual void do_UPDATE(PE::Events::Event *pEvt);
		
		virtual void Update(float FrameTime);

		void PostPreDraw(int &ThreadOwnershipMask);
		/**
		 * Checks whether a given particle system exists (hasn't been removed)
		 * @retval true if particle system exists
		 * @retval false if particle system does not exist
		 */
		bool DoesExist(Handle hEmitter);

		//// COMPONENTS ////////////////////////////////////////////////////////////
		virtual void addDefaultComponents();

		//// INDIVIDUAL EVENTS ////////////////////////////////////////////////////

		//// PUBLIC DATA //////////////////////////////////////////////////////////
		
	private:
		//// PRIVATE DATA ////////////////////////////////////////////////////////
		/* handle data */
		static Handle s_myHandle;
		Handle m_hMyParticleManager;
		/* particle emmiter pre-allocation */
		
		Array<Handle>	m_hParticleEmitters;
		Array<Handle>	m_hDeactivatingParticleEmitters;
		Array<PrimitiveTypes::Int32>	m_DeactivatingEmitterIndices;
		//int		NumActiveParticleEmitters;
		static int NumEmittersSpawned;
		static int MaxNumEmitters;
	};
};/* end namespace Components */
};/* end namespace PE */