#ifndef MULTI2CUT_LOSS_SLICE_DISTANCE_LOSS_H__
#define MULTI2CUT_LOSS_SLICE_DISTANCE_LOSS_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/Slices.h>
#include "LossFunction.h"

class SliceDistanceLoss : public pipeline::SimpleProcessNode<> {

public:

	SliceDistanceLoss();

private:

	void updateOutputs();

	pipeline::Input<Slices>        _slices;
	pipeline::Input<Slices>        _groundTruth;
	pipeline::Output<LossFunction> _lossFunction;
};

#endif // MULTI2CUT_LOSS_SLICE_DISTANCE_LOSS_H__

