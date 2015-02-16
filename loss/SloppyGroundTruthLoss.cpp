#include <limits>
#include <util/ProgramOptions.h>
#include <util/Logger.h>
#include "SloppyGroundTruthLoss.h"
#include "Options.h"

util::ProgramOption optionSloppiness(
		util::_module           = "loss.sloppygroundtruthloss",
		util::_long_name        = "sloppiness",
		util::_description_text = "The slice distance until which larger candidates will get a reward. This is to compensate for sloppy ground truth regions that are subsets of the real regions.",
		util::_default_value    = 5);

logger::LogChannel sloppygroundtruthlosslog("sloppygroundtruthlosslog", "[SloppyGroundTruthLoss] ");

SloppyGroundTruthLoss::SloppyGroundTruthLoss() :
	_sloppiness(optionSloppiness.as<double>()),
	_overlap(false, false) {

	registerInput(_slices, "slices");
	registerInput(_groundTruth, "ground truth");
	registerOutput(_lossFunction, "loss function");
}

void
SloppyGroundTruthLoss::updateOutputs() {

	LOG_USER(sloppygroundtruthlosslog) << "computing sloppy ground truth loss..." << std::endl;

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

	LOG_USER(sloppygroundtruthlosslog) << "done." << std::endl;
}

void
SloppyGroundTruthLoss::getLoss(const Slice& slice) {

	// the reward part:

		// for each gt region with overlap, get the diameter of intersection
		// -> chose the max value, negate; this is the reward

	// the penalty part:

		// for the gt region with max overlap diameter, get the slice distance 
		// in both direction
		//
		// for candidate to ground-truth, reward slice distance until 
		// _sloppiness, punish after that
		//
		// -> sum them; this is the penalty

	double maxOverlapDiameter = 0;
	boost::shared_ptr<Slice> bestGtRegion;

	// for all GT slices in a threshold distance
	foreach (boost::shared_ptr<Slice> gtSlice, *_groundTruth) {

		// does overlap?
		if (!_overlap.exceeds(slice, *gtSlice, 0))
			continue;

		ConnectedComponent overlap = slice.getComponent()->intersect(*gtSlice->getComponent());
		double overlapDiameter = _sliceDiameter(overlap);

		if (maxOverlapDiameter < overlapDiameter) {

			maxOverlapDiameter = overlapDiameter;
			bestGtRegion = gtSlice;
		}
	}

	if (!bestGtRegion) {

		(*_lossFunction)[slice.getId()] = _sliceDiameter(slice);
		return;
	}

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

	double sloppyDistance =
			std::max(candidateToGt - _sloppiness, 0.0) -
			std::min(candidateToGt, _sloppiness) +
			gtToCandidate;

	(*_lossFunction)[slice.getId()] = sloppyDistance - maxOverlapDiameter;
}

