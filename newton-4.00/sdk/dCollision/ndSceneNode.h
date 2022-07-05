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

#ifndef __ND_SCENE_NODE_H__
#define __ND_SCENE_NODE_H__

#include "ndCollisionStdafx.h"

class ndBodyKinematic;
class ndSceneBodyNode;
class ndSceneTreeNode;

D_MSV_NEWTON_ALIGN_32
class ndSceneNode : public ndContainersFreeListAlloc<ndSceneNode>
{
	public:
	ndSceneNode(ndSceneNode* const parent);
	virtual ~ndSceneNode();

	void Kill();
	void GetAabb(ndVector& minBox, ndVector& maxBox) const;
	void SetAabb(const ndVector& minBox, const ndVector& maxBox);

	virtual ndSceneNode* GetAsSceneNode() const;
	virtual ndSceneBodyNode* GetAsSceneBodyNode() const;
	virtual ndSceneTreeNode* GetAsSceneTreeNode() const;
	
	virtual ndSceneNode* GetLeft() const;
	virtual ndSceneNode* GetRight() const;
	virtual ndBodyKinematic* GetBody() const;

	virtual bool SanityCheck(ndUnsigned32 level) const;

	ndVector m_minBox;
	ndVector m_maxBox;
	ndSceneNode* m_parent;
	ndSpinLock m_lock;
	ndInt32 m_depthLevel;
	ndUnsigned8 m_isDead;
	ndUnsigned8 m_bhvLinked;
#ifdef _DEBUG
	ndInt32 m_nodeId;
#endif

	static ndVector m_aabbQuantization;
	static ndVector m_aabbInvQuantization;
} D_GCC_NEWTON_ALIGN_32 ;

D_MSV_NEWTON_ALIGN_32
class ndSceneBodyNode: public ndSceneNode
{
	public:
	ndSceneBodyNode(ndBodyKinematic* const body);
	virtual ~ndSceneBodyNode();

	virtual ndSceneBodyNode* GetAsSceneBodyNode() const 
	{ 
		return (ndSceneBodyNode*)this;
	}

	virtual ndBodyKinematic* GetBody() const
	{
		return m_body;
	}

	ndBodyKinematic* m_body;
} D_GCC_NEWTON_ALIGN_32 ;

class ndSceneTreeNode: public ndSceneNode
{
	public:
	ndSceneTreeNode();
	virtual ~ndSceneTreeNode();

	virtual ndSceneTreeNode* GetAsSceneTreeNode() const 
	{ 
		return (ndSceneTreeNode*)this;
	}
	
	virtual ndSceneNode* GetLeft() const
	{
		return m_left;
	}
	
	virtual ndSceneNode* GetRight() const
	{
		return m_right;
	}

	bool SanityCheck(ndUnsigned32 level) const;

	ndSceneNode* m_left;
	ndSceneNode* m_right;
} D_GCC_NEWTON_ALIGN_32;

inline ndSceneNode::ndSceneNode(ndSceneNode* const parent)
	:ndContainersFreeListAlloc<ndSceneNode>()
	,m_minBox(ndFloat32(-1.0e15f))
	,m_maxBox(ndFloat32(1.0e15f))
	,m_parent(parent)
	,m_lock()
	,m_isDead(0)
	,m_depthLevel(0)
	,m_bhvLinked(0)
{
#ifdef _DEBUG
	m_nodeId = 0;
#endif
}

inline ndSceneNode::~ndSceneNode()
{
}

inline ndSceneNode* ndSceneNode::GetAsSceneNode() const
{ 
	return (ndSceneNode*)this;
}

inline ndSceneBodyNode* ndSceneNode::GetAsSceneBodyNode() const
{ 
	return nullptr; 
}

inline ndSceneTreeNode* ndSceneNode::GetAsSceneTreeNode() const
{ 
	return nullptr; 
}

inline ndBodyKinematic* ndSceneNode::GetBody() const
{
	return nullptr;
}

inline ndSceneNode* ndSceneNode::GetLeft() const
{
	return nullptr;
}

inline ndSceneNode* ndSceneNode::GetRight() const
{
	return nullptr;
}

inline void ndSceneNode::Kill()
{
	m_isDead = 1;
}

inline void ndSceneNode::GetAabb(ndVector& minBox, ndVector& maxBox) const
{
	minBox = m_minBox;
	maxBox = m_maxBox;
}

inline void ndSceneNode::SetAabb(const ndVector& minBox, const ndVector& maxBox)
{
	dAssert(minBox.m_x <= maxBox.m_x);
	dAssert(minBox.m_y <= maxBox.m_y);
	dAssert(minBox.m_z <= maxBox.m_z);

	const ndVector p0(minBox * m_aabbQuantization);
	const ndVector p1(maxBox * m_aabbQuantization + ndVector::m_one);

	m_minBox = p0.Floor() * m_aabbInvQuantization;
	m_maxBox = p1.Floor() * m_aabbInvQuantization;

	dAssert(m_minBox.m_w == ndFloat32(0.0f));
	dAssert(m_maxBox.m_w == ndFloat32(0.0f));
}

#endif
