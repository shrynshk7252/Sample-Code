
#pragma once

/* API Abstraction */
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

/* Outer-Engine includes */
#include <assert.h>

/* Inter-Engine includes */

#include "PrimeEngine/APIAbstraction/Effect/Effect.h"
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"

#include "PrimeEngine/Scene/Mesh.h"
#include "PrimeEngine/Utils/Array/Array.h"

/* Sibling/Children includes */
#include "Particle.h"
#include "ParticleSceneNode.h"

namespace PE {
namespace Components {

	struct ParticleMesh : public Mesh
	{
		PE_DECLARE_CLASS(ParticleMesh);

		// Constructor -------------------------------------------------------------
		ParticleMesh(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself);
		/*: Mesh(context, arena, hMyself)
		, m_hMyself(hMyself)
		, m_eyePos(Vector3(0,0,0))
		{
			m_loaded = false;
		}*/

		virtual ~ParticleMesh(){}

		virtual void addDefaultComponents();


		//** EVENTS **//
		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_GATHER_DRAWCALLS);
		virtual void do_GATHER_DRAWCALLS(Events::Event *pEvt);
		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE);
		virtual void do_UPDATE(Events::Event *pEvt);

		/** SETTERS **/

		//** UPDATE **//
		void Update();
		void UpdatePositionBuffer(Vector3& eyePos, PrimitiveTypes::Float32 frameTime);
		void UpdateTextCoordBuffer(PrimitiveTypes::Float32 frameTime);
		void UpdateIndexBuffer(PrimitiveTypes::Float32 frameTime);
		void UpdateNormalBuffers(PrimitiveTypes::Float32 frameTime);

		void PostPreDraw(int &ThreadOwnershipMask);






		void loadFromFile_needsRC(Particle& particle, int &threadOwnershipMask);

		PrimitiveTypes::Float32 m_Width;		/* */
		PrimitiveTypes::Float32 m_Height;		/* */
		PrimitiveTypes::Bool m_loaded;			/* */
		Handle m_meshCPU;						/* */

		PrimitiveTypes::Float32 m_numParticles; /* used to track number of particles in mesh*/

		Handle m_hMyself;

		Array<PrimitiveTypes::Float32> TexCoordSets;
		Array<Particle>* particleArray;

		Vector3 m_eyePos;

	}; /* end struct ParticleMesh */

}; /* namespace Components */
}; /* end namespace PE */