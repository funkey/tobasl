#include <limits>
#include <util/ProgramOptions.h>
#include "SliceDistanceLoss.h"

util::ProgramOption optionLossMaxSliceDistance(
		util::_module           = "loss",
		util::_long_name        = "maxCenterDistance",
		util::_description_text = "The maximal center distance between candidates and ground truth to consider for computing the slice distance loss.",
		util::_default_value    = 1000);

SliceDistanceLoss::SliceDistanceLoss() :
	_maxSliceDistance(optionLossMaxSliceDistance) {

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

	double minDistance = std::numeric_limits<double>::infinity();

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
