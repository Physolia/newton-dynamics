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

#include "ndCoreStdafx.h"
#include "ndNewtonStdafx.h"
#include "ndWorld.h"
#include "ndBodyDynamic.h"
#include "ndBodyKinematic.h"
#include "ndMultiBodyVehicle.h"
#include "dJoints/ndJointHinge.h"
#include "ndMultiBodyVehicleMotor.h"
#include "ndMultiBodyVehicleGearBox.h"
#include "ndMultiBodyVehicleTireJoint.h"
#include "ndMultiBodyVehicleTorsionBar.h"
#include "ndMultiBodyVehicleDifferential.h"
#include "ndMultiBodyVehicleDifferentialAxle.h"

#define D_MAX_CONTACT_SPEED_TRESHOLD  ndFloat32 (0.25f)
#define D_MAX_CONTACT_PENETRATION	  ndFloat32 (1.0e-2f)
#define D_MIN_CONTACT_CLOSE_DISTANCE2 ndFloat32 (5.0e-2f * 5.0e-2f)

ndMultiBodyVehicle::ndDownForce::ndDownForce()
	:m_gravity(ndFloat32(-10.0f))
	,m_suspensionStiffnessModifier(ndFloat32(1.0f))
{
	m_downForceTable[0].m_speed = ndFloat32(0.0f) * ndFloat32(0.27f);
	m_downForceTable[0].m_forceFactor = 0.0f;
	m_downForceTable[0].m_aerodynamicDownforceConstant = ndFloat32(0.0f);

	m_downForceTable[1].m_speed = ndFloat32(30.0f) * ndFloat32(0.27f);
	m_downForceTable[1].m_forceFactor = 1.0f;
	m_downForceTable[1].m_aerodynamicDownforceConstant = CalculateFactor(&m_downForceTable[0]);

	m_downForceTable[2].m_speed = ndFloat32(60.0f) * ndFloat32(0.27f);
	m_downForceTable[2].m_forceFactor = 1.6f;
	m_downForceTable[2].m_aerodynamicDownforceConstant = CalculateFactor(&m_downForceTable[1]);

	m_downForceTable[3].m_speed = ndFloat32(140.0f) * ndFloat32(0.27f);
	m_downForceTable[3].m_forceFactor = 3.0f;
	m_downForceTable[3].m_aerodynamicDownforceConstant = CalculateFactor(&m_downForceTable[2]);

	m_downForceTable[4].m_speed = ndFloat32(1000.0f) * ndFloat32(0.27f);
	m_downForceTable[4].m_forceFactor = 3.0f;
	m_downForceTable[4].m_aerodynamicDownforceConstant = CalculateFactor(&m_downForceTable[3]);
}

ndFloat32 ndMultiBodyVehicle::ndDownForce::CalculateFactor(const ndSpeedForcePair* const entry0) const
{
	const ndSpeedForcePair* const entry1 = entry0 + 1;
	ndFloat32 num = ndMax(entry1->m_forceFactor - entry0->m_forceFactor, ndFloat32(0.0f));
	ndFloat32 den = ndMax(ndAbs(entry1->m_speed - entry0->m_speed), ndFloat32(1.0f));
	return num / (den * den);
}

ndFloat32 ndMultiBodyVehicle::ndDownForce::GetDownforceFactor(ndFloat32 speed) const
{
	ndAssert(speed >= ndFloat32(0.0f));
	ndInt32 index = 0;
	for (ndInt32 i = sizeof(m_downForceTable) / sizeof(m_downForceTable[0]) - 1; i; i--)
	{
		if (m_downForceTable[i].m_speed <= speed)
		{
			index = i;
			break;
		}
	}

	ndFloat32 deltaSpeed = speed - m_downForceTable[index].m_speed;
	ndFloat32 downForceFactor = m_downForceTable[index].m_forceFactor + m_downForceTable[index + 1].m_aerodynamicDownforceConstant * deltaSpeed * deltaSpeed;
	return downForceFactor * m_gravity;
}


#if 0
ndMultiBodyVehicle* ndMultiBodyVehicle::GetAsMultiBodyVehicle()
{
	return this;
}



//ndMultiBodyVehicleTorsionBar* ndMultiBodyVehicle::AddTorsionBar(ndBodyKinematic* const sentinel)
ndMultiBodyVehicleTorsionBar* ndMultiBodyVehicle::AddTorsionBar(ndBodyKinematic* const)
{
	ndAssert(0);
	return nullptr;

	//m_torsionBar = ndSharedPtr<ndMultiBodyVehicleTorsionBar>(new ndMultiBodyVehicleTorsionBar(this, sentinel));
	//return *m_torsionBar;
}

//void ndMultiBodyVehicle::ApplyVehicleDynamicControl(ndFloat32 timestep, ndTireContactPair* const tireContacts, ndInt32 contactCount)
void ndMultiBodyVehicle::ApplyVehicleDynamicControl(ndFloat32, ndTireContactPair* const, ndInt32)
{
	//contactCount = 0;
	//for (ndInt32 i = contactCount - 1; i >= 0; --i)
	//{
	//	ndContact* const contact = tireContacts[i].m_contact;
	//	ndMultiBodyVehicleTireJoint* const tire = tireContacts[i].m_tireJoint;
	//	ndContactPointList& contactPoints = contact->GetContactPoints();
	//	ndMatrix tireBasisMatrix(tire->GetLocalMatrix1() * tire->GetBody1()->GetMatrix());
	//	tireBasisMatrix.m_posit = tire->GetBody0()->GetMatrix().m_posit;
	//	for (ndContactPointList::ndNode* contactNode = contactPoints.GetFirst(); contactNode; contactNode = contactNode->GetNext())
	//	{
	//		ndContactMaterial& contactPoint = contactNode->GetInfo();
	//		ndFloat32 contactPathLocation = ndAbs(contactPoint.m_normal.DotProduct(tireBasisMatrix.m_front).GetScalar());
	//		// contact are consider on the contact patch strip only if the are less than 
	//		// 45 degree angle from the tire axle
	//		if (contactPathLocation < ndFloat32(0.71f))
	//		{
	//			// align tire friction direction
	//			const ndVector longitudinalDir(contactPoint.m_normal.CrossProduct(tireBasisMatrix.m_front).Normalize());
	//			const ndVector lateralDir(longitudinalDir.CrossProduct(contactPoint.m_normal));
	//
	//			contactPoint.m_dir1 = lateralDir;
	//			contactPoint.m_dir0 = longitudinalDir;
	//
	//			// check if the contact is in the contact patch,
	//			// the is the 45 degree point around the tire vehicle axis. 
	//			ndVector dir(contactPoint.m_point - tireBasisMatrix.m_posit);
	//			ndAssert(dir.DotProduct(dir).GetScalar() > ndFloat32(0.0f));
	//			ndFloat32 contactPatch = tireBasisMatrix.m_up.DotProduct(dir.Normalize()).GetScalar();
	//			if (contactPatch < ndFloat32(-0.71f))
	//			{
	//				switch (tire->m_frictionModel.m_frictionModel)
	//				{
	//					case ndTireFrictionModel::m_brushModel:
	//					{
	//						BrushTireModel(tire, contactPoint, timestep);
	//						break;
	//					}
	//
	//					case ndTireFrictionModel::m_pacejka:
	//					{
	//						PacejkaTireModel(tire, contactPoint, timestep);
	//						break;
	//					}
	//
	//					case ndTireFrictionModel::m_coulombCicleOfFriction:
	//					{
	//						CoulombFrictionCircleTireModel(tire, contactPoint, timestep);
	//						break;
	//					}
	//
	//					case ndTireFrictionModel::m_coulomb:
	//					default:
	//					{
	//						CoulombTireModel(tire, contactPoint, timestep);
	//						break;
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
}



