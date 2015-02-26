#include <limits>
#include <util/ProgramOptions.h>
#include <util/Logger.h>
#include "ContourDistanceLoss.h"
#include "Options.h"

util::ProgramOption optionHardLoss(
		util::_module           = "loss.contourdistanceloss",
		util::_long_name        = "hardLossThreshold",
		util::_description_text = "Turn the loss into a hard loss by thresholding with the given value: below threshold will be -1, above 1.");

logger::LogChannel contourdistancelosslog("contourdistancelosslog", "[ContourDistanceLoss] ");

ContourDistanceLoss::ContourDistanceLoss() :
	_maxCenterDistance(optionLossMaxCenterDistance),
	_hardLossThreshold(optionHardLoss.as<double>()),
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

	if (!optionHardLoss) {

		// constant offset
		double constant = 0;
		foreach (boost::shared_ptr<Slice> gtSlice, *_groundTruth)
			constant += _sliceDiameter(*gtSlice);

		_lossFunction->setConstant(constant);
	}

	_sliceDistance.clearCache();

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

	if (optionHardLoss) {

		// not overlapping with any gt region?
		if (!bestGtRegion) {

			(*_lossFunction)[slice.getId()] =  1; // bad!

		} else {

			// total score should be zero, if the candidate is a perfect fit
			double totalScore =
					_sliceDiameter(*bestGtRegion) - maxOverlapDiameter + penalty;

			if (totalScore > _hardLossThreshold)
				(*_lossFunction)[slice.getId()] =  1; // bad!
			else
				(*_lossFunction)[slice.getId()] = -1; // good!
		}

	} else {

		(*_lossFunction)[slice.getId()] = penalty - maxOverlapDiameter;
	}
}

