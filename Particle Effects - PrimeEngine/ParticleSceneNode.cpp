/* Outer Engine Includes */

/* Inter-Engine Includes */
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/Scene/MeshManager.h"
#include "PrimeEngine/Scene/MeshInstance.h"

/* Sister and Child Includes */
#include "ParticleSceneNode.h"
#include "ParticleMesh.h"

namespace PE {
namespace Components {

	PE_IMPLEMENT_CLASS1(ParticleSceneNode, SceneNode);

//******************//
//** Construction **//
//******************//
	ParticleSceneNode::ParticleSceneNode(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
	: SceneNode(context, arena, hMyself)
	{
		m_cachedAspectRatio = 1.0f;
		m_Xscale = 1.0f;
		m_Yscale = 1.0f;

		m_drawType = InWorldFacingCamera; //our particle 
		if (IRenderer* pS = context.getGPUScreen())
		{
			m_cachedAspectRatio = float(pS->getWidth()) / float(pS->getHeight());
		}
	}

	void ParticleSceneNode::addDefaultComponents()
	{
		SceneNode::addDefaultComponents();
		// event handlers
		PE_REGISTER_EVENT_HANDLER(Events::Event_PRE_GATHER_DRAWCALLS, ParticleSceneNode::do_PRE_GATHER_DRAWCALLS);
	}

	void ParticleSceneNode::loadFromFile_needsRC(int &threadOwnershipMask)
	{
		ParticleMesh *pParticleMesh = NULL;
		if (m_hMyParticleMesh.isValid())
		{
			 pParticleMesh = m_hMyParticleMesh.getObject<ParticleMesh>();
		}
		else
		{
			//Initialize the particle mesh
			m_hMyParticleMesh = PE::Handle("ParticleMesh", sizeof(ParticleMesh));
			pParticleMesh = new(m_hMyParticleMesh) ParticleMesh(*m_pContext, m_arena, m_hMyParticleMesh);
			pParticleMesh->addDefaultComponents();

			m_pContext->getMeshManager()->registerAsset(m_hMyParticleMesh);

			// Initialize the mesh instance from the ParticleMesh
			m_hMyParticleMeshInstance = PE::Handle("MeshInstance", sizeof(MeshInstance));
			MeshInstance *pMeshInstance = new(m_hMyParticleMeshInstance) MeshInstance(*m_pContext, m_arena, m_hMyParticleMeshInstance);
			pMeshInstance->addDefaultComponents();
			pMeshInstance->initFromRegisteredAsset(m_hMyParticleMesh);

			addComponent(m_hMyParticleMeshInstance);
		}

		PE::IRenderer::checkForErrors("");

		/* hardcoding data for calling load from file function call */
		const char* fileName = "EnergyParticle.dds";
		const char* package = "Default";
		const char* tech = "StdMesh_Diffuse_Tech";
		int Width = 1;
		int Height = 1;
		// Now that variables needed to call the function are set call the function 
		//pParticleMesh->loadFromFile_needsRC(fileName, package, Width, Height, tech, threadOwnershipMask);
	}
//*************//
//** SETTERS **//
//*************//
	void ParticleSceneNode::setSelfAndMeshAssetEnabled(bool enabled)
	{
		setEnabled(enabled);

		if (m_hMyParticleMesh.isValid())
		{
			m_hMyParticleMesh.getObject<Component>()->setEnabled(enabled);
		}
	}
//************//
//** EVENTS **//
//************//
	void ParticleSceneNode::do_PRE_GATHER_DRAWCALLS(Events::Event *pEvt)
	{
		// Declare and initialize the draw event variable
		Events::Event_PRE_GATHER_DRAWCALLS *pDrawEvent = NULL;
		// Assign and type cast the event from the event parameter
		pDrawEvent = (Events::Event_PRE_GATHER_DRAWCALLS *)(pEvt);

		Matrix4x4 projectionViewWorldMatrix = pDrawEvent->m_projectionViewTransform;
		Matrix4x4 worldMatrix;

		/* if particleMeshInstance is invalid do nothing */
		if (!m_hMyParticleMeshInstance.isValid())
		{
			return;
		}

		ParticleMesh* pParticleMesh = m_hMyParticleMesh.getObject<ParticleMesh>();
		//This preps the m_worldTransform for transformation of our mesh
		if (m_drawType == InWorldFacingCamera)
		{
			m_worldTransform.turnTo(pDrawEvent->m_eyePos);
		}
	}

}; // namespace Components
}; // namespace PE
