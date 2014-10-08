#include "OverlapSliceCostFunction.h"

OverlapSliceCostFunction::OverlapSliceCostFunction() :
		_overlap(false /* don't normalize */, false /* don't align */) {

	registerInput(_groundTruth, "ground truth");
	registerInput(_slices, "slices");

	registerOutput(_costs, "slice costs");
}

void
OverlapSliceCostFunction::updateOutputs() {

	_costs = new SliceCosts();

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		double score = computeMaxGroundTruthOverlap(*slice);

		// get a reward for maximizing overlap
		_costs->setCosts(slice->getId(), score);
	}
}

double
OverlapSliceCostFunction::computeMaxGroundTruthOverlap(const Slice& slice) {

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

	return setDifference - maxOverlap;
}
