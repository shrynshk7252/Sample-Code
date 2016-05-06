
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
#include "PrimeEngine/Scene/SceneNode.h"


namespace PE
{
namespace Components
{

	struct ParticleSceneNode : public SceneNode
	{
		PE_DECLARE_CLASS(ParticleSceneNode);

		//// Constructor /////////////////////////////////////////////////////////////////
		ParticleSceneNode(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself);

		virtual ~ParticleSceneNode() {}

		void setSelfAndMeshAssetEnabled(bool enabled);

		//// Component ////////////////////////////////////////////////////////////////////
		virtual void addDefaultComponents();
	
		//// Individual events ////////////////////////////////////////////////////////////
		PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_PRE_GATHER_DRAWCALLS);
		virtual void do_PRE_GATHER_DRAWCALLS(Events::Event *pEvt);

		//// ENUMS ///////////////////////////////////////////////////////////////////////
		enum DrawType
		{
			InWorld,
			InWorldFacingCamera,
			Overlay2D,
			Overlay2D_3DPos
		};

		//// OTHER METHODS ///////////////////////////////////////////////////////////////
		void loadFromFile_needsRC(/*DrawType drawType,*/ int &threadOwnershipMask);

		//// DATA ///////////////////////////////////////////////////////////////////////
		DrawType m_drawType;
		float m_Xscale;
		float m_Yscale;
		Handle m_hMyParticleMesh;
		Handle m_hMyParticleMeshInstance;
		float m_cachedAspectRatio;
		bool m_canBeRecreated;

	}; // struct ParticleSceneNode

}; // namespace Components
}; // namespace PE
