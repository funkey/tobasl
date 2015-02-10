#include "SliceDistanceLoss.h"

SliceDistanceLoss::SliceDistanceLoss() {

	registerInput(_slices, "slices");
	registerInput(_groundTruth, "ground truth");
	registerOutput(_lossFunction, "loss function");
}

void
SliceDistanceLoss::updateOutputs() {

	if (!_lossFunction)
		_lossFunction = new LossFunction();

	_lossFunction->clear();

	// TODO:
	//
	// compute minimal slice distance of each candidate slice to any ground 
	// truth slice
}
