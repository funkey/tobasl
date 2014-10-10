#include "FeatureExtractor.h"
#include <region_features/RegionFeatures.h>
#include <util/Logger.h>

logger::LogChannel featureextractorlog("featureextractorlog", "[FeatureExtractor] ");

FeatureExtractor::FeatureExtractor() {

	registerInput(_slices, "slices");
	registerInput(_image, "raw image");
	registerOutput(_features, "features");
}

void
FeatureExtractor::updateOutputs() {

	const unsigned int numFeatures = 2;

	if (!_features)
		_features = new Features(numFeatures);

	if (_slices->size() == 0)
		return;

	RegionFeatures<2, float, bool> regionFeatures;

	LOG_USER(featureextractorlog) << "extracting features for " << _slices->size() << " slices" << std::endl;

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		// the bounding box of the slice in the raw image
		const util::rect<unsigned int>& sliceBoundingBox = slice->getComponent()->getBoundingBox();

		// a view to the raw image for the slice bounding box
		typedef vigra::MultiArrayView<2, float>::difference_type Shape;
		vigra::MultiArrayView<2, float> sliceImage =
				_image->subarray(
						Shape(sliceBoundingBox.minX, sliceBoundingBox.minY),
						Shape(sliceBoundingBox.maxX, sliceBoundingBox.maxY));

		// the "label" image
		vigra::MultiArrayView<2, bool> labelImage = slice->getComponent()->getBitmap();

		regionFeatures.computeFeatures(sliceImage, labelImage);

		// get the features for "region 1"
		std::vector<double> features = regionFeatures.getFeatures(1);

		// compute all products of all features and add them as well
		unsigned int numOriginalFeatures = features.size();
		for (unsigned int i = 0; i < numOriginalFeatures; i++)
			for (unsigned int j = i; j < numOriginalFeatures; j++)
				features.push_back(features[i]*features[j]);

		_features->setFeatures(slice->getId(), features);
	}

	LOG_USER(featureextractorlog) << "done" << std::endl;
}