//void ndMultiBodyVehicle::RemoveFromToWorld()
//{
//	if (m_world)
//	{
//		if (*m_motor)
//		{
//			m_world->RemoveJoint(*m_motor);
//		}
//		if (*m_gearBox)
//		{
//			m_world->RemoveJoint(*m_gearBox);
//		}
//		if (*m_torsionBar)
//		{
//			m_world->RemoveJoint(*m_torsionBar);
//		}
//
//		for (ndReferencedObjects<ndBody>::ndNode* node = m_internalBodies.GetFirst(); node; node = node->GetNext())
//		{
//			ndSharedPtr<ndBody>& ptr = node->GetInfo();
//			m_world->RemoveBody(*ptr);
//		}
//
//		for (ndReferencedObjects<ndMultiBodyVehicleTireJoint>::ndNode* node = m_tireList.GetFirst(); node; node = node->GetNext())
//		{
//			ndJointBilateralConstraint* const joint = *node->GetInfo();
//			m_world->RemoveJoint(joint);
//		}
//
//		for (ndReferencedObjects<ndMultiBodyVehicleDifferentialAxle>::ndNode* node = m_axleList.GetFirst(); node; node = node->GetNext())
//		{
//			ndJointBilateralConstraint* const joint = *node->GetInfo();
//			m_world->RemoveJoint(joint);
//		}
//
//		for (ndReferencedObjects<ndMultiBodyVehicleDifferential>::ndNode* node = m_differentialList.GetFirst(); node; node = node->GetNext())
//		{
//			ndJointBilateralConstraint* const joint = *node->GetInfo();
//			m_world->RemoveJoint(joint);
//		}
//	}
//
//	ndModel::RemoveFromToWorld();
//}
//
//void ndMultiBodyVehicle::AddToWorld(ndWorld* const world)
//{
//	ndAssert(0);
//	ndModel::AddToWorld(world);
//
//	if (*m_motor)
//	{
//		ndSharedPtr<ndJointBilateralConstraint>& motor = (ndSharedPtr<ndJointBilateralConstraint>&)m_motor;
//		world->AddJoint(motor);
//	}
//	if (*m_gearBox)
//	{
//		ndSharedPtr<ndJointBilateralConstraint>& gearBox = (ndSharedPtr<ndJointBilateralConstraint>&)m_gearBox;
//		world->AddJoint(gearBox);
//	}
//	if (*m_torsionBar)
//	{
//		ndSharedPtr<ndJointBilateralConstraint>& torsionBar = (ndSharedPtr<ndJointBilateralConstraint>&)m_torsionBar;
//		world->AddJoint(torsionBar);
//	}
//
//	for (ndReferencedObjects<ndBody>::ndNode* node = m_internalBodies.GetFirst(); node; node = node->GetNext())
//	{
//		ndSharedPtr<ndBody>& ptr = node->GetInfo();
//		world->AddBody(ptr);
//	}
//
//	for (ndReferencedObjects<ndMultiBodyVehicleTireJoint>::ndNode* node = m_tireList.GetFirst(); node; node = node->GetNext())
//	{
//		ndSharedPtr<ndJointBilateralConstraint>& joint = (ndSharedPtr<ndJointBilateralConstraint>&)node->GetInfo();
//		world->AddJoint(joint);
//	}
//
//	for (ndReferencedObjects<ndMultiBodyVehicleDifferentialAxle>::ndNode* node = m_axleList.GetFirst(); node; node = node->GetNext())
//	{
//		ndSharedPtr<ndJointBilateralConstraint>& joint = (ndSharedPtr<ndJointBilateralConstraint>&)node->GetInfo();
//		world->AddJoint(joint);
//	}
//
//	for (ndReferencedObjects<ndMultiBodyVehicleDifferential>::ndNode* node = m_differentialList.GetFirst(); node; node = node->GetNext())
//	{
//		ndSharedPtr<ndJointBilateralConstraint>& joint = (ndSharedPtr<ndJointBilateralConstraint>&)node->GetInfo();
//		world->AddJoint(joint);
//	}
//}


#endif


ndMultiBodyVehicle::ndMultiBodyVehicle()
	:ndModelArticulation()
	,m_localFrame(ndGetIdentityMatrix())
	,m_tireShape(new ndShapeChamferCylinder(ndFloat32(0.75f), ndFloat32(0.5f)))
	,m_downForce()
{
	m_motor = nullptr;
	m_gearBox = nullptr;
	m_chassis = nullptr;
	m_torsionBar = nullptr;

	m_tireShape->AddRef();
}

ndMultiBodyVehicle::~ndMultiBodyVehicle()
{
	m_tireShape->Release();
}

const ndMatrix& ndMultiBodyVehicle::GetLocalFrame() const
{
	return m_localFrame;
}

void ndMultiBodyVehicle::SetLocalFrame(const ndMatrix& localframe)
{
	m_localFrame.m_front = (localframe.m_front & ndVector::m_triplexMask).Normalize();
	m_localFrame.m_up = localframe.m_up & ndVector::m_triplexMask;
	m_localFrame.m_right = m_localFrame.m_front.CrossProduct(m_localFrame.m_up).Normalize();
	m_localFrame.m_up = m_localFrame.m_right.CrossProduct(m_localFrame.m_front).Normalize();
}

ndBodyDynamic* ndMultiBodyVehicle::GetChassis() const
{
	return m_chassis;
}

ndMultiBodyVehicleMotor* ndMultiBodyVehicle::GetMotor() const
{
	return m_motor;
}

ndMultiBodyVehicleGearBox* ndMultiBodyVehicle::GetGearBox() const
{
	return m_gearBox;
}

const ndList<ndMultiBodyVehicleTireJoint*>& ndMultiBodyVehicle::GetTireList() const
{
	return m_tireList;
}

ndFloat32 ndMultiBodyVehicle::GetSpeed() const
{
	const ndVector dir(m_chassis->GetMatrix().RotateVector(m_localFrame.m_front));
	const ndFloat32 speed = ndAbs(m_chassis->GetVelocity().DotProduct(dir).GetScalar());
	return speed;
}

void ndMultiBodyVehicle::AddChassis(const ndSharedPtr<ndBody>& chassis)
{
	AddRootBody(chassis);
	m_chassis = chassis->GetAsBodyDynamic();
	ndAssert(m_chassis);
}

