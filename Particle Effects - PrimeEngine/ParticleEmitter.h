
#pragma once

/* API Abstraction */
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

/* Outer-Engine includes */
#include <assert.h>

/* Inter-Engine includes */
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/Utils/Array/Array.h"
#include "PrimeEngine/Math/Vector4.h"
#include "PrimeEngine/Math/Matrix4x4.h"

/* particle specific includes */
#include "Particle.h"
#include "ParticleHelper.h"
#include "ParticleSceneNode.h"
#include "ParticleMesh.h"



namespace PE
{
namespace Components
{
	struct ParticleEmitter: public SceneNode
	{
		/// PARTS ADAPTED FROM DEBUG RENDERER ///////////////////////////////////////////////
		PE_DECLARE_CLASS(ParticleEmitter);

		ParticleEmitter(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself);	
		virtual ~ParticleEmitter() {}

		virtual void addDefaultComponents();

		//** EVENTS **//
		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PRE_GATHER_DRAWCALLS);
		virtual void do_PRE_GATHER_DRAWCALLS(Events::Event *pEvt);
		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
		virtual void do_UPDATE(PE::Events::Event *pEvt);
	
		void PostPreDraw(int &ThreadOwnershipMask);
		//** GETTERS **//
		Handle GetHandle(){return s_myHandle;}
		
		int GetNumActiveParticles(){return m_ActiveParticles.m_size;}
		//// SETTERS ////////////////////////////////////////////////////////////////////
		
		/**
		 * Calculates and sets the defaults for a fresh particle
		 */
		
		virtual void SetBaseParticle(Particle& particle);
		/**
		 * Initializes all variables to 0 so we know they have a default value.
		 * Meant to keep our data under control.
		 * 
		 */
		virtual void InitializeParticleEmitter( );

		/**
		 * used to set the basic emitter properties
		 */
		virtual void SetEmitterProperties(	PrimitiveTypes::Int32 		In_MaxParticles, 			//NOT SURE WHAT THIS DOES
											PrimitiveTypes::Float32		In_EmissionsPerSecond,		/** Base number of particle emissions per second */
											PrimitiveTypes::Int32 		In_EmitterLifetime,			/** Number of Sectonds to live */
											PrimitiveTypes::Float32		In_StretchScale,			/** how much the particles are stretched in their direction of motion */
											PrimitiveTypes::Float32		In_VelocityStretchScale,	/** how much the particles are stretched in their direction of motion proportional to their speed*/
											Vector3						In_EmitterVelocity
										);

		/**
		 * used after construction to set emitter variability
		 */
		virtual void SetEmitterVariability( PrimitiveTypes::Float32		In_ThetaVariability		);
		/**
		 * used to set the emitter IDs
		 */
		void SetEmitterID(PrimitiveTypes::Int32	In_ID);
		/**
		 * used to set the emitter's transform
		 */
		void SetEmitterTransform(Matrix4x4& m_base_in);
		/**
		 * used to set the data used for generating UV texture coordinates
		 */
		void SetEmitterUVData(	PrimitiveTypes::Int32 In_UVAnimationCycles, /** number of uv animation cycles */
								PrimitiveTypes::Int32 In_UVNumXTiles,		/** number of horizontal tiles in the mesh texture */
								PrimitiveTypes::Int32 In_UVNumYTiles		/** number of vertical tiles in the mesh texture */
							  );
		//// OTHER METHODS ////////////////////////////////////////////////////////////////////
		/**
		 * Deactivates the emiter. It will no longer emit particles.
		 */
		void DeactivateEmitter();
		/**
		 * Lets the emitter know to stop emitting particles and to kill itself once all particles die
		 */
		void KillEmitter();
		//// UPDATES
		//// lots of update methods meant to make it easier to make manf different styles of emmitters and interesting effects
		/**
		 * Updates: Particle positions, particle attributes, particle bounding box, updates num alive particles
		 *
		 * @retval false if no particlse are alive
		 * @retval true if particlse are still alive
		 */
		virtual void Update(float FrameTime);
		//PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE)
		//virtual void do_UPDATE(PE::Events::Event *pEvt);

		 //// DATA MEMBERS ////////////////////////////////////////////////////////////
		 //static const Handle m_hComponentParent;
		 Matrix4x4 m_base;
		 Matrix4x4 m_worldTransform;
	
