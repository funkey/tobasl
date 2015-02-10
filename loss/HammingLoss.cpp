#include "HammingLoss.h"

HammingLoss::HammingLoss() {

	registerInput(_slices, "slices");
	registerInput(_bestEffort, "best effort");
	registerInput(_baseLoss, "base loss function", pipeline::Optional);
	registerOutput(_lossFunction, "loss function");
}

void
HammingLoss::updateOutputs() {

	if (!_lossFunction)
		_lossFunction = new LossFunction();

	_lossFunction->clear();

	if (_baseLoss.isSet()) {

		*_lossFunction = *_baseLoss;

		// normalize base loss to be in [0,1)
		double baseUpperBound = 0;
		unsigned int id;
		double costs;
		foreach (boost::tie(id, costs), *_lossFunction)
			baseUpperBound += costs;
		foreach (boost::tie(id, costs), *_lossFunction)
			(*_lossFunction)[id] /= baseUpperBound;
	}

	foreach (boost::shared_ptr<Slice> slice, *_slices)
		(*_lossFunction)[slice->getId()] += 1;

	double constant = 0;

	foreach (boost::shared_ptr<Slice> slice, *_bestEffort) {

		// -2 to get -1 for best effort, since we added 1 already for all slices
		(*_lossFunction)[slice->getId()] -= 2;
		constant -= (*_lossFunction)[slice->getId()];
	}

	_lossFunction->setConstant(constant);
}
