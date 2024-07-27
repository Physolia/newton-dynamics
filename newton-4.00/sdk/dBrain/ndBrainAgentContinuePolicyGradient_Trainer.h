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

#ifndef _ND_AGENT_CONTINUE_POLICY_GRADIENT_TRAINER_H__
#define _ND_AGENT_CONTINUE_POLICY_GRADIENT_TRAINER_H__

#include "ndBrainStdafx.h"
#include "ndBrain.h"
#include "ndBrainAgent.h"
#include "ndBrainThreadPool.h"
#include "ndBrainLayerActivationRelu.h"
#include "ndBrainLayerActivationTanh.h"
#include "ndBrainLossLeastSquaredError.h"
#include "ndBrainLayerActivationSigmoidLinear.h"

// this is an implementation of the vanilla policy Gradient as described in:
// https://spinningup.openai.com/en/latest/algorithms/vpg.html

class ndBrainOptimizerAdam;
class ndBrainAgentContinuePolicyGradient_TrainerMaster;

class ndBrainAgentContinuePolicyGradient_Trainer : public ndBrainAgent
{
	class ndTrajectoryStep : protected ndBrainVector
	{
		public:
		ndTrajectoryStep(ndInt32 actionsSize, ndInt32 obsevationsSize);

		ndInt32 GetStepNumber() const;
		ndBrainFloat GetReward(ndInt32 entry) const;
		ndBrainFloat GetAdvantage(ndInt32 entry) const;

		void SetStepNumber(ndInt32 count);
		void SetReward(ndInt32 entry, ndBrainFloat reward);
		void SetAdvantage(ndInt32 entry, ndBrainFloat advantage);

		ndBrainFloat* GetActions(ndInt32 entry);
		ndBrainFloat* GetObservations(ndInt32 entry);

		ndInt32 m_actionsSize;
		ndInt32 m_obsevationsSize;
	};
	public:
	ndBrainAgentContinuePolicyGradient_Trainer(ndSharedPtr<ndBrainAgentContinuePolicyGradient_TrainerMaster>& master);
	~ndBrainAgentContinuePolicyGradient_Trainer();

	ndBrain* GetActor();
	void SelectAction(ndBrainVector& actions) const;

	void InitWeights() { ndAssert(0); }
	virtual void OptimizeStep() { ndAssert(0); }
	void Save(ndBrainSave* const) { ndAssert(0); }
	bool IsTrainer() const { ndAssert(0); return true; }
	ndInt32 GetEpisodeFrames() const { ndAssert(0); return 0; }
	void InitWeights(ndBrainFloat, ndBrainFloat) { ndAssert(0); }

	virtual void Step();
	virtual void SaveTrajectory();
	virtual bool IsTerminal() const;

	ndBrainVector m_workingBuffer;
	ndTrajectoryStep m_trajectory;
	ndSharedPtr<ndBrainAgentContinuePolicyGradient_TrainerMaster> m_master;

	mutable std::random_device m_rd;
	mutable std::mt19937 m_gen;
	mutable std::normal_distribution<ndFloat32> m_d;

	friend class ndBrainAgentContinuePolicyGradient_TrainerMaster;
};

class ndBrainAgentContinuePolicyGradient_TrainerMaster : public ndBrainThreadPool
{
	class LastActivationLayer;
	public:
	class HyperParameters
	{
		public:
		HyperParameters();

		ndBrainFloat m_policyLearnRate;
		ndBrainFloat m_criticLearnRate;
		ndBrainFloat m_regularizer;
		ndBrainFloat m_discountFactor;

		ndInt32 m_threadsCount;
		ndInt32 m_numberOfActions;
		ndInt32 m_numberOfObservations;

		ndInt32 m_numberOfLayers;
		ndInt32 m_bashBufferSize;
		ndInt32 m_neuronPerLayers;
		ndInt32 m_maxTrajectorySteps;
		ndInt32 m_bashTrajectoryCount;
		ndInt32 m_extraTrajectorySteps;
		ndUnsigned32 m_randomSeed;
	};

	class MemoryStateValues: protected ndBrainVector
	{
		public:
		MemoryStateValues(ndInt32 obsevationsSize);
		ndBrainFloat GetReward(ndInt32 index) const;
		const ndBrainFloat* GetObservations(ndInt32 index) const;
		void SaveTransition(ndInt32 index, ndBrainFloat reward, const ndBrainFloat* const observations);

		ndInt32 m_obsevationsSize;
	};

	ndBrainAgentContinuePolicyGradient_TrainerMaster(const HyperParameters& hyperParameters);
	virtual ~ndBrainAgentContinuePolicyGradient_TrainerMaster();

	ndBrain* GetActor();
	ndBrain* GetCritic();

	const ndString& GetName() const;
	void SetName(const ndString& name);

	ndInt32 GetFramesCount() const;
	ndInt32 GetEposideCount() const;

	bool IsSampling() const;
	ndFloat32 GetAverageScore() const;
	ndFloat32 GetAverageFrames() const;

	void OptimizeStep();

	private:
	void Optimize();
	void OptimizePolicy();
	void OptimizeCritic();
	void UpdateBaseLineValue();

	ndBrain m_actor;
	ndBrain m_baseLineValue;
	ndBrainOptimizerAdam* m_optimizer;
	ndArray<ndBrainTrainer*> m_trainers;
	ndArray<ndBrainTrainer*> m_weightedTrainer;
	ndArray<ndBrainTrainer*> m_auxiliaryTrainers;
	ndBrainOptimizerAdam* m_baseLineValueOptimizer;
	ndArray<ndBrainTrainer*> m_baseLineValueTrainers;

	MemoryStateValues m_stateValues;
	ndArray<ndInt32> m_randomPermutation;
	ndBrainAgentContinuePolicyGradient_Trainer::ndTrajectoryStep m_trajectoryAccumulator;
	
	ndBrainFloat m_gamma;
	ndBrainFloat m_policyLearnRate;
	ndBrainFloat m_criticLearnRate;
	ndInt32 m_numberOfActions;
	ndInt32 m_numberOfObservations;
	ndInt32 m_frameCount;
	ndInt32 m_framesAlive;
	ndInt32 m_eposideCount;
	ndInt32 m_bashBufferSize;
	ndInt32 m_maxTrajectorySteps;
	ndInt32 m_extraTrajectorySteps;
	ndInt32 m_bashTrajectoryIndex;
	ndInt32 m_bashTrajectoryCount;
	ndInt32 m_bashTrajectorySteps;
	ndInt32 m_baseValueWorkingBufferSize;
	ndInt32 m_memoryStateIndex;
	ndInt32 m_memoryStateIndexFull;
	ndUnsigned32 m_randomSeed;
	ndBrainVector m_workingBuffer;
	ndMovingAverage<8> m_averageScore;
	ndMovingAverage<8> m_averageFramesPerEpisodes;
	ndString m_name;
	ndList<ndBrainAgentContinuePolicyGradient_Trainer*> m_agents;
	friend class ndBrainAgentContinuePolicyGradient_Trainer;
};

#endif 