		//EMITTER DATA
		PrimitiveTypes::Int32		MaxNumOfParticles;		/** Maximum number of particles the will can have active */
		PrimitiveTypes::Float32		EmissionsPerSecond;		/** the number of particles to emitt per second.*/
		PrimitiveTypes::Float32 	EmitterLifetime;		/** Number of Sectonds to live */
		PrimitiveTypes::Float32		EmitterTimeLived;		/** the time the emmiter was created */
		PrimitiveTypes::Float32		StretchScale;			/** how much the particles are stretched in their direction of motion */
		PrimitiveTypes::Float32		VelocityStretchScale;	/** how much the particles are stretched in their direction of motion proportional to their speed*/
		Vector3						EmitterVelocity;

		//PARTICLE VARIABILITY FACTORS
		PrimitiveTypes::Float32		ThetaVariability;

		//RENDERING DATA
		//PEmitterBlendMode	BlendMode;			/** DIFFERENT BLEND MODES FOR DIFFERENT SYSTEMS */
		//PEmitterType		EmitterType;		/** Represents the type of system, instant, burst, continuous */
		ParticleMesh*		m_pMyParticleMesh;  /** The particle mesh data for drawing our particles */


		//Properties we can add later, but for now, not a part of CORE functionality
		//AABB			BoundingBox;		/** The bounding volume of the emitter */
		//Vector4 		StartColor;			/** color of the particle when it comes to life */
		//Vector4		EndColor;			/** color of the particle the last frame of its existance */
		Particle						m_pBaseParticle;
	private:
		//** HANDLES **//
		Handle s_myHandle;
		Handle m_hMyParticleMesh;
		Handle m_hMyParticleMeshInstance;

		//** PARTICLE DATA **//
		Array<Particle>					m_ActiveParticles;							/** the particle data coresponding to each particleSN */
		Array<PrimitiveTypes::Int32>	m_DeactivatingParticleIndices;
								/** the base particle data. Emitter uses this for creating randomized particles */
		/* EMISSION DATA */
		PrimitiveTypes::Float32 SecondsUntilEmission;
		
		/* IDENTIFICATION DATA */
		PrimitiveTypes::Int32	m_ID;			/** Emitter ID */

		/* DEACTIVATION DATA */
		bool bIsDeactivated;

		/**
		 * updates the particle data
		 */
		void updateParticles(PrimitiveTypes::Float32 TimeStep);
		/**
		 * spawns new particle for the given timestep
		 */
		/**
		 * deactivates the particle with the given index
		 */
		void deactivateParticle(Particle& particle);
		/**
		 * inactivates particles that were deactivating
		 */
		void inactivateDeactivatingParticles();
		/**
		 *
		 */
		void generateNewParticles(PrimitiveTypes::Float32 TimeStep);
		/**
		 *
		 */
		PrimitiveTypes::Int32 CalcutateNumParticlesToAdd(PrimitiveTypes::Float32 TimeStep);

		  //***********************************//
		 //** Particle Attribute GENERATORS **//
		//**********************************//
		/**
		 * Generates new particle data for when a particle is spawned
		 */
		void GenerateNewParticleData(Particle& particle);
		/**
		 * @todo implement and integrate
		 */
		virtual void GenerateNewParticlePosition(Particle& particle);
		/**
		  * @todo implement and integrate
		 */
		virtual void GenerateNewParticleSize(Particle& particle);
		/**
		  * @todo implement and integrate
		 */
		virtual void GenerateNewParticleWidth(Particle& particle);
				/**
		  * @todo implement and integrate
		 */
		virtual void GenerateNewParticleHeight(Particle& particle);
		/**
		  * @todo implement and integrate
		 */
		virtual Vector3 GenerateNewParticleVelocity(Particle& particle);
		/**
		  * @todo implement and integrate
		 */
		virtual void GenerateNewParticleAngularVelocity(Particle& particle);
		/**
		  * @todo implement and integrate
		 */
		virtual void GenerateNewParticleColor(Particle& particle);
		/**
		  * @todo implement and integrate
		 */
		virtual void GenerateNewParticleDeltaColor(Particle& particle);
	};


};/* end namespace Components */
};/* end namespace PE */