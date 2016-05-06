
#pragma once
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"

namespace PE
{
namespace Components
{

	enum PEmitterType
	 {
 		Instant,
 		Bursts,
 		Continual,
 		Periodic
	 };
	enum ParticleType
	 {
 		Sparks,
 		WaterSpray,
 		Lightning,
 		Snow,
 		Fireworks
	 };

	enum ParticleState
	 {
 		Active,
 		Deactivating,
 		Inactive
	 };

	/**
	 * Enums for the different types of emitters blend modes
	 */
	 enum PEmitterBlendMode
	 {
 		Plain,
 		Blur /** stretches the particle to span distance. Useful for sparks */
	 };

	 struct AABB
	 {
		Vector4 Center;
		PrimitiveTypes::Int32 XExtents;
		PrimitiveTypes::Int32 YExtents;
		PrimitiveTypes::Int32 ZExtents;
	 };

};/* end namespace Components */
};/* end namespace PE */