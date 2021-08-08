/* Copyright (c) <2003-2021> <Julio Jerez, Newton Game Dynamics>
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

#include "dCoreStdafx.h"
#include "ndNewtonStdafx.h"
#include "ndWorld.h"
#include "ndCharacter.h"
#include "ndCharacterEffectorNode.h"
#include "ndCharacterBipedPoseController.h"

ndCharacterBipedPoseController::ndCharacterBipedPoseController(ndCharacter* const owner)
	:ndCharacterPoseController(owner)
	,m_leftFootEffector(nullptr)
	,m_rightFootEffector(nullptr)
{
	m_walkCycle.m_angle = dFloat32(0.0f);
	m_walkCycle.m_stride = dFloat32(0.25f);
	m_walkCycle.m_high = dFloat32(1.0f);
}

ndCharacterBipedPoseController::~ndCharacterBipedPoseController()
{
}

void ndCharacterBipedPoseController::SetLeftFootEffector(ndCharacterEffectorNode* const node)
{
	m_leftFootEffector = node;
}

void ndCharacterBipedPoseController::SetRightFootEffector(ndCharacterEffectorNode* const node)
{
	m_rightFootEffector = node;
}

void ndCharacterBipedPoseController::ndProceduralWalk::Update(ndCharacterEffectorNode* const leftFootEffector, ndCharacterEffectorNode* const rightFootEffector, dFloat32 timestep)
{
	dFloat32 leftX = m_stride * dSin(m_angle + dFloat32 (0.0f));
	dFloat32 rightX = m_stride * dSin(m_angle + dPi);

	dFloat32 high = dFloat32(0.5f * 0.125f);
	leftFootEffector->SetTargetMatrix(dVector (leftX, high, dFloat32(0.0f), dFloat32(1.0f)));
	rightFootEffector->SetTargetMatrix(dVector(rightX, high, dFloat32(0.0f), dFloat32(1.0f)));

	m_angle += dMod (timestep * 2.0f, dFloat32(2.0f) * dPi);
}

//bool ndCharacterBipedPoseController::Evaluate(ndWorld* const world, dFloat32 timestep)
bool ndCharacterBipedPoseController::Evaluate(ndWorld* const , dFloat32 timestep)
{
	//ndCharacter::ndCentreOfMassState comState(m_owner->CalculateCentreOfMassState());
	//m_owner->UpdateGlobalPose(world, timestep);
	//m_owner->CalculateLocalPose(world, timestep);
	m_walkCycle.Update(m_leftFootEffector, m_rightFootEffector, timestep);
	return true;
}


