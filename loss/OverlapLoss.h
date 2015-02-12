#ifndef MULTI2CUT_LOSS_OVERLAP_LOSS_H__
#define MULTI2CUT_LOSS_OVERLAP_LOSS_H__

#include <pipeline/SimpleProcessNode.h>

#include <slices/Slices.h>
#include <features/Overlap.h>
#include "LossFunction.h"

class OverlapLoss : public pipeline::SimpleProcessNode<> {

public:

	OverlapLoss();

private:

	void updateOutputs();

	double computeMaxGroundTruthOverlap(const Slice& slice);

	pipeline::Input<Slices> _groundTruth;
	pipeline::Input<Slices> _slices;

	pipeline::Output<LossFunction> _costs;

	// functor to compute overlap between two slices
	Overlap _overlap;

	double _setDifferenceScale;
};

#endif // MULTI2CUT_LOSS_OVERLAP_LOSS_H__


