#include "HammingLoss.h"

HammingLoss::HammingLoss() {

	registerInput(_slices, "slices");
	registerInput(_bestEffort, "best effort");
	registerOutput(_lossFunction, "loss function");
}

void
HammingLoss::updateOutputs() {

	if (!_lossFunction)
		_lossFunction = new LossFunction();

	foreach (boost::shared_ptr<Slice> slice, *_slices)
		(*_lossFunction)[slice->getId()] = 1;

	foreach (boost::shared_ptr<Slice> slice, *_bestEffort)
		(*_lossFunction)[slice->getId()] = -1;

	_lossFunction->setConstant(_bestEffort->size());
}
