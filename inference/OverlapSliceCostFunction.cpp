#include "OverlapSliceCostFunction.h"

OverlapSliceCostFunction::OverlapSliceCostFunction() {

	registerInput(_groundTruth, "ground truth");
	registerInput(_slices, "slices");

	registerOutput(_costs, "slice costs");
}

void
OverlapSliceCostFunction::updateOutputs() {

	_costs = new SliceCosts();

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		double maxOverlap = computeMaxGroundTruthOverlap(*slice);

		// get a reward for maximizing overlap
		_costs->setCosts(slice->getId(), -maxOverlap);
	}
}

double
OverlapSliceCostFunction::computeMaxGroundTruthOverlap(const Slice& slice) {

	double maxOverlap = 0;

	foreach (boost::shared_ptr<Slice> gtSlice, *_groundTruth) {

		double overlap = _overlap(slice, *gtSlice);

		if (overlap > maxOverlap)
			maxOverlap = overlap;
	}

	return maxOverlap*maxOverlap;
}
