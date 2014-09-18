#include "FeatureExtractor.h"

FeatureExtractor::FeatureExtractor() {

	registerInput(_slices, "slices");
	registerOutput(_features, "features");
}

void
FeatureExtractor::updateOutputs() {

	const unsigned int numFeatures = 2;

	if (!_features)
		_features = new Features(numFeatures);

	std::vector<double> features(numFeatures);

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		double size  = slice->getComponent()->getSize();
		double size2 = size*size;

		features[0] = size;
		features[1] = size2;

		_features->setFeatures(slice->getId(), features);
	}
}
