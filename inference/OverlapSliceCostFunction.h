#ifndef MULTI2CUT_INFERENCE_OVERLAP_SLICE_COST_FUNCTION_H__
#define MULTI2CUT_INFERENCE_OVERLAP_SLICE_COST_FUNCTION_H__

#include <pipeline/SimpleProcessNode.h>

#include <slices/Slices.h>
#include <features/Overlap.h>
#include "SliceCosts.h"

class OverlapSliceCostFunction : public pipeline::SimpleProcessNode<> {

public:

	OverlapSliceCostFunction();

private:

	void updateOutputs();

	double computeMaxGroundTruthOverlap(const Slice& slice);

	pipeline::Input<Slices> _groundTruth;
	pipeline::Input<Slices> _slices;

	pipeline::Output<SliceCosts> _costs;

	// functor to compute overlap between two slices
	Overlap _overlap;
};

#endif // MULTI2CUT_INFERENCE_OVERLAP_SLICE_COST_FUNCTION_H__

