#pragma once

#include "PrimeEngine/Particles/Particles/Particle.h"

namespace PE
{
namespace Components
{
	struct SmokeParticle : public Particle
	{
		virtual char* GetPackage() override
		{ 
			return "Default";
		}
		virtual char* GetFilename() override
		{
			return "SmokeParticle.dds";
		}

		virtual PrimitiveTypes::Int32 GetNumAnimationCycles() override
		{
			return 16;
		}
		virtual PrimitiveTypes::Int32 GetNumRows() override
		{
			return 4;
		}
		virtual PrimitiveTypes::Int32 GetNumColumns() override
		{
			return 4;
		}
		virtual PrimitiveTypes::Int32 GetCurrentAnimCycle() override
		{
			PrimitiveTypes::Float32 percentOfLifeLived = (TimeLived / Lifetime);
			PrimitiveTypes::Int32 CurrentAnimCycle = ceil(percentOfLifeLived * GetNumAnimationCycles()) - 1;
			CurrentAnimCycle = (CurrentAnimCycle < 0) ?  0:CurrentAnimCycle;
			return CurrentAnimCycle;
		}
	};
};
};