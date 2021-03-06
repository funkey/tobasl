#include <fstream>
#include <sstream>
#include <vigra/flatmorphology.hxx>
#include <region_features/RegionFeatures.h>
#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include <util/helpers.hpp>
#include "FeatureExtractor.h"

logger::LogChannel featureextractorlog("featureextractorlog", "[FeatureExtractor] ");

util::ProgramOption optionShapeFeatures(
	util::_module           = "multi2cut.features",
	util::_long_name        = "shapeFeatures",
	util::_description_text = "Compute shape morphology features for each candidate.",
	util::_default_value    = true
);

util::ProgramOption optionProbabilityImageFeatures(
	util::_module           = "multi2cut.features",
	util::_long_name        = "probabilityImageFeatures",
	util::_description_text = "Use the probability image to compute image features for each candidate."
);

util::ProgramOption optionProbabilityImageBoundaryFeatures(
	util::_module           = "multi2cut.features",
	util::_long_name        = "probabilityImageBoundaryFeatures",
	util::_description_text = "Use the probability image to compute image features for the boundary of each candidate."
);

util::ProgramOption optionNoCoordinatesStatistics(
	util::_module           = "multi2cut.features",
	util::_long_name        = "noCoordinatesStatistics",
	util::_description_text = "Do not include statistics features that compute coordinates."
);

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

util::ProgramOption optionFeaturePointinessAnglePoints(
	util::_module           = "multi2cut.features.pointiness",
	util::_long_name        = "numAnglePoints",
	util::_description_text = "The number of points to sample equidistantly on the contour of slices. Default is 50.",
	util::_default_value    = 50
);

util::ProgramOption optionFeaturePointinessVectorLength(
	util::_module           = "multi2cut.features.pointiness",
	util::_long_name        = "angleVectorLength",
	util::_description_text = "The amount to walk on the contour from a sample point in either direction, to estimate the angle. Values are between "
	                          "0 (at the sample point) and 1 (at the next sample point). Default is 0.1.",
	util::_default_value    = 0.1
);

util::ProgramOption optionFeaturePointinessHistogramBins(
	util::_module           = "multi2cut.features.pointiness",
	util::_long_name        = "numHistogramBins",
	util::_description_text = "The number of histogram bins for the measured angles. Default is 16.",
	util::_default_value    = 16
);

FeatureExtractor::FeatureExtractor() {

	registerInput(_slices, "slices");
	registerInput(_rawImage, "raw image");
	registerInput(_probabilityImage, "probability image");
	registerOutput(_features, "features");
}

void
FeatureExtractor::updateOutputs() {

	if (!_features)
		_features = new Features();
	else
		_features->clear();

	if (_slices->size() == 0)
		return;

	LOG_USER(featureextractorlog)
			<< "extracting features for "
			<< _slices->size()
			<< " slices" << std::endl;

	//////////////////////////
	// TOPOLOGICAL FEATURES //
	//////////////////////////

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		_features->append(slice->getId(), slice->getLevel());
		_features->append(slice->getId(), slice->getNumDescendants());
	}

	/////////////////////
	// REGION FEATURES //
	/////////////////////

	foreach (boost::shared_ptr<Slice> slice, *_slices) {
		// the bounding box of the slice in the raw image
		const util::rect<unsigned int>& sliceBoundingBox = slice->getComponent()->getBoundingBox();

		LOG_DEBUG(featureextractorlog) << "extracting features for slice " << slice->getId() << std::endl;
		LOG_ALL(featureextractorlog) << "slice bounding box: " << sliceBoundingBox << std::endl;
		foreach (const util::point<unsigned int>& p, slice->getComponent()->getPixels())
			LOG_ALL(featureextractorlog) << "  " << p << std::endl;

		// a view to the raw image for the slice bounding box
		typedef vigra::MultiArrayView<2, float>::difference_type Shape;
		vigra::MultiArrayView<2, float> rawSliceImage =
				_rawImage->subarray(
						Shape(sliceBoundingBox.minX, sliceBoundingBox.minY),
						Shape(sliceBoundingBox.maxX, sliceBoundingBox.maxY));

		// the "label" image
		vigra::MultiArrayView<2, bool> labelImage = slice->getComponent()->getBitmap();

		// an adaptor to access the feature map with the right id
		FeatureIdAdaptor adaptor(slice->getId(), *_features);

		RegionFeatures<2, float, bool>::Parameters p;
		p.computeRegionprops = optionShapeFeatures;
		if (optionNoCoordinatesStatistics)
			p.statisticsParameters.computeCoordinateStatistics = false;
		p.regionpropsParameters.numAnglePoints = optionFeaturePointinessAnglePoints;
		p.regionpropsParameters.contourVecAsArcSegmentRatio = optionFeaturePointinessVectorLength;
		p.regionpropsParameters.numAngleHistBins = optionFeaturePointinessHistogramBins;
		RegionFeatures<2, float, bool> regionFeatures(rawSliceImage, labelImage, p);

		regionFeatures.fill(adaptor);

		if (optionProbabilityImageFeatures) {

			vigra::MultiArrayView<2, float> probabilitySliceImage =
					_probabilityImage->subarray(
							Shape(sliceBoundingBox.minX, sliceBoundingBox.minY),
							Shape(sliceBoundingBox.maxX, sliceBoundingBox.maxY));
			RegionFeatures<2, float, bool>::Parameters probParams;
			if (optionNoCoordinatesStatistics)
				probParams.statisticsParameters.computeCoordinateStatistics = false;
			RegionFeatures<2, float, bool> probRegionFeatures(probabilitySliceImage, labelImage, probParams);
			probRegionFeatures.fill(adaptor);

			if (optionProbabilityImageBoundaryFeatures) {

				// create the boundary image
				vigra::MultiArray<2, bool> erosionImage(labelImage.shape());
				vigra::discErosion(labelImage, erosionImage, 1);
				vigra::MultiArray<2, bool> boundaryImage(labelImage.shape());
				boundaryImage = labelImage;
				boundaryImage -= erosionImage;

				unsigned int width  = boundaryImage.width();
				unsigned int height = boundaryImage.height();

				for (unsigned int x = 0; x < width; x++) {

					boundaryImage(x, 0)		|= labelImage(x, 0);
					boundaryImage(x, height-1) |= labelImage(x, height-1);
				}

				for (unsigned int y = 1; y < height-1; y++) {

					boundaryImage(0, y)	   |= labelImage(0, y);
					boundaryImage(width-1, y) |= labelImage(width-1, y);
				}

				RegionFeatures<2, float, bool>::Parameters boundaryParams;
				if (optionNoCoordinatesStatistics)
					boundaryParams.statisticsParameters.computeCoordinateStatistics = false;
				RegionFeatures<2, float, bool> boundaryFeatures(probabilitySliceImage, boundaryImage, boundaryParams);
				boundaryFeatures.fill(adaptor);
			}
		}
	}

	LOG_USER(featureextractorlog)
			<< "extracted "
			<< _features->getFeatures((*_slices->begin())->getId()).size()
			<< " features" << std::endl;

	///////////////////
	// NORMALIZATION //
	///////////////////

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

	////////////////////
	// POSTPROCESSING //
	////////////////////

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

	// append a 1 for bias
	foreach (boost::shared_ptr<Slice> slice, *_slices)
		_features->append(slice->getId(), 1);

	LOG_USER(featureextractorlog)
			<< "after postprocessing, we have "
			<< _features->getFeatures((*_slices->begin())->getId()).size()
			<< " features" << std::endl;

	LOG_USER(featureextractorlog) << "done" << std::endl;
}