void ndMultiBodyVehicle::SetVehicleSolverModel(bool hardJoint)
{
	ndAssert(m_chassis);
	ndJointBilateralSolverModel openLoopMode = hardJoint ? m_jointkinematicOpenLoop : m_jointIterativeSoft;
	for (ndNode* node = GetRoot()->GetFirstChild(); node; node = node->GetNext())
	{
		ndJointBilateralConstraint* const joint = *node->m_joint;
		const char* const className = joint->ClassName();
		if (!strcmp(className, "ndMultiBodyVehicleTireJoint") ||
			!strcmp(className, "ndMultiBodyVehicleDifferential") ||
			!strcmp(className, "ndMultiBodyVehicleMotor"))
		{
			joint->SetSolverModel(openLoopMode);
		}
	}
	
	ndJointBilateralSolverModel driveTrainMode = hardJoint ? m_jointkinematicCloseLoop : m_jointIterativeSoft;
	for (ndList<ndNode>::ndNode* node = m_closeLoops.GetFirst(); node; node = node->GetNext())
	{
		ndModelArticulation::ndNode& closeLoop = node->GetInfo();
		ndJointBilateralConstraint* const joint = *closeLoop.m_joint;
		const char* const clasName = joint->ClassName();
		if (strcmp(clasName, "ndMultiBodyVehicleDifferential") || strcmp(clasName, "ndMultiBodyVehicleGearBox"))
		{
			joint->SetSolverModel(driveTrainMode);
		}
	}
	
	if (m_torsionBar)
	{
		ndAssert(0);
		//m_torsionBar->SetSolverModel(driveTrainMode);
	}
}

ndMultiBodyVehicleTireJoint* ndMultiBodyVehicle::AddTire(const ndMultiBodyVehicleTireJointInfo& desc, const ndSharedPtr<ndBody>& tire)
{
	ndAssert(m_chassis);
	ndMatrix tireFrame(ndGetIdentityMatrix());
	tireFrame.m_front = ndVector(0.0f, 0.0f, 1.0f, 0.0f);
	tireFrame.m_up = ndVector(0.0f, 1.0f, 0.0f, 0.0f);
	tireFrame.m_right = ndVector(-1.0f, 0.0f, 0.0f, 0.0f);
	ndMatrix matrix(tireFrame * m_localFrame * m_chassis->GetMatrix());
	matrix.m_posit = tire->GetMatrix().m_posit;
	
	ndBodyDynamic* const tireBody = tire->GetAsBodyDynamic();

	// make tire inertia spherical
	ndVector inertia(tireBody->GetMassMatrix());
	ndFloat32 maxInertia(ndMax(ndMax(inertia.m_x, inertia.m_y), inertia.m_z));
	inertia.m_x = maxInertia;
	inertia.m_y = maxInertia;
	inertia.m_z = maxInertia;
	tireBody->SetMassMatrix(inertia);

	ndSharedPtr<ndJointBilateralConstraint> tireJoint (new ndMultiBodyVehicleTireJoint(matrix, tireBody, m_chassis, desc, this));
	m_tireList.Append((ndMultiBodyVehicleTireJoint*) *tireJoint);
	AddLimb(GetRoot(), tire, tireJoint);
	
	tireBody->SetDebugMaxLinearAndAngularIntegrationStep(ndFloat32(2.0f * 360.0f) * ndDegreeToRad, ndFloat32(10.0f));
	return m_tireList.GetLast()->GetInfo();
}

ndMultiBodyVehicleTireJoint* ndMultiBodyVehicle::AddAxleTire(const ndMultiBodyVehicleTireJointInfo& desc, const ndSharedPtr<ndBody>& tire, const ndSharedPtr<ndBody>& axleBody)
{
	ndAssert(m_chassis);

	ndMatrix tireFrame(ndGetIdentityMatrix());
	tireFrame.m_front = ndVector(0.0f, 0.0f, 1.0f, 0.0f);
	tireFrame.m_up = ndVector(0.0f, 1.0f, 0.0f, 0.0f);
	tireFrame.m_right = ndVector(-1.0f, 0.0f, 0.0f, 0.0f);
	ndMatrix matrix(tireFrame * m_localFrame * axleBody->GetMatrix());
	matrix.m_posit = tire->GetMatrix().m_posit;
	
	ndBodyDynamic* const tireBody = tire->GetAsBodyDynamic();
	// make tire inertia spherical
	ndVector inertia(tireBody->GetMassMatrix());
	ndFloat32 maxInertia(ndMax(ndMax(inertia.m_x, inertia.m_y), inertia.m_z));
	inertia.m_x = maxInertia;
	inertia.m_y = maxInertia;
	inertia.m_z = maxInertia;
	tireBody->SetMassMatrix(inertia);
	
	//ndSharedPtr<ndMultiBodyVehicleTireJoint> tireJoint (new ndMultiBodyVehicleTireJoint(matrix, tire, axleBody, desc, this));
	ndSharedPtr<ndJointBilateralConstraint> tireJoint(new ndMultiBodyVehicleTireJoint(matrix, tireBody, axleBody->GetAsBodyDynamic(), desc, this));
	m_tireList.Append((ndMultiBodyVehicleTireJoint*)*tireJoint);
	ndNode* const parentNode = FindByBody(*axleBody);
	ndAssert(parentNode);
	AddLimb(parentNode, tire, tireJoint);

	tireBody->SetDebugMaxLinearAndAngularIntegrationStep(ndFloat32(2.0f * 360.0f) * ndDegreeToRad, ndFloat32(10.0f));
	return m_tireList.GetLast()->GetInfo();
}


ndShapeInstance ndMultiBodyVehicle::CreateTireShape(ndFloat32 radius, ndFloat32 width) const
{
	ndShapeInstance tireCollision(m_tireShape);
	ndVector scale(ndFloat32 (2.0f) * width, radius, radius, 0.0f);
	tireCollision.SetScale(scale);
	return tireCollision;
}

ndBodyKinematic* ndMultiBodyVehicle::CreateInternalBodyPart(ndFloat32 mass, ndFloat32 radius) const
{
	ndShapeInstance diffCollision(new ndShapeSphere(radius));
	diffCollision.SetCollisionMode(false);

	ndBodyDynamic* const body = new ndBodyDynamic();
	ndAssert(m_chassis);
	body->SetMatrix(m_localFrame * m_chassis->GetMatrix());
	body->SetCollisionShape(diffCollision);
	body->SetMassMatrix(mass, diffCollision);
	//body->SetDebugMaxAngularIntegrationSteepAndLinearSpeed(ndFloat32(2.0f * 360.0f) * ndDegreeToRad, ndFloat32(100.0f));
	body->SetDebugMaxLinearAndAngularIntegrationStep(ndFloat32(2.0f * 360.0f) * ndDegreeToRad, ndFloat32(10.0f));
	return body;
}

