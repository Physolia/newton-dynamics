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

#include "ndBrainStdafx.h"
#include "ndBrainFloat4.h"
#include "ndBrainSaveLoad.h"
#include "ndBrainGpuBuffer.h"
#include "ndBrainGpuCommand.h"
#include "ndBrainGpuContext.h"
#include "ndBrainGpuFloatBuffer.h"
#include "ndBrainGpuIntegerBuffer.h"
#include "ndBrainGpuUniformBuffer.h"
#include "ndBrainLayerActivationRelu.h"

ndBrainLayerActivationRelu::ndBrainLayerActivationRelu(ndInt32 neurons)
	:ndBrainLayerActivation(neurons)
{
}

ndBrainLayerActivationRelu::ndBrainLayerActivationRelu(const ndBrainLayerActivationRelu& src)
	:ndBrainLayerActivation(src)
{
}

ndBrainLayer* ndBrainLayerActivationRelu::Clone() const
{
	return new ndBrainLayerActivationRelu(*this);
}

const char* ndBrainLayerActivationRelu::GetLabelId() const
{
	return "ndBrainLayerActivationRelu";
}

ndBrainLayer* ndBrainLayerActivationRelu::Load(const ndBrainLoad* const loadSave)
{
	char buffer[1024];
	loadSave->ReadString(buffer);

	loadSave->ReadString(buffer);
	ndInt32 inputs = loadSave->ReadInt();
	ndBrainLayerActivationRelu* const layer = new ndBrainLayerActivationRelu(inputs);
	loadSave->ReadString(buffer);
	return layer;
}

void ndBrainLayerActivationRelu::MakePrediction(const ndBrainVector& input, ndBrainVector& output) const
{
	ndAssert(input.GetCount() == output.GetCount());
	//for (ndInt32 i = ndInt32(input.GetCount() - 1); i >= 0; --i)
	//{
	//	output[i] = (input[i] > ndBrainFloat(0.0f)) ? input[i] : ndBrainFloat(0.0f);
	//	ndAssert(ndCheckFloat(output[i]));
	//}

	const ndBrainFloat4 zero(ndBrainFloat(0.0f));
	ndBrainFloat4* const dst = (ndBrainFloat4*)&output[0];
	const ndBrainFloat4* const src = (ndBrainFloat4*)&input[0];
	
	const ndInt32 count4 = ndInt32(output.GetCount() / 4);
	for (ndInt32 i = count4 - 1; i >= 0; --i)
	{
		ndBrainFloat4 test(src[i] > zero);
		dst[i] = test & src[i];
	}
	
	for (ndInt32 i = ndInt32(input.GetCount() - 1); i >= count4 * 4; --i)
	{
		output[i] = (input[i] > ndBrainFloat(0.0f)) ? input[i] : ndBrainFloat(0.0f);
	}
}

void ndBrainLayerActivationRelu::InputDerivative(const ndBrainVector&, const ndBrainVector& output, const ndBrainVector& outputDerivative, ndBrainVector& inputDerivative) const
{
	ndAssert(output.GetCount() == outputDerivative.GetCount());
	ndAssert(output.GetCount() == inputDerivative.GetCount());

	//for (ndInt32 i = ndInt32(output.GetCount() - 1); i >= 0; --i)
	//{
	//	inputDerivative[i] = (output[i] > ndBrainFloat(0.0f)) ? ndBrainFloat(1.0f) : ndBrainFloat(0.0f);
	//	ndAssert(ndCheckFloat(inputDerivative[i]));
	//}

	const ndBrainFloat4 one(ndBrainFloat(1.0f));
	const ndBrainFloat4 zero(ndBrainFloat(0.0f));
	const ndBrainFloat4* const src = (ndBrainFloat4*)&output[0];
	ndBrainFloat4* const dst = (ndBrainFloat4*)&inputDerivative[0];
	const ndInt32 count4 = ndInt32(output.GetCount() / 4);
	for (ndInt32 i = count4 - 1; i >= 0; --i)
	{
		ndBrainFloat4 test(src[i] > zero);
		dst[i] = test & one;
	}
	for (ndInt32 i = ndInt32(output.GetCount() - 1); i >= count4 * 4; --i)
	{
		inputDerivative[i] = (output[i] > ndBrainFloat(0.0f)) ? ndBrainFloat(1.0f) : ndBrainFloat(0.0f);
	}

	inputDerivative.Mul(outputDerivative);
}

ndBrainGpuCommand* ndBrainLayerActivationRelu::AssemblyGPUCommand(ndBrainGpuContext* const context, ndInt32 layerIndex, ndInt32 batchCount, ndFixSizeArray<ndBufferOffsetPair*, 8>& params)
{
	class ndBrainLayerActivationCommand : public ndBrainGpuCommand
	{
		public:
		struct UniformBufferObject
		{
			ndInt32 m_inputSize;
			ndInt32 m_inputStart;
			ndInt32 m_outputStart;
			ndInt32 m_workBufferSize;
		};

		ndBrainLayerActivationCommand(
			const ndBrainLayerActivation* const layer, ndBrainGpuContext* const context,
			ndInt32 layerIndex, ndInt32 batchCount,	const ndBufferOffsetPair& workingBuffer)
			:ndBrainGpuCommand(context)
			,m_parammeters(m_context, sizeof(UniformBufferObject))
		{
			UniformBufferObject uniformParam;
			uniformParam.m_inputSize = layer->GetInputSize();
			uniformParam.m_inputStart = workingBuffer.m_offsets[layerIndex + 0];
			uniformParam.m_outputStart = workingBuffer.m_offsets[layerIndex + 1];
			uniformParam.m_workBufferSize = workingBuffer.m_offsets[workingBuffer.m_offsets.GetCount() - 1];
			
			m_parammeters.LoadData(sizeof(uniformParam), &uniformParam);

			ndFixSizeArray<ndBrainGpuBuffer*, 4> params;
			params.PushBack(&m_parammeters);
			params.PushBack(workingBuffer.m_buffer);
			Assembly(context->m_ndBrainLayerRluActivation, batchCount, params.GetCount(), &params[0]);
		}

		ndBrainGpuUniformBuffer m_parammeters;
	};

	ndAssert(params.GetCount() == 2);
	const ndBufferOffsetPair& workingBuffer = *params[1];
	return new ndBrainLayerActivationCommand(this, context, layerIndex, batchCount, workingBuffer);
}