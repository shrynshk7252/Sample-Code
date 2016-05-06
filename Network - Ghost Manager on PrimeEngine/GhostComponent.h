#ifndef _PE_GhostComponent_
#define _PE_GhostComponent_

#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/Utils/StringOps.h"

#include "PrimeEngine/Networking/NetworkContext.h"
#include "PrimeEngine/Utils/PEClassDecl.h"
#include "PrimeEngine/Math/Matrix4x4.h"



namespace PE {
	//struct GameContext;
	namespace Components {
		//struct NetworkManager;


		struct GhostComponent : public Component
		{

			PE_DECLARE_CLASS(GhostComponent);

			// Constructor -------------------------------------------------------------
			GhostComponent(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself);

			virtual ~GhostComponent() {}

			// Component ------------------------------------------------------------

			virtual void addDefaultComponents();

			int m_mainClientID;
			int m_ghostID;
			int sizeOfStateMask;
			int stateMask;
			bool gactive;
			int gtype; // 0: Player 1:zombie
			int gstatus; //0: new 1: delete 2:update!
			Matrix4x4 gm_base;
			bool playerReady;


			// used in all classes that can be created over network
			// events use inly this method and no other ones since events are instantaneous objects
			virtual int packData(char *pDataStream){ assert(!"This function is not overridden by this class and should not be called! Make sure to implement override of this function in current class"); return 0; }
			virtual void useMatrix(Matrix4x4 *m_b){ assert(!"This function is not overridden by this class and should not be called! Make sure to implement override of this function in current class");  }

			// usually used in game classes
			virtual int unpackData(char *pDataStream){ assert(!"This function is not overridden by this class and should not be called! Make sure to implement override of this function in current class"); return 0; }

			
		};

	};
}; // namespace PE

#endif
