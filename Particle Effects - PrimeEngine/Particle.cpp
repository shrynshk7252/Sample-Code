#include "Particle.h"

namespace PE
{
namespace Components
{
	//**************//
	//** UPDATERS **//
	//**************//
	void Particle::Update(const float& DeltaTime)
	{
		UpdateTimeLived(DeltaTime);
		if ( (this->TimeLived) > (this->Lifetime) )
		{
			//particle will be deleted
			//don't waste time updating the others
			return; 
		}
		UpdateVelocity(DeltaTime);
		UpdatePosition(DeltaTime);
		UpdateAngularVelocity(DeltaTime);
		UpdateColor(DeltaTime);
		UpdateWidth(DeltaTime);
		UpdateHeight(DeltaTime);
	}

	void Particle::UpdateVelocity(const float& DeltaTime)
	{
		this->Velocity += (this->Acceleration) * DeltaTime;
	}

	void Particle::UpdatePosition(const float& DeltaTime)
	{
		this->Position += (this->Velocity) * DeltaTime;
	}
	
	void Particle::UpdateColor(const float& DeltaTime)
	{
		this->Color = (this->Color) + ((this->DeltaColor) * DeltaTime);
	}

	void Particle::UpdateAngularVelocity(const float& DeltaTime)
	{
		// Currently no update for angular velocity
	}

	void Particle::UpdateWidth(const float& DeltaTime)
	{
		// for default particle width is static
		//Width = Velocity.lengthSqr()/5.f;
		return;
	}

	void Particle::UpdateHeight(const float& DeltaTime)
	{
		// for default particle height is static
		return;
	}

	void Particle::UpdateTimeLived(const float& DeltaTime)
	{
		this->TimeLived += DeltaTime;
	}
	
}; //end namespace Components
}; //end namespace PE