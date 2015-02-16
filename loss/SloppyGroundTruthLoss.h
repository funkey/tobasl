#ifndef MULTI2CUT_LOSS_SLOPPY_GROUNDTRUTH_LOSS_H__
#define MULTI2CUT_LOSS_SLOPPY_GROUNDTRUTH_LOSS_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/Slices.h>
#include <features/Distance.h>
#include <features/Diameter.h>
#include <features/Overlap.h>
#include "LossFunction.h"

class SloppyGroundTruthLoss : public pipeline::SimpleProcessNode<> {

public:

	SloppyGroundTruthLoss();

private:

	void updateOutputs();

	void getLoss(const Slice& slice);

	pipeline::Input<Slices>        _slices;
	pipeline::Input<Slices>        _groundTruth;
	pipeline::Output<LossFunction> _lossFunction;

	double _sloppiness;

	Distance _sliceDistance;
	Diameter _sliceDiameter;
	Overlap  _overlap;
};

#endif // MULTI2CUT_LOSS_SLOPPY_GROUNDTRUTH_LOSS_H__

