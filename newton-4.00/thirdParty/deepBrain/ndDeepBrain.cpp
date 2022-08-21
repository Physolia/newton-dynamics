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

#include "ndDeepBrainStdafx.h"
#include "ndDeepBrain.h"

ndDeepBrain::ndDeepBrain()
	:ndArray<ndDeepBrainLayer*>()
	,m_isReady(false)
{
}

ndDeepBrain::ndDeepBrain(const ndDeepBrain& src)
	:ndArray<ndDeepBrainLayer*>()
	,m_isReady(src.m_isReady)
{
	const ndArray<ndDeepBrainLayer*>& srcLayers = src;
	for (ndInt32 i = 0; i < srcLayers.GetCount(); ++i)
	{
		ndDeepBrainLayer* const layer = srcLayers[i]->Clone();
		PushBack(layer);
	}
}

ndDeepBrain::~ndDeepBrain()
{
	for (ndInt32 i = GetCount() - 1; i >= 0 ; --i)
	{
		delete (*this)[i];
	}
}

void ndDeepBrain::CopyFrom(const ndDeepBrain& src)
{
	const ndArray<ndDeepBrainLayer*>& layers = *this;
	const ndArray<ndDeepBrainLayer*>& srcLayers = src;
	for (ndInt32 i = 0; i < layers.GetCount(); ++i)
	{
		layers[i]->CopyFrom(*srcLayers[i]);
	}
}

ndDeepBrainLayer* ndDeepBrain::AddLayer(ndDeepBrainLayer* const layer)
{
	ndAssert(!GetCount() || ((*this)[GetCount() - 1]->GetOuputSize() == layer->GetInputSize()));
	PushBack(layer);
	return layer;
}

void ndDeepBrain::BeginAddLayer()
{
	m_isReady = false;
}

void ndDeepBrain::EndAddLayer()
{
	InitGaussianWeights(0.0f, 0.25f);
	m_isReady = true;
}

bool ndDeepBrain::Compare(const ndDeepBrain& src) const
{
	if (m_isReady != src.m_isReady)
	{
		ndAssert(0);
		return false;
	}

	if (GetCount() != src.GetCount())
	{
		ndAssert(0);
		return false;
	}

	const ndArray<ndDeepBrainLayer*>& layers0 = *this;
	const ndArray<ndDeepBrainLayer*>& layers1 = src;
	for (ndInt32 i = 0; i < layers0.GetCount(); ++i)
	{
		bool test = layers0[i]->Compare(*layers1[i]);
		if (!test)
		{
			ndAssert(0);
			return false;
		}
	}

	return true;
}

void ndDeepBrain::InitGaussianWeights(ndReal mean, ndReal variance)
{
	ndArray<ndDeepBrainLayer*>& layers = *this;
	for (ndInt32 i = layers.GetCount() - 1; i >= 0; --i)
	{
		layers[i]->InitGaussianWeights(mean, variance);
	}
}

void ndDeepBrain::Save(const char* const pathName) const
{
	nd::TiXmlDocument asciifile;
	nd::TiXmlDeclaration* const decl = new nd::TiXmlDeclaration("1.0", "", "");
	asciifile.LinkEndChild(decl);

	nd::TiXmlElement* const rootNode = new nd::TiXmlElement("ndDeepBrain");
	asciifile.LinkEndChild(rootNode);

	for (ndInt32 i = 0; i < GetCount(); i++)
	{
		ndDeepBrainLayer* const layer = (*this)[i];
		nd::TiXmlElement* const layerNode = new nd::TiXmlElement("ndLayer");
		rootNode->LinkEndChild(layerNode);
		layer->Save (layerNode);
	}

	char* const oldloc = setlocale(LC_ALL, 0);
	setlocale(LC_ALL, "C");
	asciifile.SaveFile(pathName);
	setlocale(LC_ALL, oldloc);
}

bool ndDeepBrain::Load(const char* const pathName)
{
	char* const oldloc = setlocale(LC_ALL, 0);
	setlocale(LC_ALL, "C");

	nd::TiXmlDocument doc(pathName);
	doc.LoadFile();
	if (doc.Error())
	{
		setlocale(LC_ALL, oldloc);
		return false;
	}
	ndAssert(!doc.Error());

	const nd::TiXmlElement* const rootNode = doc.RootElement();
	if (!rootNode)
	{
		return false;
	}
	
	for (const nd::TiXmlNode* layerNode = rootNode->FirstChild("ndLayer"); layerNode; layerNode = layerNode->NextSibling())
	{
		const char* const layerType = xmlGetString(layerNode, "type");
		if (layerType)
		{
			ndDeepBrainLayer* layer = nullptr;
			if (!strcmp(layerType, "fullyConnected"))
			{
				layer = new ndDeepBrainLayer(layerNode);
			}
			else
			{
				ndAssert(0);
			}
			if (layer)
			{
				AddLayer(layer);
			}
		}
	}
	m_isReady = true;

	return true;
}