ndMultiBodyVehicleDifferential* ndMultiBodyVehicle::AddDifferential(ndFloat32 mass, ndFloat32 radius, ndMultiBodyVehicleTireJoint* const leftTire, ndMultiBodyVehicleTireJoint* const rightTire, ndFloat32 slipOmegaLock)
{
	ndAssert(m_chassis);
	ndSharedPtr<ndBody> differentialBody (CreateInternalBodyPart(mass, radius));
	//m_internalBodies.Append(differentialBody);
	
	//ndSharedPtr<ndMultiBodyVehicleDifferential> differential(new ndMultiBodyVehicleDifferential(differentialBody->GetAsBodyKinematic(), m_chassis, slipOmegaLock));
	ndSharedPtr<ndJointBilateralConstraint> differential(new ndMultiBodyVehicleDifferential(differentialBody->GetAsBodyDynamic(), m_chassis, slipOmegaLock));
	AddLimb(GetRoot(), differentialBody, differential);
	
	ndVector pin0(differentialBody->GetMatrix().RotateVector(differential->GetLocalMatrix0().m_front));
	ndVector upPin(differentialBody->GetMatrix().RotateVector(differential->GetLocalMatrix0().m_up));
	ndVector leftPin1(leftTire->GetBody0()->GetMatrix().RotateVector(leftTire->GetLocalMatrix0().m_front));
	
	//ndSharedPtr<ndMultiBodyVehicleDifferentialAxle> leftAxle (new ndMultiBodyVehicleDifferentialAxle(pin0, upPin, differentialBody->GetAsBodyKinematic(), leftPin1, leftTire->GetBody0()));
	ndSharedPtr<ndJointBilateralConstraint> leftAxle (new ndMultiBodyVehicleDifferentialAxle(pin0, upPin, differentialBody->GetAsBodyKinematic(), leftPin1, leftTire->GetBody0()));
	//m_axleList.Append(leftAxle);
	
	//ndSharedPtr<ndMultiBodyVehicleDifferentialAxle> rightAxle (new ndMultiBodyVehicleDifferentialAxle(pin0, upPin.Scale(ndFloat32(-1.0f)), differentialBody->GetAsBodyKinematic(), leftPin1, rightTire->GetBody0()));
	ndSharedPtr<ndJointBilateralConstraint> rightAxle (new ndMultiBodyVehicleDifferentialAxle(pin0, upPin.Scale(ndFloat32(-1.0f)), differentialBody->GetAsBodyKinematic(), leftPin1, rightTire->GetBody0()));
	//m_axleList.Append(rightAxle);
	AddCloseLoop(leftAxle);
	AddCloseLoop(rightAxle);

	ndMultiBodyVehicleDifferential* const differentialJoint = (ndMultiBodyVehicleDifferential*)*differential;
	m_differentialList.Append(differentialJoint);
	return differentialJoint;
}

ndMultiBodyVehicleDifferential* ndMultiBodyVehicle::AddDifferential(ndFloat32 mass, ndFloat32 radius, ndMultiBodyVehicleDifferential* const leftDifferential, ndMultiBodyVehicleDifferential* const rightDifferential, ndFloat32 slipOmegaLock)
{
	ndAssert(m_chassis);
	//ndSharedPtr<ndBody> differentialBody(CreateInternalBodyPart(mass, radius));
	ndSharedPtr<ndBody> differentialBody(CreateInternalBodyPart(mass, radius));
	//m_internalBodies.Append(differentialBody);

	//ndSharedPtr<ndMultiBodyVehicleDifferential> differential(new ndMultiBodyVehicleDifferential(differentialBody->GetAsBodyKinematic(), m_chassis, slipOmegaLock));
	ndSharedPtr<ndJointBilateralConstraint> differential(new ndMultiBodyVehicleDifferential(differentialBody->GetAsBodyKinematic(), m_chassis, slipOmegaLock));
	//m_differentialList.Append(differential);
	AddLimb(GetRoot(), differentialBody, differential);

	ndVector pin0(differentialBody->GetMatrix().RotateVector(differential->GetLocalMatrix0().m_front));
	ndVector upPin(differentialBody->GetMatrix().RotateVector(differential->GetLocalMatrix0().m_up));
	ndVector leftPin1(leftDifferential->GetBody0()->GetMatrix().RotateVector(leftDifferential->GetLocalMatrix0().m_front));
	leftPin1 = leftPin1.Scale(ndFloat32(-1.0f));
	
	//ndSharedPtr<ndMultiBodyVehicleDifferentialAxle> leftAxle (new ndMultiBodyVehicleDifferentialAxle(pin0, upPin, differentialBody->GetAsBodyKinematic(), leftPin1, leftDifferential->GetBody0()));
	ndSharedPtr<ndJointBilateralConstraint> leftAxle (new ndMultiBodyVehicleDifferentialAxle(pin0, upPin, differentialBody->GetAsBodyKinematic(), leftPin1, leftDifferential->GetBody0()));
	//m_axleList.Append(leftAxle);
	
	//ndSharedPtr<ndMultiBodyVehicleDifferentialAxle> rightAxle (new ndMultiBodyVehicleDifferentialAxle(pin0, upPin.Scale(ndFloat32(-1.0f)), differentialBody->GetAsBodyKinematic(), leftPin1, rightDifferential->GetBody0()));
	ndSharedPtr<ndJointBilateralConstraint> rightAxle (new ndMultiBodyVehicleDifferentialAxle(pin0, upPin.Scale(ndFloat32(-1.0f)), differentialBody->GetAsBodyKinematic(), leftPin1, rightDifferential->GetBody0()));
	//m_axleList.Append(rightAxle);

	AddCloseLoop(leftAxle);
	AddCloseLoop(rightAxle);

	ndMultiBodyVehicleDifferential* const differentialJoint = (ndMultiBodyVehicleDifferential*)*differential;
	m_differentialList.Append(differentialJoint);
	return differentialJoint;
}

ndMultiBodyVehicleMotor* ndMultiBodyVehicle::AddMotor(ndFloat32 mass, ndFloat32 radius)
{
	ndAssert(m_chassis);
	ndSharedPtr<ndBody> motorBody (CreateInternalBodyPart(mass, radius));
	//m_internalBodies.Append(motorBody);
	//m_motor = ndSharedPtr<ndMultiBodyVehicleMotor>(new ndMultiBodyVehicleMotor(motorBody->GetAsBodyKinematic(), this));
	ndSharedPtr<ndJointBilateralConstraint> motorJoint(new ndMultiBodyVehicleMotor(motorBody->GetAsBodyKinematic(), this));
	AddLimb(GetRoot(), motorBody, motorJoint);
	m_motor = (ndMultiBodyVehicleMotor*)*motorJoint;
	return m_motor;
}

ndMultiBodyVehicleGearBox* ndMultiBodyVehicle::AddGearBox(ndMultiBodyVehicleDifferential* const differential)
{
	ndAssert(m_motor);
	//m_gearBox = ndSharedPtr<ndMultiBodyVehicleGearBox>(new ndMultiBodyVehicleGearBox(m_motor->GetBody0(), differential->GetBody0(), this));
	ndSharedPtr<ndJointBilateralConstraint> gearBox(new ndMultiBodyVehicleGearBox(m_motor->GetBody0(), differential->GetBody0(), this));
	AddCloseLoop(gearBox);
	m_gearBox = (ndMultiBodyVehicleGearBox*)*gearBox;
	return m_gearBox;
}

