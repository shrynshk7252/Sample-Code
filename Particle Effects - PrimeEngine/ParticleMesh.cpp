/* API Abstraction */
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

/* Outer-Engine includes */
#include <assert.h>

/* Inter-Engine includes */
#include "PrimeEngine/APIAbstraction/GPUMaterial/GPUMaterialSet.h"
#include "PrimeEngine/APIAbstraction/Texture/Texture.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPUManager.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "PrimeEngine/FileSystem/FileReader.h"
#include "PrimeEngine/GameObjectModel/Camera.h"
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/Scene/SceneNode.h"
#include "PrimeEngine/Scene/CameraManager.h"

/* Sibling/Children includes */
#include "ParticleMesh.h"
#include "PrimeEngine/Scene/MeshInstance.h"


namespace PE {
namespace Components{

	PE_IMPLEMENT_CLASS1(ParticleMesh, Mesh);

	ParticleMesh::ParticleMesh(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
	: Mesh(context, arena, hMyself)
	, m_hMyself(hMyself)
	, m_eyePos(Vector3(0,0,0))
	{
		m_loaded = false;
	}


	void ParticleMesh::addDefaultComponents()
	{
		

		//add this handler before Mesh's handlers so we can intercept draw and modify transform
		PE_REGISTER_EVENT_HANDLER(Events::Event_UPDATE, ParticleMesh::do_UPDATE);
		PE_REGISTER_EVENT_HANDLER(Events::Event_GATHER_DRAWCALLS, ParticleMesh::do_GATHER_DRAWCALLS);
		

		Mesh::addDefaultComponents();
		
		m_eyePos = Vector3(0,0,0);
		
	}
  //********************//
 //** INITIALIZATION **//
//********************//
	void ParticleMesh::loadFromFile_needsRC(Particle& particle, int &threadOwnershipMask)
	{
		const char* FileName = particle.GetFilename();//"EnergyParticle.dds";//particle.Filename;
		const char* Package = particle.GetPackage();//"Default";//particle.Package;
		PrimitiveTypes::Float32 Width = particle.Width;
		PrimitiveTypes::Float32 Height = particle.Height;

		const char* techName = 0;
		//if (drawType == Overlay2D_3DPos || drawType == Overlay2D)
		//	tech = "StdMesh_2D_Diffuse_A_RGBIntensity_Tech";
		//if (drawType == InWorldFacingCamera)
		techName = "StdMesh_Diffuse_Tech"; //HARDCODED!"StdMesh_2D_Diffuse_A_RGBIntensity_Tech";//
		

		if (!m_meshCPU.isValid())
		{
			m_meshCPU = Handle("MeshCPU ParticleMesh", sizeof(MeshCPU));
			new (m_meshCPU) MeshCPU(*m_pContext, m_arena);
		}
		MeshCPU &mcpu = *m_meshCPU.getObject<MeshCPU>();

		if (!m_loaded)
		{
			mcpu.createBillboardMeshWithColorTexture(FileName, Package, Width, Height, SamplerState_NotNeeded); //HardCoded sampler state, needs to change for mip-map particles
		}
	
		// this will cause not using the vertex buffer manager
		//so that engine always creates a new vertex buffer gpu and doesn't try to find and
		//existing one
		mcpu.m_manualBufferManagement = true;

		//Get the various buffers for the mesh
		PositionBufferCPU *pVB = mcpu.m_hPositionBufferCPU.getObject<PositionBufferCPU>();
		TexCoordBufferCPU *pTCB = mcpu.m_hTexCoordBufferCPU.getObject<TexCoordBufferCPU>();
		NormalBufferCPU *pNB = mcpu.m_hNormalBufferCPU.getObject<NormalBufferCPU>();
		IndexBufferCPU *pIB = mcpu.m_hIndexBufferCPU.getObject<IndexBufferCPU>();
		//reset values
		pVB->m_values.reset(4 * 3); // 4 verts * (x,y,z)
		pTCB->m_values.reset(4 * 2);
		pNB->m_values.reset(4 * 3);
		pIB->m_values.reset(6); // 2 tris

		pIB->m_indexRanges[0].m_start = 0;	//0
		pIB->m_indexRanges[0].m_end = 5;	//(len*6) * 2
		pIB->m_indexRanges[0].m_minVertIndex = 0;	//0
		pIB->m_indexRanges[0].m_maxVertIndex = 3;	// (len*4) - 1

		pIB->m_minVertexIndex = pIB->m_indexRanges[0].m_minVertIndex;
		pIB->m_maxVertexIndex = pIB->m_indexRanges[0].m_maxVertIndex;

		// Member variables for this mesh
		m_Width = (float) Width;
		m_Height = (float) Height;

		// time to fill in the buffers
		float XExtent = Width/2.f;
		float YExtent = Height/2.f;

		//Mesh Vertices
		pVB->m_values.add(-3, 3, 0);	//-XExtent, YExtent, 0);	// top left
		pVB->m_values.add( 3, 3, 0);	//XExtent, YExtent, 0);	// top right
		pVB->m_values.add( 3,-3, 0);	//XExtent,-YExtent, 0);	// bot right
		pVB->m_values.add(-3,-3, 0);	//-XExtent,-YExtent, 0);	// bot left

		//Mesh vertex buffers
		pIB->m_values.add(0, 1, 2); //first tri tex-coord buffer
		pIB->m_values.add(2, 3, 0); //second tri tex-coord buffer

		//Texture Co-ord buffer
		pTCB->m_values.add(0.6f, 0.4f); // top left
		pTCB->m_values.add(0.4f, 0.4f); // top right
		pTCB->m_values.add(0.4f, 0.6f); // bot right
		pTCB->m_values.add(0.6f, 0.6f); // bot left
		
		//Normal buffers
		pNB->m_values.add(0, 0, 0);
		pNB->m_values.add(0, 0, 0);
		pNB->m_values.add(0, 0, 0);
		pNB->m_values.add(0, 0, 0);

		if (!m_loaded)
		{
			// first time creating gpu mesh
			loadFromMeshCPU_needsRC(mcpu, threadOwnershipMask);

			// Not sure how this part works, but I think it takes care of itself through inheritance
			if (techName)
			{
				Handle hEffect = EffectManager::Instance()->getEffectHandle(techName);

				for (unsigned int imat = 0; imat < m_effects.m_size; imat++)
				{
					if (m_effects[imat].m_size)
						m_effects[imat][0] = hEffect;
				}
			}
			m_loaded = true;
		}
		else
		{
			//just need to update vertex buffers gpu
			updateGeoFromMeshCPU_needsRC(mcpu, threadOwnershipMask);
		}
	}

  //*************//
 //** SETTERS **//
//*************//

  //************//
 //** EVENTS **//
//************//
	void ParticleMesh::do_UPDATE(Events::Event* pEvt)
	{
		PE::Events::Event_UPDATE *pRealEvt = (PE::Events::Event_UPDATE *)(pEvt);
		float frameTime = pRealEvt->m_frameTime;

		//we have to update ALL buffers since we will be writing to the GPU
		//if we don't keep their scaling proportional, we will get array axis
		//errors
		CameraSceneNode *pcam = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
		m_eyePos = pcam->m_worldTransform.getPos();

		UpdatePositionBuffer(m_eyePos, frameTime);
		UpdateTextCoordBuffer(frameTime);
		UpdateIndexBuffer(frameTime);
		UpdateNormalBuffers(frameTime);
	}

	void ParticleMesh::do_GATHER_DRAWCALLS(Events::Event *pEvt)
	{
		Events::Event_PRE_GATHER_DRAWCALLS *pDrawEvent = NULL;
		pDrawEvent = (Events::Event_PRE_GATHER_DRAWCALLS *)(pEvt);
		//m_eyePos = pDrawEvent->m_eyePos;
	}

	void ParticleMesh::PostPreDraw(int &ThreadOwnershipMask)
	{
		//Finally we get to update our mesh's GPU buffer
		int numActiveParticles = particleArray->m_size;
		bool bParticlesActive = ( numActiveParticles > 0);
		if (bParticlesActive)
		{
			MeshCPU* pMeshCPU = m_meshCPU.getObject<MeshCPU>();
			updateGeoFromMeshCPU_needsRC((*pMeshCPU), ThreadOwnershipMask);
		}
	}
  //************//
 //** UPDATE **//
//************//
	void ParticleMesh::UpdatePositionBuffer(Vector3& eyePos, PrimitiveTypes::Float32 frameTime)
	{
		int numActiveParticles = particleArray->m_size;
		//get the Position buffer we will be tweaking
		MeshCPU* mcpu = m_meshCPU.getObject<MeshCPU>();
		Handle hPB = mcpu->m_hPositionBufferCPU;
		PositionBufferCPU* pPB = hPB.getObject<PositionBufferCPU>();

		//reset the buffer to the appropriate size
		PrimitiveTypes::Int32 numVertsPerParticle = 4;
		PrimitiveTypes::Int32 numFloatsPerVert = 3;
		pPB->m_values.reset(numActiveParticles * numVertsPerParticle * numFloatsPerVert);

		Matrix4x4 localToWorld;
		//reassign mesh positions
		for (int i = 0; i < numActiveParticles; i++)
		{
			Particle p = (*particleArray)[i];
			Vector3 pos = p.Position;
			PrimitiveTypes::Float32 w = (*particleArray)[i].Width;
			PrimitiveTypes::Float32 h = (*particleArray)[i].Height;
			//get the points in local space
			Vector3 p1 = Vector3(-w/2, h/2, 0);
			Vector3 p2 = Vector3( w/2, h/2, 0);
			Vector3 p3 = Vector3( w/2,-h/2, 0);
			Vector3 p4 = Vector3(-w/2,-h/2, 0);
			//create the local to world transform
			localToWorld.loadIdentity();
			localToWorld.setPos(pos);

			localToWorld.setN(Vector3(0,0,1)); //set local space to forward
			localToWorld.turnTo(eyePos);
			
			//ANGLED FIREWORKS BEGIN
			// TEST CODE:: ROTATE ANGLE OF IMAGE
			//float velocityX = (p.Velocity).m_x;
			//float velocityY = (p.Velocity).m_y;
			//float velocityZ = (p.Velocity).m_z;
			//float opposite = velocityX;
			//float adjacent = sqrt((velocityX*velocityX) + (velocityZ*velocityZ));
			//float mag = (p.Velocity).length();
			////float theta = atan(opposite/adjacent);
			//float theta = asin(velocityY/mag);
			//theta *=1.4;
			////if (velocityY < 0) { theta *= -1;}
			////Quaternion q = Quaternion(Vector3(0,0,1), theta);
			////now apply the rotation of the quaternion
			///*p1 = rotateCCW(q, p1);
			//p2 = rotateCCW(q, p2);
			//p3 = rotateCCW(q, p3);
			//p4 = rotateCCW(q, p4);*/
			//Matrix3x3 rot = Matrix3x3(RotateZ, theta);
			//p1 = rot * p1;
			//p2 = rot * p2;
			//p3 = rot * p3;
			//p4 = rot * p4;
			//END ANGLED FIREWORKS

			//apply the transform to each vertex
			p1 = localToWorld * p1;
			p2 = localToWorld * p2;
			p3 = localToWorld * p3;
			p4 = localToWorld * p4;



			//now place in the position buffer
			pPB->add3Floats(p1.m_x, p1.m_y, p1.m_z);
			pPB->add3Floats(p2.m_x, p2.m_y, p2.m_z);
			pPB->add3Floats(p3.m_x, p3.m_y, p3.m_z);
			pPB->add3Floats(p4.m_x, p4.m_y, p4.m_z);
		}
	}
	
	void ParticleMesh::UpdateTextCoordBuffer(PrimitiveTypes::Float32 frameTime)
	{
		int numActiveParticles = (*particleArray).m_size;
		//get the texture buffer we will be tweaking
		MeshCPU* mcpu = m_meshCPU.getObject<MeshCPU>();
		Handle hTCB = mcpu->m_hTexCoordBufferCPU;
		TexCoordBufferCPU* pTCB = hTCB.getObject<TexCoordBufferCPU>();

		//reset the buffef to the appropriate size
		PrimitiveTypes::Int32 numPointsPerParticle = 4;
		PrimitiveTypes::Int32 numCoordsPerPoint = 2;
		pTCB->m_values.reset(numActiveParticles * numPointsPerParticle * numCoordsPerPoint);
		/* THE OLD WAF
		//reassign texture coordinate values
		for (int i = 0; i < numActiveParticles; i++)
		{
			Particle p = (*particleArray)[i];
			PrimitiveTypes::Int32 curCycle = p.GetCurrentAnimCycle();
			pTCB->m_values.add(0.0,0.0);
			pTCB->m_values.add(1.0,0.0);
			pTCB->m_values.add(1.0,1.0);
			pTCB->m_values.add(0.0,1.0);
		}
		*/

		//LET THEIR BE PARTICLE ANIMATION THE MASTER OF THIS ENGINE SPOKE
		if ( numActiveParticles > 0 )
		{	// get particle anim cycle data
			Particle					pParticle	= particleArray->getByIndexUnchecked(0);
			PrimitiveTypes::Int32		numRows		= pParticle.GetNumRows();
			PrimitiveTypes::Int32		numColumns	= pParticle.GetNumColumns();
			PrimitiveTypes::Float32		delU		= 1.f/numRows;
			PrimitiveTypes::Float32		delV		= 1.f/numColumns;
		
			//now, for each particle, update the correct texture coordinates
			for (int i = 0; i < numActiveParticles; i++)
			{
				Particle p = (*particleArray)[i];
				PrimitiveTypes::Int32 curCycle = p.GetCurrentAnimCycle();

				PrimitiveTypes::Int32 curColumn = curCycle / numColumns;
				PrimitiveTypes::Int32 curRow	= curCycle - (curColumn * numColumns);

				//calculate the appropriate texture coordinates
				PrimitiveTypes::Float32 V_min = 0.f + (curColumn * delV);
				PrimitiveTypes::Float32 V_max = 0.f + ((curColumn+1)*delV);
				PrimitiveTypes::Float32 U_min = 0.f + (curRow * delU);
				PrimitiveTypes::Float32 U_max = 0.f + ((curRow+1)* delU);

				//now update the texture coordinate buffer with the appropriate values
				pTCB->m_values.add( U_min, V_min);
				pTCB->m_values.add( U_max, V_min);
				pTCB->m_values.add( U_max, V_max);
				pTCB->m_values.add(	U_min, V_max);
			}
		}
	}

	void ParticleMesh::UpdateIndexBuffer(PrimitiveTypes::Float32 frameTime)
	{
		/** we really don't need to update this, we just need to change the size */
		PrimitiveTypes::UInt16 numActiveParticles = (*particleArray).m_size;
		// get the index buffer we will be tweaking
		MeshCPU* mcpu = m_meshCPU.getObject<MeshCPU>();
		Handle hIB = mcpu->m_hIndexBufferCPU;
		IndexBufferCPU* pIB = hIB.getObject<IndexBufferCPU>();

		//set the buffef to the appropriate size
		//assert(pIB->m_values.m_capacity >= numActiveParticles ); /* index buffer was initialized to a smaller size than max num particles */
		pIB->m_values.m_size = numActiveParticles;

		PrimitiveTypes::Int32 numPointsPerParticle = 6;
		PrimitiveTypes::Int32 numCoordsPerPoint = 1;
		pIB->m_values.reset(numActiveParticles * numPointsPerParticle * numCoordsPerPoint);

		for (PrimitiveTypes::UInt16 i = 0; i < numActiveParticles; i++)
		{
			//first tri
			PrimitiveTypes::UInt16 indexOfFirstPoint = i * 4;

			pIB->m_values.add(indexOfFirstPoint);
			pIB->m_values.add(indexOfFirstPoint+2);
			pIB->m_values.add(indexOfFirstPoint+1);
			//second tri
			pIB->m_values.add(indexOfFirstPoint);
			pIB->m_values.add(indexOfFirstPoint+3);
			pIB->m_values.add(indexOfFirstPoint+2);
		}
		pIB->m_indexRanges[0].m_start = 0;
		pIB->m_indexRanges[0].m_end = numActiveParticles * 6 - 1;
		pIB->m_indexRanges[0].m_minVertIndex = 0;
		pIB->m_indexRanges[0].m_maxVertIndex = numActiveParticles * 4 - 1;

		pIB->m_minVertexIndex = 0;
		pIB->m_maxVertexIndex = numActiveParticles * 4 - 1;
	}

	void ParticleMesh::UpdateNormalBuffers( PrimitiveTypes::Float32 frameTime)
	{
		int numActiveParticles = particleArray->m_size;
		//get the Position buffer we will be tweaking
		MeshCPU* mcpu = m_meshCPU.getObject<MeshCPU>();
		Handle hNB = mcpu->m_hNormalBufferCPU;
		NormalBufferCPU* pNB = hNB.getObject<NormalBufferCPU>();

		//assert(pNB->m_values.m_capacity >= numActiveParticles ); /* index buffer was initialized to a smaller size than max num particles */
		//pNB->m_values.m_size = numActiveParticles;
		// reset the buffer to the appropriate size
		PrimitiveTypes::Int32 numPointsPerParticle = 4;
		PrimitiveTypes::Int32 numValuesPerPoint = 3;
		pNB->m_values.reset(numActiveParticles * numPointsPerParticle * numValuesPerPoint);

		

		for (int i = 0; i < numActiveParticles; ++i)
		{

			//get the particle data
			Vector4 col = (*particleArray)[i].Color;
			//our shader currently does not use this so zero for all
			pNB->m_values.add(col.m_x, col.m_y, col.m_z);
			pNB->m_values.add(col.m_x, col.m_y, col.m_z);
			pNB->m_values.add(col.m_x, col.m_y, col.m_z);
			pNB->m_values.add(col.m_x, col.m_y, col.m_z);
		}
	}

}; // namespace Components
}; // namespace PE