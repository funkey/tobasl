#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include "CoverLoss.h"

logger::LogChannel coverlosslog("coverlosslog", "[CoverLoss] ");

CoverLoss::CoverLoss() {

	registerInput(_slices, "slices");
	registerInput(_groundTruth, "ground truth");
	registerOutput(_lossFunction, "loss function");
}

void
CoverLoss::updateOutputs() {

	if (!_lossFunction)
		_lossFunction = new LossFunction();

	_lossFunction->clear();
	_constant = 0;

	// get the bounding box of all slices (gt and candidates)
	util::rect<unsigned int> bb(0, 0, 0, 0);
	foreach (boost::shared_ptr<Slice> slice, *_slices)
		bb.fit(slice->getComponent()->getBoundingBox());
	foreach (boost::shared_ptr<Slice> slice, *_groundTruth)
		bb.fit(slice->getComponent()->getBoundingBox());

	// make sure (0,0) is included
	bb.fit(util::point<unsigned int>(0, 0));

	LOG_DEBUG(coverlosslog) << "bounding box of all slices is " << bb << std::endl;

	// create multi array to store gt slice centroids
	_centroids.reshape(vigra::Shape2(bb.width(), bb.height()));

	// mark all gt centroids
	_centroids = false;
	foreach (boost::shared_ptr<Slice> slice, *_groundTruth) {

		util::point<unsigned int> centroid = slice->getComponent()->getCenter();
		_centroids(centroid.x, centroid.y) = true;

		// constant is total number of centroids
		_constant++;

		LOG_ALL(coverlosslog) << "gt centroid at " << centroid << std::endl;
	}

	// set candidate scores
	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		bool coversCentroid = false;
		foreach (const util::point<unsigned int>& p, slice->getComponent()->getPixels()) {

			if (_centroids(p.x, p.y)) {

				coversCentroid = true;
				LOG_ALL(coverlosslog) << "slice " << slice->getId() << " covers at least one centroid" << std::endl;
				break;
			}
		}

		(*_lossFunction)[slice->getId()] = (coversCentroid ? -1.0 : 0.0);
		LOG_ALL(coverlosslog) << "loss of slice " << slice->getId() << " = " << (*_lossFunction)[slice->getId()] << std::endl;
	}

	// set the constant
	_lossFunction->setConstant(_constant);
}


