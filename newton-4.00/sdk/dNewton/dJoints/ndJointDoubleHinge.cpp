/* Copyright (c) <2003-2021> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#include "dCoreStdafx.h"
#include "ndNewtonStdafx.h"
#include "ndJointDoubleHinge.h"

ndJointDoubleHinge::ndJointDoubleHinge(const dMatrix& pinAndPivotFrame, ndBodyKinematic* const child, ndBodyKinematic* const parent)
	:ndJointBilateralConstraint(6, child, parent, pinAndPivotFrame)
	,m_jointAngle0(dFloat32(0.0f))
	,m_jointSpeed0(dFloat32(0.0f))
	,m_jointAngle1(dFloat32(0.0f))
	,m_jointSpeed1(dFloat32(0.0f))
{
}

ndJointDoubleHinge::~ndJointDoubleHinge()
{
}

void ndJointDoubleHinge::JacobianDerivative(ndConstraintDescritor& desc)
{
	dMatrix matrix0;
	dMatrix matrix1;
	CalculateGlobalMatrix(matrix0, matrix1);

	AddLinearRowJacobian(desc, matrix0.m_posit, matrix1.m_posit, matrix1[0]);
	AddLinearRowJacobian(desc, matrix0.m_posit, matrix1.m_posit, matrix1[1]);
	AddLinearRowJacobian(desc, matrix0.m_posit, matrix1.m_posit, matrix1[2]);

	const dVector frontDir((matrix0.m_front - matrix1.m_up.Scale(matrix0.m_front.DotProduct(matrix1.m_up).GetScalar())).Normalize());
	const dVector sideDir(frontDir.CrossProduct(matrix1.m_up));
	const dFloat32 angle = CalculateAngle(matrix0.m_front, frontDir, sideDir);
	AddAngularRowJacobian(desc, sideDir, angle);

	// not happy with this method because it is a penalty system, 
	// but is hard for an first order integrator to prevent the side angle from drifting, 
	// even an implicit one with expanding the Jacobian partial derivatives still has a hard time
	// nullifying the gyro torque generate by the two angular velocities.
	const dFloat32 alphaRollError = GetMotorZeroAcceleration(desc) + 0.5f * angle * desc.m_invTimestep * desc.m_invTimestep;
	SetMotorAcceleration(desc, alphaRollError);

	//// save the current joint Omega
	dVector omega0(m_body0->GetOmega());
	dVector omega1(m_body1->GetOmega());
	
	// calculale joint parameters, angles and omega
	const dFloat32 deltaAngle0 = AnglesAdd(-CalculateAngle(matrix0.m_up, matrix1.m_up, frontDir), -m_jointAngle0);
	m_jointAngle0 += deltaAngle0;
	m_jointSpeed0 = frontDir.DotProduct(omega0 - omega1).GetScalar();

	const dFloat32 deltaAngle1 = AnglesAdd(-CalculateAngle(frontDir, matrix1.m_front, matrix1.m_up), -m_jointAngle1);
	m_jointAngle1 += deltaAngle1;
	m_jointSpeed1 = matrix1.m_up.DotProduct(omega0 - omega1).GetScalar();

	//dTrace(("%f %f\n", m_jointAngle1 * dRadToDegree, m_jointSpeed1));
	
	//// two rows to restrict rotation around around the parent coordinate system
	//const dFloat32 angle0 = CalculateAngle(matrix0.m_front, matrix1.m_front, matrix1.m_up);
	//AddAngularRowJacobian(desc, matrix1.m_up, angle0);
	//
	//const dFloat32 angle1 = CalculateAngle(matrix0.m_front, matrix1.m_front, matrix1.m_right);
	//AddAngularRowJacobian(desc, matrix1.m_right, angle1);
}