bool ndMultiBodyVehicle::IsSleeping() const
{
	bool sleeping = true;
	for (ndNode* node = GetRoot()->GetFirstIterator(); sleeping && node; node = node->GetNextIterator())
	{
		ndBodyDynamic* const body = node->m_body->GetAsBodyDynamic();
		//active = active || (body->GetScene() ? true : false);
		sleeping = sleeping && body->GetSleepState();
	}
	return sleeping;
}

void ndMultiBodyVehicle::ApplyAerodynamics(ndWorld* const, ndFloat32)
{
	m_downForce.m_suspensionStiffnessModifier = ndFloat32(1.0f);
	ndFloat32 gravity = m_downForce.GetDownforceFactor(GetSpeed());
	if (ndAbs (gravity) > ndFloat32(1.0e-2f))
	{
		const ndVector up(m_chassis->GetMatrix().RotateVector(m_localFrame.m_up));
		const ndVector weight(m_chassis->GetForce());
		const ndVector downForce(up.Scale(gravity * m_chassis->GetMassMatrix().m_w));
		m_chassis->SetForce(weight + downForce);
		m_downForce.m_suspensionStiffnessModifier = up.DotProduct(weight).GetScalar() / up.DotProduct(weight + downForce.Scale (0.5f)).GetScalar();
		//dTrace(("%f\n", m_suspensionStiffnessModifier));
		
		for (ndList<ndMultiBodyVehicleTireJoint*>::ndNode* node = m_tireList.GetFirst(); node; node = node->GetNext())
		{
			ndMultiBodyVehicleTireJoint* const tire = node->GetInfo();
			ndBodyKinematic* const tireBody = tire->GetBody0();
			const ndVector tireWeight(tireBody->GetForce());
			const ndVector tireDownForce(up.Scale(gravity * tireBody->GetMassMatrix().m_w));
			tireBody->SetForce(tireWeight + tireDownForce);
		}
	}
}

//void ndMultiBodyVehicle::CoulombTireModel(ndMultiBodyVehicleTireJoint* const tire, ndContactMaterial& contactPoint) const
void ndMultiBodyVehicle::CoulombTireModel(ndMultiBodyVehicleTireJoint* const, ndContactMaterial& contactPoint, ndFloat32) const
{
	const ndFloat32 frictionCoefficient = contactPoint.m_material.m_staticFriction0;
	const ndFloat32 normalForce = contactPoint.m_normal_Force.GetInitialGuess() + ndFloat32(1.0f);
	const ndFloat32 maxForceForce = frictionCoefficient * normalForce;

	contactPoint.m_material.m_staticFriction0 = maxForceForce;
	contactPoint.m_material.m_dynamicFriction0 = maxForceForce;
	contactPoint.m_material.m_staticFriction1 = maxForceForce;
	contactPoint.m_material.m_dynamicFriction1 = maxForceForce;
	contactPoint.m_material.m_flags = contactPoint.m_material.m_flags | m_override0Friction | m_override1Friction;
}

//void ndMultiBodyVehicle::CalculateNormalizedAlgniningTorque(ndMultiBodyVehicleTireJoint* const tire, ndFloat32 sideSlipTangent) const
void ndMultiBodyVehicle::CalculateNormalizedAlgniningTorque(ndMultiBodyVehicleTireJoint* const, ndFloat32 sideSlipTangent) const
{
	//I need to calculate the integration of the align torque 
	//using the calculate contact patch, form the standard brush model.
	//for now just set the torque to zero.
	ndFloat32 angle = ndAtan(sideSlipTangent);
	ndFloat32 a = ndFloat32(0.1f);

	ndFloat32 slipCos(ndCos(angle));
	ndFloat32 slipSin(ndSin(angle));
	ndFloat32 y1 = ndFloat32(2.0f) * slipSin * slipCos;
	ndFloat32 x1 = -a + ndFloat32(2.0f) * slipCos * slipCos;

	ndVector p1(x1, y1, ndFloat32(0.0f), ndFloat32(0.0f));
	ndVector p0(-a, ndFloat32(0.0f), ndFloat32(0.0f), ndFloat32(0.0f));

	static int xxxx;
	xxxx++;
	if (xxxx % 1000 == 0)
	{
		ndTrace(("aligning torque\n"));
	}
	//ndFloat32 alignTorque = ndFloat32(0.0f);
	//ndFloat32 sign = ndSign(alignTorque);
	//tire->m_normalizedAligningTorque = sign * ndMax(ndAbs(alignTorque), ndAbs(tire->m_normalizedAligningTorque));
}

void ndMultiBodyVehicle::BrushTireModel(ndMultiBodyVehicleTireJoint* const tire, ndContactMaterial& contactPoint, ndFloat32 timestep) const
{
	// calculate longitudinal slip ratio
	const ndBodyKinematic* const chassis = m_chassis;
	ndAssert(chassis);
	const ndBodyKinematic* const tireBody = tire->GetBody0()->GetAsBodyDynamic();
	const ndBodyKinematic* const otherBody = (contactPoint.m_body0 == tireBody) ? ((ndBodyKinematic*)contactPoint.m_body1)->GetAsBodyDynamic() : ((ndBodyKinematic*)contactPoint.m_body0)->GetAsBodyDynamic();
	ndAssert(tireBody != otherBody);
	ndAssert((tireBody == contactPoint.m_body0) || (tireBody == contactPoint.m_body1));

	//tire non linear brush model is only considered 
	//when is moving faster than 0.5 m/s (approximately 1.0 miles / hours) 
	//this is just an arbitrary limit, based of the model 
	//not been defined for stationary tires.
	const ndVector contactVeloc0(tireBody->GetVelocity());
	const ndVector contactVeloc1(otherBody->GetVelocityAtPoint(contactPoint.m_point));
	const ndVector relVeloc(contactVeloc0 - contactVeloc1);
	const ndVector lateralDir = contactPoint.m_dir1;
	const ndVector longitudDir = contactPoint.m_dir0;
	const ndFloat32 relSpeed = ndAbs(relVeloc.DotProduct(longitudDir).GetScalar());
	if (relSpeed > D_MAX_CONTACT_SPEED_TRESHOLD)
	{
		// tire is in breaking and traction mode.
		const ndVector contactVeloc(tireBody->GetVelocityAtPoint(contactPoint.m_point) - contactVeloc1);

		//const ndVector tireVeloc(tireBody->GetVelocity());
		const ndFloat32 vr = contactVeloc.DotProduct(longitudDir).GetScalar();
		const ndFloat32 longitudialSlip = ndAbs(vr) / relSpeed;

		//const ndFloat32 sideSpeed = ndAbs(relVeloc.DotProduct(lateralDir).GetScalar());
		const ndFloat32 sideSpeed = relVeloc.DotProduct(lateralDir).GetScalar();
		const ndFloat32 signedLateralSlip = sideSpeed / (relSpeed + ndFloat32(1.0f));
		CalculateNormalizedAlgniningTorque(tire, signedLateralSlip);

		const ndFloat32 lateralSlip = ndAbs(signedLateralSlip);

		ndAssert(longitudialSlip >= ndFloat32(0.0f));

		CalculateNormalizedAlgniningTorque(tire, lateralSlip);

		tire->m_lateralSlip = ndMax(tire->m_lateralSlip, lateralSlip);
		tire->m_longitudinalSlip = ndMax(tire->m_longitudinalSlip, longitudialSlip);

		const ndFloat32 den = ndFloat32(1.0f) / (longitudialSlip + ndFloat32(1.0f));
		const ndFloat32 v = lateralSlip * den;
		const ndFloat32 u = longitudialSlip * den;

		const ndTireFrictionModel& info = tire->m_frictionModel;
		const ndFloat32 vehicleMass = chassis->GetMassMatrix().m_w;
		const ndFloat32 cz = vehicleMass * info.m_laterialStiffness * v;
		const ndFloat32 cx = vehicleMass * info.m_longitudinalStiffness * u;

		const ndFloat32 gamma = ndMax(ndSqrt(cx * cx + cz * cz), ndFloat32(1.0e-8f));
		const ndFloat32 frictionCoefficient = contactPoint.m_material.m_staticFriction0;
		const ndFloat32 normalForce = contactPoint.m_normal_Force.GetInitialGuess() + ndFloat32(1.0f);

		const ndFloat32 maxForceForce = frictionCoefficient * normalForce;
		ndFloat32 f = maxForceForce;
		if (gamma < (ndFloat32(3.0f) * maxForceForce))
		{
			const ndFloat32 b = ndFloat32(1.0f) / (ndFloat32(3.0f) * maxForceForce);
			const ndFloat32 c = ndFloat32(1.0f) / (ndFloat32(27.0f) * maxForceForce * maxForceForce);
			f = gamma * (ndFloat32(1.0f) - b * gamma + c * gamma * gamma);
		}

		const ndFloat32 lateralForce = f * cz / gamma;
		const ndFloat32 longitudinalForce = f * cx / gamma;
		//ndTrace(("(%d: %f %f)  ", tireBody->GetId(), longitudinalForce, lateralForce));

		contactPoint.OverrideFriction0Accel(-vr / timestep);
		contactPoint.m_material.m_staticFriction0 = longitudinalForce;
		contactPoint.m_material.m_dynamicFriction0 = longitudinalForce;
		contactPoint.m_material.m_staticFriction1 = lateralForce;
		contactPoint.m_material.m_dynamicFriction1 = lateralForce;
		contactPoint.m_material.m_flags = contactPoint.m_material.m_flags | m_override0Friction | m_override1Friction;
	}
	else
	{
		CoulombTireModel(tire, contactPoint, timestep);
	}
}

