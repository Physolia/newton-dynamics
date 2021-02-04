/* Copyright (c) <2003-2019> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#include "ndSandboxStdafx.h"
#include "ndSkyBox.h"
#include "ndTargaToOpenGl.h"
#include "ndDemoMesh.h"
#include "ndDemoCamera.h"
#include "ndPhysicsUtils.h"
#include "ndPhysicsWorld.h"
#include "ndMakeStaticMap.h"
#include "ndDemoEntityManager.h"
#include "ndDemoInstanceEntity.h"

//static void AddShape(ndDemoEntityManager* const scene,
//	ndDemoInstanceEntity* const rootEntity, const ndShapeInstance& sphereShape,
//	dFloat32 mass, const dVector& origin, const dFloat32 diameter, dInt32 count)
//{
//	dMatrix matrix(dRollMatrix(90.0f * dDegreeToRad));
//	matrix.m_posit = origin;
//	matrix.m_posit.m_w = 1.0f;
//
//	ndPhysicsWorld* const world = scene->GetWorld();
//
//	dVector floor(FindFloor(*world, matrix.m_posit + dVector(0.0f, 100.0f, 0.0f, 0.0f), 200.0f));
//	matrix.m_posit.m_y = floor.m_y + diameter * 0.5f + 7.0f;
//
//	for (dInt32 i = 0; i < count; i++)
//	{
//		ndBodyDynamic* const body = new ndBodyDynamic();
//		ndDemoEntity* const entity = new ndDemoEntity(matrix, rootEntity);
//
//		body->SetNotifyCallback(new ndDemoEntityNotify(scene, entity));
//		body->SetMatrix(matrix);
//		body->SetCollisionShape(sphereShape);
//		body->SetMassMatrix(mass, sphereShape);
//		body->SetAngularDamping(dVector(dFloat32(0.5f)));
//
//		world->AddBody(body);
//		matrix.m_posit.m_y += diameter * 2.5f;
//	}
//}
//
//static void AddCapsulesStacks(ndDemoEntityManager* const scene, const dVector& origin)
//{
//	dFloat32 diameter = 1.0f;
//	ndShapeInstance shape(new ndShapeCapsule(diameter * 0.5f, diameter * 0.5f, diameter * 1.0f));
//	ndDemoMeshIntance* const instanceMesh = new ndDemoMeshIntance("shape", scene->GetShaderCache(), &shape, "marble.tga", "marble.tga", "marble.tga");
//
//	ndDemoInstanceEntity* const rootEntity = new ndDemoInstanceEntity(instanceMesh);
//	scene->AddEntity(rootEntity);
//
//	//const dInt32 n = 1;
//	//const dInt32 stackHigh = 1;
//	const dInt32 n = 10;
//	const dInt32 stackHigh = 7;
//	for (dInt32 i = 0; i < n; i++)
//	{
//		for (dInt32 j = 0; j < n; j++)
//		{
//			dVector location((j - n / 2) * 4.0f, 0.0f, (i - n / 2) * 4.0f, 0.0f);
//			AddShape(scene, rootEntity, shape, 10.0f, location + origin, diameter, stackHigh);
//		}
//	}
//
//	instanceMesh->Release();
//}

ndBodyKinematic* BuildFrictionRamp(ndDemoEntityManager* const scene)
{
	ndPhysicsWorld* const world = scene->GetWorld();

	ndShapeInstance box(new ndShapeBox(20.0f, 0.25f, 30.f));
	dMatrix uvMatrix(dGetIdentityMatrix());
	uvMatrix[0][0] *= 0.25f;
	uvMatrix[1][1] *= 0.25f;
	uvMatrix[2][2] *= 0.25f;
	uvMatrix.m_posit = dVector(-0.5f, -0.5f, 0.0f, 1.0f);
	const char* const textureName = "wood_3.tga";
	ndDemoMesh* const geometry = new ndDemoMesh("box", scene->GetShaderCache(), &box, textureName, textureName, textureName, 1.0f, uvMatrix);

	dMatrix matrix(dPitchMatrix(20.0f * dDegreeToRad));
	matrix.m_posit.m_y = 5.0f;
	ndDemoEntity* const entity = new ndDemoEntity(matrix, nullptr);
	entity->SetMesh(geometry, dGetIdentityMatrix());

	ndBodyDynamic* const body = new ndBodyDynamic();
	body->SetNotifyCallback(new ndDemoEntityNotify(scene, entity));
	body->SetMatrix(matrix);
	body->SetCollisionShape(box);

	world->AddBody(body);

	scene->AddEntity(entity);
	geometry->Release();
	return body;
}


void ndBasicFrictionRamp (ndDemoEntityManager* const scene)
{
	// build a floor
	BuildFloorBox(scene);

	BuildFrictionRamp(scene);

	dVector origin1(0.0f, 0.0f, 0.0f, 0.0f);
//	AddCapsulesStacks(scene, origin1);

	dQuaternion rot;
	dVector origin(-30.0f, 10.0f, 0.0f, 0.0f);
	scene->SetCameraMatrix(rot, origin);
}
