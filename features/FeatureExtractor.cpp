#include <fstream>
#include <sstream>
#include <region_features/RegionFeatures.h>
#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include <util/helpers.hpp>
#include "FeatureExtractor.h"

logger::LogChannel featureextractorlog("featureextractorlog", "[FeatureExtractor] ");

util::ProgramOption optionAddPairwiseFeatureProducts(
	util::_module           = "multi2cut.features",
	util::_long_name        = "addPairwiseProducts",
	util::_description_text = "For each pair of features f_i and f_j, add the product f_i*f_j to the feature vector as well."
);

util::ProgramOption optionAddFeatureSquares(
	util::_module           = "multi2cut.features",
	util::_long_name        = "addSquares",
	util::_description_text = "For each features f_i add the square f_i*f_i to the feature vector as well (implied by addPairwiseFeatureProducts)."
);

util::ProgramOption optionNormalize(
	util::_module           = "multi2cut.features",
	util::_long_name        = "normalize",
	util::_description_text = "Normalize each original feature to be in the interval [0,1]."
);

util::ProgramOption optionFeatureMinMaxFile(
	util::_module           = "multi2cut.features",
	util::_long_name        = "minMaxFile",
	util::_description_text = "For the feature normalization, instead of using the min and max of the extracted features, """""
	                          "use the min and max provided in the given file (first row min, second row max)."
);

FeatureExtractor::FeatureExtractor() {

	registerInput(_slices, "slices");
	registerInput(_image, "raw image");
	registerOutput(_features, "features");
}

void
FeatureExtractor::updateOutputs() {

	if (!_features)
		_features = new Features();

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
		_features->setFeatures(slice->getId(), regionFeatures.getFeatures(1));
	}

	// normalization

	if (optionNormalize) {

		LOG_USER(featureextractorlog) << "normalizing features" << std::endl;

		if (optionFeatureMinMaxFile) {

			LOG_USER(featureextractorlog)
					<< "reading feature minmax from "
					<< optionFeatureMinMaxFile.as<std::string>()
					<< std::endl;

			std::vector<double> min, max;

			std::ifstream minMaxFile(optionFeatureMinMaxFile.as<std::string>().c_str());
			std::string line;

			if (!minMaxFile.good())
				UTIL_THROW_EXCEPTION(
						IOError,
						"unable to open " << optionFeatureMinMaxFile.as<std::string>());

			// min
			{
				std::getline(minMaxFile, line);
				std::stringstream lineStream(line);

				while (lineStream.good()) {

					double f;

					lineStream >> f;
					if (lineStream.good())
						min.push_back(f);
				}
			}

			// max
			{
				std::getline(minMaxFile, line);
				std::stringstream lineStream(line);

				while (lineStream.good()) {

					double f;

					lineStream >> f;
					if (lineStream.good())
						max.push_back(f);
				}
			}

			LOG_ALL(featureextractorlog)
					<< "normalizing features with"
					<<  std::endl << min << std::endl
					<< "and"
					<<  std::endl << max << std::endl;

			_features->normalize(min, max);

		} else {

			_features->normalize();
		}
	}

	// postprocessing

	if (optionAddFeatureSquares || optionAddPairwiseFeatureProducts) {

		LOG_USER(featureextractorlog) << "adding feature products" << std::endl;

		foreach (boost::shared_ptr<Slice> slice, *_slices) {

			std::vector<double>& features = _features->getFeatures(slice->getId());

			if (optionAddPairwiseFeatureProducts) {

				// compute all products of all features and add them as well
				unsigned int numOriginalFeatures = features.size();
				for (unsigned int i = 0; i < numOriginalFeatures; i++)
					for (unsigned int j = i; j < numOriginalFeatures; j++)
						features.push_back(features[i]*features[j]);
			} else {

				// compute all squares of all features and add them as well
				unsigned int numOriginalFeatures = features.size();
				for (unsigned int i = 0; i < numOriginalFeatures; i++)
					features.push_back(features[i]*features[i]);
			}
		}
	}

	LOG_USER(featureextractorlog) << "done" << std::endl;
}
