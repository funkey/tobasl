#ifndef MULTI2CUT_LOSS_SLICE_DISTANCE_LOSS_H__
#define MULTI2CUT_LOSS_SLICE_DISTANCE_LOSS_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/Slices.h>
#include <features/Distance.h>
#include "LossFunction.h"

class SliceDistanceLoss : public pipeline::SimpleProcessNode<> {

public:

	SliceDistanceLoss();

private:

	void updateOutputs();

	void getLoss(const Slice& slice);

	pipeline::Input<Slices>        _slices;
	pipeline::Input<Slices>        _groundTruth;
	pipeline::Output<LossFunction> _lossFunction;

	double _maxSliceDistance;

	Distance _sliceDistance;
};

#endif // MULTI2CUT_LOSS_SLICE_DISTANCE_LOSS_H__

