#ifndef MULTI2CUT_INFERENCE_LINEAR_SLICE_COST_FUNCTION_H__
#define MULTI2CUT_INFERENCE_LINEAR_SLICE_COST_FUNCTION_H__

#include <pipeline/SimpleProcessNode.h>

#include <features/Features.h>
#include <features/FeatureWeights.h>
#include <slices/Slices.h>
#include "SliceCosts.h"

class LinearSliceCostFunction : public pipeline::SimpleProcessNode<> {

public:

	LinearSliceCostFunction();

private:

	void updateOutputs();

	pipeline::Input<Slices>         _slices;
	pipeline::Input<Features>       _features;
	pipeline::Input<FeatureWeights> _featureWeights;

	pipeline::Output<SliceCosts> _costs;
};

#endif // MULTI2CUT_INFERENCE_LINEAR_SLICE_COST_FUNCTION_H__

