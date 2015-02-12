#include <util/ProgramOptions.h>
#include "OverlapLoss.h"

util::ProgramOption optionOverlapLossSetDifferenceScale(
		util::_module           = "loss.overlaploss",
		util::_long_name        = "setDifferenceScale",
		util::_description_text = "The overlap loss is set_difference*w - overlap. This parameter sets the w (default 1.0).",
		util::_default_value    = 1.0);

OverlapLoss::OverlapLoss() :
		_overlap(false /* don't normalize */, false /* don't align */),
		_setDifferenceScale(optionOverlapLossSetDifferenceScale) {

	registerInput(_groundTruth, "ground truth");
	registerInput(_slices, "slices");

	registerOutput(_costs, "loss function");
}

void
OverlapLoss::updateOutputs() {

	_costs = new LossFunction();

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		double score = computeMaxGroundTruthOverlap(*slice);

		// get a reward for maximizing overlap
		(*_costs)[slice->getId()] = score;
	}
}

double
OverlapLoss::computeMaxGroundTruthOverlap(const Slice& slice) {

	double maxOverlap = 0;

	boost::shared_ptr<Slice> maxOverlapGtSlice;

	// find the slice with max overlap
	foreach (boost::shared_ptr<Slice> gtSlice, *_groundTruth) {

		double overlap = _overlap(slice, *gtSlice);

		if (overlap > maxOverlap) {

			maxOverlap        = overlap;
			maxOverlapGtSlice = gtSlice;
		}
	}

	// compute the set difference
	double setDifference;

	if (maxOverlapGtSlice)
		setDifference = slice.getComponent()->getSize() + maxOverlapGtSlice->getComponent()->getSize() - 2*maxOverlap;
	else
		setDifference = slice.getComponent()->getSize();

	return _setDifferenceScale*setDifference - maxOverlap;
}