void ndMultiBodyVehicle::PacejkaTireModel(ndMultiBodyVehicleTireJoint* const tire, ndContactMaterial& contactPoint, ndFloat32 timestep) const
{
	BrushTireModel(tire, contactPoint, timestep);
}

void ndMultiBodyVehicle::CoulombFrictionCircleTireModel(ndMultiBodyVehicleTireJoint* const tire, ndContactMaterial& contactPoint, ndFloat32 timestep) const
{
	BrushTireModel(tire, contactPoint, timestep);
}

#if 0
void ndMultiBodyVehicle::ApplyTireModel(ndFloat32 timestep, ndTireContactPair* const tireContacts, ndInt32 tireCount)
{
	for (ndInt32 i = tireCount - 1; i >= 0; --i)
	{
		ndContact* const contact = tireContacts[i].m_contact;
		ndMultiBodyVehicleTireJoint* const tire = tireContacts[i].m_tireJoint;
		ndContactPointList& contactPoints = contact->GetContactPoints();
		ndMatrix tireBasisMatrix(tire->GetLocalMatrix1() * tire->GetBody1()->GetMatrix());
		tireBasisMatrix.m_posit = tire->GetBody0()->GetMatrix().m_posit;
		for (ndContactPointList::ndNode* contactNode = contactPoints.GetFirst(); contactNode; contactNode = contactNode->GetNext())
		{
			ndContactMaterial& contactPoint = contactNode->GetInfo();
			ndFloat32 contactPathLocation = ndAbs(contactPoint.m_normal.DotProduct(tireBasisMatrix.m_front).GetScalar());
			// contact are consider on the contact patch strip only if the are less than 
			// 45 degree angle from the tire axle
			if (contactPathLocation < ndFloat32(0.71f))
			{
				// align tire friction direction
				const ndVector longitudinalDir(contactPoint.m_normal.CrossProduct(tireBasisMatrix.m_front).Normalize());
				const ndVector lateralDir(longitudinalDir.CrossProduct(contactPoint.m_normal));

				contactPoint.m_dir1 = lateralDir;
				contactPoint.m_dir0 = longitudinalDir;

				// check if the contact is in the contact patch,
				// the is the 45 degree point around the tire vehicle axis. 
				ndVector dir(contactPoint.m_point - tireBasisMatrix.m_posit);
				ndAssert(dir.DotProduct(dir).GetScalar() > ndFloat32(0.0f));
				ndFloat32 contactPatch = tireBasisMatrix.m_up.DotProduct(dir.Normalize()).GetScalar();
				if (contactPatch < ndFloat32(-0.71f))
				{
					switch (tire->m_frictionModel.m_frictionModel)
					{
					case ndTireFrictionModel::m_brushModel:
					{
						BrushTireModel(tire, contactPoint, timestep);
						break;
					}

					case ndTireFrictionModel::m_pacejka:
					{
						PacejkaTireModel(tire, contactPoint, timestep);
						break;
					}

					case ndTireFrictionModel::m_coulombCicleOfFriction:
					{
						CoulombFrictionCircleTireModel(tire, contactPoint, timestep);
						break;
					}

					case ndTireFrictionModel::m_coulomb:
					default:
					{
						CoulombTireModel(tire, contactPoint, timestep);
						break;
					}
					}
				}
			}
		}
	}
}

