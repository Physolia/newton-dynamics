/* Copyright (c) <2003-2019> <Julio Jerez, Newton Game Dynamics>
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

#ifndef __D_SHAPE_NULL_H__ 
#define __D_SHAPE_NULL_H__ 

#include "dShapeConvex.h"

class dShapeNull: public dShapeConvex
{
	public:
	D_NEWTON_API dShapeNull();
	//dShapeNull(dUnsigned32 signature);
	//dShapeNull(dgWorld* const world, dgDeserialize deserialization, void* const userData, dgInt32 revisionNumber);
	virtual ~dShapeNull() {}

	virtual dShapeNull* GetAsShapeNull() { return this; }


	dVector SupportVertex(const dVector& dir, dInt32* const vertexIndex) const
	{
		return dVector(dFloat32(0.0f));
	}

	virtual void DebugShape(const dMatrix& matrix, dShapeDebugCallback& debugCallback) const
	{
	}

	virtual void CalcAABB(const dMatrix& matrix, dVector& p0, dVector& p1) const
	{
		p0 = dVector::m_zero;
		p1 = dVector::m_zero;
	}

/*
	protected:
	virtual dgFloat32 GetVolume () const;
	
	virtual dVector SupportVertex (const dVector& dir, dgInt32* const vertexIndex) const;
	virtual dVector SupportVertexSpecial (const dVector& dir, dgFloat32 skinThickness, dgInt32* const vertexIndex) const;
	
	virtual dVector CalculateVolumeIntegral (const dMatrix& globalMatrix, const dVector& plane, const dgCollisionInstance& parentScale) const;
	virtual dgFloat32 RayCast (const dVector& localP0, const dVector& localP1, dgFloat32 maxT, dgContactPoint& contactOut, const dgBody* const body, void* const userData, OnRayPrecastAction preFilter) const;

	private:
	virtual dgInt32 CalculateSignature () const;
	virtual void SetCollisionBBox (const dVector& p0, const dVector& p1);

	virtual void Serialize(dgSerialize callback, void* const userData) const;
	friend class dgWorld;
*/
};


#endif 

