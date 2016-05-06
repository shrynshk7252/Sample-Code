#pragma once

#include "PrimeEngine/Particles/Particles/Particle.h"



namespace PE
{
namespace Components
{
	struct EnergyParticle : public Particle
	{		
		virtual char* GetPackage() override
		{ 
			return "Default";
		}
		virtual char* GetFilename() override
		{
			return "circle.dds";
		}

		virtual PrimitiveTypes::Int32 GetNumAnimationCycles() override
		{
			return 1;
		}
		virtual PrimitiveTypes::Int32 GetNumRows() override
		{
			return 1;
		}
		virtual PrimitiveTypes::Int32 GetNumColumns() override
		{
			return 1;
		}
		virtual PrimitiveTypes::Int32 GetCurrentAnimCycle() override
		{
			return 0;
		}
	};
}; // end namespace Components
}; // end namespace PE