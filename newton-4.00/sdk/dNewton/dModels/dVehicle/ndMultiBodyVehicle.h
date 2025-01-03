/* Copyright (c) <2003-2022> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __ND_MULTIBODY_VEHICLE_H__
#define __ND_MULTIBODY_VEHICLE_H__

#include "ndNewtonStdafx.h"
#include "dModels/ndModel.h"
#include "dIkSolver/ndIkSolver.h"
#include "dModels/ndModelArticulation.h"

class ndWorld;
class ndBodyDynamic;
class ndMultiBodyVehicleMotor;
class ndMultiBodyVehicleGearBox;
class ndMultiBodyVehicleTireJoint;
class ndMultiBodyVehicleTorsionBar;
class ndMultiBodyVehicleDifferential;
class ndMultiBodyVehicleTireJointInfo;
class ndMultiBodyVehicleDifferentialAxle;

#define dRadPerSecToRpm ndFloat32(9.55f)

class ndMultiBodyVehicle : public ndModelArticulation
{
	public:
	//class ndTireContactPair
	//{
	//	public:
	//	ndContact* m_contact;
	//	ndMultiBodyVehicleTireJoint* m_tireJoint;
	//};
	//
	class ndDownForce
	{
		public:
		class ndSpeedForcePair
		{
			public:
			ndFloat32 m_speed;
			ndFloat32 m_forceFactor;
			ndFloat32 m_aerodynamicDownforceConstant;
			friend class ndDownForce;
		};
	
		ndDownForce();
		ndFloat32 GetDownforceFactor(ndFloat32 speed) const;
	
		private:
		ndFloat32 CalculateFactor(const ndSpeedForcePair* const entry) const;
	
		ndFloat32 m_gravity;
		ndFloat32 m_suspensionStiffnessModifier;
		ndSpeedForcePair m_downForceTable[5];
		friend class ndMultiBodyVehicle;
		friend class ndMultiBodyVehicleTireJoint;
	};

	D_CLASS_REFLECTION(ndMultiBodyVehicle, ndModelArticulation)

	D_NEWTON_API ndMultiBodyVehicle(const ndVector& frontDir, const ndVector& upDir);
	D_NEWTON_API virtual ~ndMultiBodyVehicle ();


	D_NEWTON_API void AddChassis(const ndSharedPtr<ndBody>& chassis);

	D_NEWTON_API ndShapeInstance CreateTireShape(ndFloat32 radius, ndFloat32 width) const;
	D_NEWTON_API ndMultiBodyVehicleTireJoint* AddTire(const ndMultiBodyVehicleTireJointInfo& desc, const ndSharedPtr<ndBody>& tire);
	D_NEWTON_API ndMultiBodyVehicleDifferential* AddDifferential(ndFloat32 mass, ndFloat32 radius, ndMultiBodyVehicleTireJoint* const leftTire, ndMultiBodyVehicleTireJoint* const rightTire, ndFloat32 slipOmegaLock);
	D_NEWTON_API ndMultiBodyVehicleDifferential* AddDifferential(ndFloat32 mass, ndFloat32 radius, ndMultiBodyVehicleDifferential* const leftDifferential, ndMultiBodyVehicleDifferential* const rightDifferential, ndFloat32 slipOmegaLock);

#if 0
	virtual void OnAddToWorld() { ndAssert(0); }
	virtual void OnRemoveFromToWorld() { ndAssert(0); }

	D_NEWTON_API ndFloat32 GetSpeed() const;
	

	D_NEWTON_API ndMultiBodyVehicleMotor* AddMotor(ndFloat32 mass, ndFloat32 radius);
	D_NEWTON_API ndMultiBodyVehicleGearBox* AddGearBox(ndMultiBodyVehicleDifferential* const differential);
	
	D_NEWTON_API ndMultiBodyVehicleTireJoint* AddAxleTire(const ndMultiBodyVehicleTireJointInfo& desc, ndBodyKinematic* const tire, ndBodyKinematic* const axleBody);
	
	D_NEWTON_API ndMultiBodyVehicleTorsionBar* AddTorsionBar(ndBodyKinematic* const sentinel);

	D_NEWTON_API void SetVehicleSolverModel(bool hardJoint);

	D_NEWTON_API ndMultiBodyVehicle* GetAsMultiBodyVehicle();

	private:
	void ApplyAerodynamics();
	void ApplyalignmentAndBalancing();
	void ApplyTireModel(ndFloat32 timestep);
	void ApplyTireModel(ndFloat32 timestep, ndTireContactPair* const tires, ndInt32 tireCount);
	void ApplyVehicleDynamicControl(ndFloat32 timestep, ndTireContactPair* const tires, ndInt32 tireCount);

	void CalculateNormalizedAlgningTorque(ndMultiBodyVehicleTireJoint* const tire, ndFloat32 sideSlipTangent) const;
	void CoulombTireModel(ndMultiBodyVehicleTireJoint* const tire, ndContactMaterial& contactPoint, ndFloat32 timestep) const;
	void BrushTireModel(ndMultiBodyVehicleTireJoint* const tire, ndContactMaterial& contactPoint, ndFloat32 timestep) const;
	void PacejkaTireModel(ndMultiBodyVehicleTireJoint* const tire, ndContactMaterial& contactPoint, ndFloat32 timestep) const;
	void CoulombFrictionCircleTireModel(ndMultiBodyVehicleTireJoint* const tire, ndContactMaterial& contactPoint, ndFloat32 timestep) const;

	protected:
	bool isActive() const;
	virtual void ApplyInputs(ndWorld* const world, ndFloat32 timestep);
	//D_NEWTON_API virtual void RemoveFromToWorld();
	//D_NEWTON_API virtual void AddToWorld(ndWorld* const world);

	D_NEWTON_API virtual void Debug(ndConstraintDebugCallback& context) const;
	D_NEWTON_API virtual void Update(ndWorld* const world, ndFloat32 timestep);
	D_NEWTON_API virtual void PostUpdate(ndWorld* const world, ndFloat32 timestep);
#endif
	private:
	ndBodyKinematic* CreateInternalBodyPart(ndFloat32 mass, ndFloat32 radius) const;

	protected:
	ndMatrix m_localFrame;
	ndBodyDynamic* m_chassis;
	//ndIkSolver m_invDynamicsSolver;
	ndShapeChamferCylinder* m_tireShape;

	//ndReferencedObjects<ndBody> m_internalBodies;
	//ndSharedPtr<ndMultiBodyVehicleMotor> m_motor;
	//ndSharedPtr<ndMultiBodyVehicleGearBox> m_gearBox;
	//ndSharedPtr<ndMultiBodyVehicleTorsionBar> m_torsionBar;
	ndList<ndMultiBodyVehicleTireJoint*> m_tireList;
	//ndReferencedObjects<ndMultiBodyVehicleDifferentialAxle> m_axleList;
	//ndReferencedObjects<ndMultiBodyVehicleDifferential> m_differentialList;
	ndDownForce m_downForce;
	
	//friend class ndMultiBodyVehicleMotor;
	//friend class ndMultiBodyVehicleGearBox;
	friend class ndMultiBodyVehicleTireJoint;
	//friend class ndMultiBodyVehicleTorsionBar;
};


#endif