#include <limits>
#include <util/ProgramOptions.h>
#include <util/Logger.h>
#include "ContourDistanceLoss.h"
#include "Options.h"

logger::LogChannel contourdistancelosslog("contourdistancelosslog", "[ContourDistanceLoss] ");

ContourDistanceLoss::ContourDistanceLoss() :
	_maxCenterDistance(optionLossMaxCenterDistance),
	_overlap(false, false) {

	registerInput(_slices, "slices");
	registerInput(_groundTruth, "ground truth");
	registerOutput(_lossFunction, "loss function");
}

void
ContourDistanceLoss::updateOutputs() {

	LOG_USER(contourdistancelosslog) << "computing contour distance loss..." << std::endl;

	if (!_lossFunction)
		_lossFunction = new LossFunction();

	_lossFunction->clear();

	// loss for each slice
	foreach (boost::shared_ptr<Slice> slice, *_slices)
		getLoss(*slice);

	// constant offset
	double constant = 0;
	foreach (boost::shared_ptr<Slice> gtSlice, *_groundTruth)
		constant += _sliceDiameter(*gtSlice);

	_lossFunction->setConstant(constant);

	LOG_USER(contourdistancelosslog) << "done." << std::endl;
}

void
ContourDistanceLoss::getLoss(const Slice& slice) {

	// the reward part:

		// for each gt region with overlap, get the diameter of intersection
		// -> chose the max value, negate; this is the reward

	// the penalty part:

		// for the gt region with max overlap diameter, get the slice distance 
		// in both direction
		// -> sum them; this is the penalty

	double maxOverlapDiameter = 0;
	boost::shared_ptr<Slice> bestGtRegion;

	// for all GT slices in a threshold distance
	foreach (boost::shared_ptr<Slice> gtSlice, *_groundTruth) {

		// does overlap?
		if (!_overlap.exceeds(slice, *gtSlice, 0))
			continue;

		// reward

		ConnectedComponent overlap = slice.getComponent()->intersect(*gtSlice->getComponent());
		double overlapDiameter = _sliceDiameter(overlap);

		if (maxOverlapDiameter < overlapDiameter) {

			maxOverlapDiameter = overlapDiameter;
			bestGtRegion = gtSlice;
		}
	}

	// penalty

	double penalty = 0;

	if (bestGtRegion) {

		double _, gtToCandidate, candidateToGt;
		_sliceDistance(
				*bestGtRegion,
				slice,
				false /* not symmetric */,
				false /* don't align */,
				_ /* average not needed */,
				gtToCandidate);
		_sliceDistance(
				slice,
				*bestGtRegion,
				false /* not symmetric */,
				false /* don't align */,
				_ /* average not needed */,
				candidateToGt);

		penalty += gtToCandidate + candidateToGt;
	}

	(*_lossFunction)[slice.getId()] = penalty - maxOverlapDiameter;
}