#else
void ndMultiBodyVehicle::ApplyTireModel(ndFloat32 timestep, ndTireContactPair* const tireContacts, ndInt32 contactCount)
{
	ndInt32 oldCount = contactCount;
	for (ndInt32 i = contactCount - 1; i >= 0; --i)
	{
		ndContact* const contact = tireContacts[i].m_contact;
		ndMultiBodyVehicleTireJoint* const tire = tireContacts[i].m_tireJoint;
		ndContactPointList& contactPoints = contact->GetContactPoints();
		ndMatrix tireBasisMatrix(tire->GetLocalMatrix1() * tire->GetBody1()->GetMatrix());
		tireBasisMatrix.m_posit = tire->GetBody0()->GetMatrix().m_posit;
		const ndMaterial* const material = contact->GetMaterial();
		bool useCoulombModel = (material->m_flags & m_useBrushTireModel) ? false : true;
		for (ndContactPointList::ndNode* contactNode = contactPoints.GetFirst(); contactNode; contactNode = contactNode->GetNext())
		{
			ndContactMaterial& contactPoint = contactNode->GetInfo();
			ndFloat32 contactPathLocation = ndAbs(contactPoint.m_normal.DotProduct(tireBasisMatrix.m_front).GetScalar());
			// contact are consider on the contact patch strip only if the are less than 
			// 45 degree angle from the tire axle
			if (contactPathLocation < ndFloat32(0.71f))
			{
				// align tire friction direction
				const ndVector longitudinalDir(contactPoint.m_normal.CrossProduct(tireBasisMatrix.m_front).Normalize());
				const ndVector lateralDir(longitudinalDir.CrossProduct(contactPoint.m_normal));

				contactPoint.m_dir1 = lateralDir;
				contactPoint.m_dir0 = longitudinalDir;

				// check if the contact is in the contact patch,
				// the is the 45 degree point around the tire vehicle axis. 
				ndVector dir(contactPoint.m_point - tireBasisMatrix.m_posit);
				ndAssert(dir.DotProduct(dir).GetScalar() > ndFloat32(0.0f));
				ndFloat32 contactPatch = tireBasisMatrix.m_up.DotProduct(dir.Normalize()).GetScalar();
				if (useCoulombModel || (contactPatch > ndFloat32(-0.71f)))
				{
					contactCount--;
					tireContacts[i] = tireContacts[contactCount];
				}
			}
		}
	}

	if (contactCount == oldCount)
	{
		for (ndInt32 i = contactCount - 1; i >= 0; --i)
		{
			ndContact* const contact = tireContacts[i].m_contact;
			ndMultiBodyVehicleTireJoint* const tire = tireContacts[i].m_tireJoint;
			ndContactPointList& contactPoints = contact->GetContactPoints();
			for (ndContactPointList::ndNode* contactNode = contactPoints.GetFirst(); contactNode; contactNode = contactNode->GetNext())
			{
				ndContactMaterial& contactPoint = contactNode->GetInfo();
				switch (tire->m_frictionModel.m_frictionModel)
				{
				case ndTireFrictionModel::m_brushModel:
				{
					BrushTireModel(tire, contactPoint, timestep);
					break;
				}

				case ndTireFrictionModel::m_pacejka:
				{
					PacejkaTireModel(tire, contactPoint, timestep);
					break;
				}

				case ndTireFrictionModel::m_coulombCicleOfFriction:
				{
					CoulombFrictionCircleTireModel(tire, contactPoint, timestep);
					break;
				}

				case ndTireFrictionModel::m_coulomb:
				default:
				{
					CoulombTireModel(tire, contactPoint, timestep);
					break;
				}
				}
			}
		}
	}
}
#endif

void ndMultiBodyVehicle::ApplyTireModel(ndWorld* const, ndFloat32 timestep)
{
	ndFixSizeArray<ndTireContactPair, 128> tireContacts;
	for (ndList<ndMultiBodyVehicleTireJoint*>::ndNode* node = m_tireList.GetFirst(); node; node = node->GetNext())
	{
		ndMultiBodyVehicleTireJoint* const tire = node->GetInfo();
		ndAssert(((ndShape*)tire->GetBody0()->GetCollisionShape().GetShape())->GetAsShapeChamferCylinder());
	
		tire->m_lateralSlip = ndFloat32(0.0f);
		tire->m_longitudinalSlip = ndFloat32(0.0f);
		tire->m_normalizedAligningTorque = ndFloat32(0.0f);
	
		const ndBodyKinematic::ndContactMap& contactMap = tire->GetBody0()->GetContactMap();
		ndBodyKinematic::ndContactMap::Iterator it(contactMap);
		for (it.Begin(); it; it++)
		{
			ndContact* const contact = *it;
			if (contact->IsActive())
			{
				ndContactPointList& contactPoints = contact->GetContactPoints();
				// for mesh collision we need to remove contact duplicates, 
				// these are contact produced by two or more polygons, 
				// that can produce two contact so are close that they can generate 
				// ill formed rows in the solver mass matrix
				for (ndContactPointList::ndNode* contactNode0 = contactPoints.GetFirst(); contactNode0; contactNode0 = contactNode0->GetNext())
				{
					const ndContactPoint& contactPoint0 = contactNode0->GetInfo();
					for (ndContactPointList::ndNode* contactNode1 = contactNode0->GetNext(); contactNode1; contactNode1 = contactNode1->GetNext())
					{
						const ndContactPoint& contactPoint1 = contactNode1->GetInfo();
						const ndVector error(contactPoint1.m_point - contactPoint0.m_point);
						ndFloat32 err2 = error.DotProduct(error).GetScalar();
						if (err2 < D_MIN_CONTACT_CLOSE_DISTANCE2)
						{
							contactPoints.Remove(contactNode1);
							break;
						}
					}
				}
				ndTireContactPair pair;
				pair.m_contact = contact;
				pair.m_tireJoint = tire;
				tireContacts.PushBack(pair);
			}
		}
	}
	
	//ApplyVehicleDynamicControl(timestep, &tireContacts[0], tireContacts.GetCount());
	ApplyTireModel(timestep, &tireContacts[0], tireContacts.GetCount());
}

void ndMultiBodyVehicle::Update(ndWorld* const world, ndFloat32 timestep)
{
	ndAssert(!IsSleeping());
	// apply down force
	ApplyAerodynamics(world, timestep);
	// apply tire model
	ApplyTireModel(world, timestep);
}

void ndMultiBodyVehicle::ApplyAlignmentAndBalancing()
{
	//for (ndReferencedObjects<ndMultiBodyVehicleTireJoint>::ndNode* node = m_tireList.GetFirst(); node; node = node->GetNext())
	for (ndList<ndMultiBodyVehicleTireJoint*>::ndNode* node = m_tireList.GetFirst(); node; node = node->GetNext())
	{
		ndMultiBodyVehicleTireJoint* const tire = node->GetInfo();
		ndBodyKinematic* const tireBody = tire->GetBody0()->GetAsBodyDynamic();
		ndBodyKinematic* const chassisBody = tire->GetBody1()->GetAsBodyDynamic();
	
		bool savedSleepState = tireBody->GetSleepState();
		tire->UpdateTireSteeringAngleMatrix();
		
		ndMatrix tireMatrix;
		ndMatrix chassisMatrix;
		tire->CalculateGlobalMatrix(tireMatrix, chassisMatrix);
		
		// align tire velocity
		const ndVector chassisVelocity(chassisBody->GetVelocityAtPoint(tireMatrix.m_posit));
		const ndVector relVeloc(tireBody->GetVelocity() - chassisVelocity);
		ndVector localVeloc(chassisMatrix.UnrotateVector(relVeloc));
		bool applyProjection = (localVeloc.m_x * localVeloc.m_x + localVeloc.m_z * localVeloc.m_z) > (ndFloat32(0.05f) * ndFloat32(0.05f));
		localVeloc.m_x *= ndFloat32(0.3f);
		localVeloc.m_z *= ndFloat32(0.3f);
		const ndVector tireVelocity(chassisVelocity + chassisMatrix.RotateVector(localVeloc));
		
		// align tire angular velocity
		const ndVector chassisOmega(chassisBody->GetOmega());
		const ndVector relOmega(tireBody->GetOmega() - chassisOmega);
		ndVector localOmega(chassisMatrix.UnrotateVector(relOmega));
		applyProjection = applyProjection || (localOmega.m_y * localOmega.m_y + localOmega.m_z * localOmega.m_z) > (ndFloat32(0.05f) * ndFloat32(0.05f));
		localOmega.m_y *= ndFloat32(0.3f);
		localOmega.m_z *= ndFloat32(0.3f);
		const ndVector tireOmega(chassisOmega + chassisMatrix.RotateVector(localOmega));
		
		if (applyProjection)
		{
			tireBody->SetOmega(tireOmega);
			tireBody->SetVelocity(tireVelocity);
		}
		tireBody->RestoreSleepState(savedSleepState);
	}
	
	for (ndList<ndMultiBodyVehicleDifferential*>::ndNode* node = m_differentialList.GetFirst(); node; node = node->GetNext())
	{
		ndMultiBodyVehicleDifferential* const diff = node->GetInfo();
		diff->AlignMatrix();
	}
	
	if (m_motor)
	{
		m_motor->AlignMatrix();
	}
}

