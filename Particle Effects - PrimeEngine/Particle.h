
#pragma once

#include "PrimeEngine/Math/Vector4.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"

namespace PE
{
namespace Components
{
	/** basic data struct for Particles */
	struct Particle
	{
		//PrimitiveTypes::Int32	UVAnimationCycles;	/** number of uv animation cycles */
		//PrimitiveTypes::Int32	UVNumXTiles;		/** number of horizontal tiles in the mesh texture */
		//PrimitiveTypes::Int32	UVNumYTiles;		/** number of vertical tiles in the mesh texture */
		//PrimitiveTypes::Int32	CurrentAnimCycle;

		Vector3				Position;
		Vector3 			Velocity;		/** Current velocity of the particle */
		Vector3 			Acceleration;	/** linear acceleration of the particle */
		Vector4				AngularVelocity;/** The Angular velocity of the particle */
		float				Speed;			/** The speed of the particle */
		Vector4 			Color;			/** Current Color of the particle */
		Vector4 			DeltaColor;		/**  Color Change Per update. This is prefered to interping to save calcs */
		short 				Flags;			/** flags for tracking if particle is alive or dead. */
		float 				Lifetime;		/** Time the particle gets to live*/
		float 				TimeLived;		/** Time the particle has been alive */

		float				Width;
		float				Height;
		int					AnimationCycles;
		int					NumRows;
		int					NumColumns;

		//*************//
		//** GETTERS **//
		//*************//
		virtual char* GetFilename(){ return "";}
		virtual char* GetPackage(){ return "";}
		virtual PrimitiveTypes::Int32 GetNumAnimationCycles(){return AnimationCycles;}//return -1;}
		virtual PrimitiveTypes::Int32 GetNumRows(){return NumRows;}//return -1;}
		virtual PrimitiveTypes::Int32 GetNumColumns(){return NumColumns;}//return -1;}
		virtual PrimitiveTypes::Int32 GetCurrentAnimCycle(){
			//return -1;
			PrimitiveTypes::Float32 percentOfLifeLived = (TimeLived / Lifetime);
			PrimitiveTypes::Int32 CurrentAnimCycle = ceil(percentOfLifeLived * GetNumAnimationCycles()) - 1;
			CurrentAnimCycle = (CurrentAnimCycle < 0) ?  0:CurrentAnimCycle;
			return CurrentAnimCycle;
		}
		//*************//
		//** UPDATES **//
		//*************//
		/**
		 * @todo implement and integrate 
		 */
		virtual void Update(const float& DeltaTime);
				/**
		 * @todo implement and integrate 
		 */
		virtual void UpdateVelocity(const float& DeltaTime);
		/**
		 * @todo implement and integrate 
		 */
		virtual void UpdatePosition(const float& DeltaTime);
		/**
		 * @todo implement and integrate
		 */
		virtual void UpdateAngularVelocity(const float& DeltaTime);
		/**
		 * @todo implement and integrate
		 */
		virtual void UpdateWidth(const float& DeltaTime);
		/**
		 * @todo implement and integrate
		 */
		virtual void UpdateHeight(const float& DeltaTime);
		/**
		 * @todo implement and integrate
		 */
		virtual void UpdateColor(const float& DeltaTime);
		/**
		 * @todo implement and integrate
		 */
		virtual void UpdateTimeLived(const float& DeltaTime);

		
	};

//************************//
//** EXPLOSION PARTICLE **//
//************************//
	//struct ExplosionParticle: public Particle
	//{
	//	//const char* Filename = "Default";
	//	//const char* Package ;
	//	static const PrimitiveTypes::Int32	UVAnimationCycles = 4;	/** number of uv animation cycles */
	//	static const PrimitiveTypes::Int32	UVNumXTiles = 2;		/** number of horizontal tiles in the mesh texture */
	//	static const PrimitiveTypes::Int32	UVNumYTiles = 2;		/** number of vertical tiles in the mesh texture */
	//	
	//	virtual char* GetPackage() override
	//	{ 
	//		return "Default";
	//	}
	//	virtual char* GetFilename() override
	//	{
	//		return "Explosion.dds";
	//	}
	//};
};/* end namespace Components */
};/* end namespace PE */