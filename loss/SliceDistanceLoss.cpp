#include <limits>
#include <util/ProgramOptions.h>
#include "SliceDistanceLoss.h"
#include "Options.h"

SliceDistanceLoss::SliceDistanceLoss() :
	_maxSliceDistance(optionLossMaxCenterDistance) {

	registerInput(_slices, "slices");
	registerInput(_groundTruth, "ground truth");
	registerOutput(_lossFunction, "loss function");
}

void
SliceDistanceLoss::updateOutputs() {

	if (!_lossFunction)
		_lossFunction = new LossFunction();

	_lossFunction->clear();

	foreach (boost::shared_ptr<Slice> slice, *_slices)
		getLoss(*slice);
}

void
SliceDistanceLoss::getLoss(const Slice& slice) {

	std::vector<boost::shared_ptr<Slice> > gtSlices =
			_groundTruth->find(slice.getComponent()->getCenter(), _maxSliceDistance);

	double minDistance = _sliceDiameter(slice);

	// for all slices in a threshold distance
	foreach (boost::shared_ptr<Slice> gtSlice, gtSlices) {

		double distance, _;

		_sliceDistance(
				slice,
				*gtSlice,
				true,    /* symmetric */
				false,   /* don't align */
				_,       /* average slice distance, not needed */
				distance /* max slice distance */
		);

		minDistance = std::min(minDistance, distance);
	}

	(*_lossFunction)[slice.getId()] = minDistance;
}