void ndMultiBodyVehicle::PostUpdate(ndWorld* const, ndFloat32)
{
	ApplyAlignmentAndBalancing();
}


void ndMultiBodyVehicle::Debug(ndConstraintDebugCallback& context) const
{
	// draw vehicle cordinade system;
	const ndBodyKinematic* const chassis = m_chassis;
	ndAssert(chassis);
	ndMatrix chassisMatrix(chassis->GetMatrix());
	chassisMatrix.m_posit = chassisMatrix.TransformVector(chassis->GetCentreOfMass());
	context.DrawFrame(chassisMatrix);
	
	ndFloat32 totalMass = chassis->GetMassMatrix().m_w;
	ndVector effectiveCom(chassisMatrix.m_posit.Scale(totalMass));
	
	// draw front direction for side slip angle reference
	
	// draw velocity vector
	ndVector veloc(chassis->GetVelocity());
	ndVector p0(chassisMatrix.m_posit + m_localFrame.m_up.Scale(1.0f));
	ndVector p1(p0 + chassisMatrix.RotateVector(m_localFrame.m_front).Scale(2.0f));
	ndVector p2(p0 + veloc.Scale (0.25f));
	
	context.DrawLine(p0, p2, ndVector(1.0f, 1.0f, 0.0f, 0.0f));
	context.DrawLine(p0, p1, ndVector(1.0f, 0.0f, 0.0f, 0.0f));
	
	//// draw body acceleration
	////ndVector accel(m_chassis->GetAccel());
	////ndVector p3(p0 + accel.Scale(0.5f));
	////context.DrawLine(p0, p3, ndVector(0.0f, 1.0f, 1.0f, 0.0f));
	
	ndFloat32 scale = ndFloat32 (3.0f);
	ndVector weight(chassis->GetForce().Scale (scale * chassis->GetInvMass() / m_downForce.m_gravity));
	
	// draw vehicle weight;
	ndVector forceColor(ndFloat32 (0.8f), ndFloat32(0.8f), ndFloat32(0.8f), ndFloat32(0.0f));
	ndVector lateralColor(ndFloat32(0.3f), ndFloat32(0.7f), ndFloat32(0.0f), ndFloat32(0.0f));
	ndVector longitudinalColor(ndFloat32(0.7f), ndFloat32(0.3f), ndFloat32(0.0f), ndFloat32(0.0f));
	context.DrawLine(chassisMatrix.m_posit, chassisMatrix.m_posit + weight, forceColor);
	
	for (ndList<ndMultiBodyVehicleTireJoint*>::ndNode* node = m_tireList.GetFirst(); node; node = node->GetNext())
	{
		ndMultiBodyVehicleTireJoint* const tireJoint = node->GetInfo();
		ndBodyKinematic* const tireBody = tireJoint->GetBody0()->GetAsBodyDynamic();
		ndMatrix tireFrame(tireBody->GetMatrix());
		totalMass += tireBody->GetMassMatrix().m_w;
		effectiveCom += tireFrame.m_posit.Scale(tireBody->GetMassMatrix().m_w);
	}
	
	for (ndList<ndMultiBodyVehicleTireJoint*>::ndNode* node = m_tireList.GetFirst(); node; node = node->GetNext())
	{
		ndMultiBodyVehicleTireJoint* const tireJoint = node->GetInfo();
		ndBodyKinematic* const tireBody = tireJoint->GetBody0()->GetAsBodyDynamic();
	
		// draw upper bumper
		ndMatrix upperBumberMatrix(tireJoint->CalculateUpperBumperMatrix());
		//context.DrawFrame(tireJoint->CalculateUpperBumperMatrix());
	
		ndMatrix tireBaseFrame(tireJoint->CalculateBaseFrame());
		//context.DrawFrame(tireBaseFrame);
	
		// show tire center of mass;
		ndMatrix tireFrame(tireBody->GetMatrix());
		//context.DrawFrame(tireFrame);
		upperBumberMatrix.m_posit = tireFrame.m_posit;
		//context.DrawFrame(upperBumberMatrix);
	
		// draw tire forces
		const ndBodyKinematic::ndContactMap& contactMap = tireBody->GetContactMap();
		ndFloat32 tireGravities = scale /(totalMass * m_downForce.m_gravity);
		ndBodyKinematic::ndContactMap::Iterator it(contactMap);
		for (it.Begin(); it; it++)
		{
			ndContact* const contact = *it;
			if (contact->IsActive())
			{
				const ndContactPointList& contactPoints = contact->GetContactPoints();
				for (ndContactPointList::ndNode* contactNode = contactPoints.GetFirst(); contactNode; contactNode = contactNode->GetNext())
				{
					const ndContactMaterial& contactPoint = contactNode->GetInfo();
					ndMatrix frame(contactPoint.m_normal, contactPoint.m_dir0, contactPoint.m_dir1, contactPoint.m_point);
	
					ndVector localPosit(m_localFrame.UntransformVector(chassisMatrix.UntransformVector(contactPoint.m_point)));
					ndFloat32 offset = (localPosit.m_z > ndFloat32(0.0f)) ? ndFloat32(0.2f) : ndFloat32(-0.2f);
					frame.m_posit += contactPoint.m_dir0.Scale(offset);
					frame.m_posit += contactPoint.m_normal.Scale(0.1f);
	
					// normal force
					ndFloat32 normalForce = -tireGravities * contactPoint.m_normal_Force.m_force;
					context.DrawLine(frame.m_posit, frame.m_posit + contactPoint.m_normal.Scale (normalForce), forceColor);
	
					// lateral force
					ndFloat32 lateralForce = -tireGravities * contactPoint.m_dir0_Force.m_force;
					context.DrawLine(frame.m_posit, frame.m_posit + contactPoint.m_dir0.Scale(lateralForce), lateralColor);
	
					// longitudinal force
					ndFloat32 longitudinalForce = tireGravities * contactPoint.m_dir1_Force.m_force;
					context.DrawLine(frame.m_posit, frame.m_posit + contactPoint.m_dir1.Scale(longitudinalForce), longitudinalColor);
				}
			}
		}
	}
	
	effectiveCom = effectiveCom.Scale(ndFloat32(1.0f) / totalMass);
	chassisMatrix.m_posit = effectiveCom;
	chassisMatrix.m_posit.m_w = ndFloat32(1.0f);
	context.DrawFrame(chassisMatrix);
}
