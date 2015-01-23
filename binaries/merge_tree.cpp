/**
 * merge_tree
 *
 * Creates a gray-level merge-tree image given a region boundary prediction 
 * image.
 */

#include <iostream>
#include <fstream>
#include <util/ProgramOptions.h>
#include <util/Logger.h>
#include <util/exceptions.h>
#include <util/helpers.hpp>
#include <vigra/impex.hxx>
#include <vigra/multi_array.hxx>
#include <mergetree/IterativeRegionMerging.h>
#include <mergetree/MedianEdgeIntensity.h>

util::ProgramOption optionSourceImage(
		util::_long_name        = "source",
		util::_short_name       = "s",
		util::_description_text = "An image to compute the merge tree for.",
		util::_default_value    = "source.png");

util::ProgramOption optionMergeTreeImage(
		util::_long_name        = "mergeTreeImage",
		util::_short_name       = "m",
		util::_description_text = "An image representing the merge tree.",
		util::_default_value    = "mergetree.png");

using namespace logger;

vigra::MultiArray<2, float>
readImage(std::string filename) {

	// get information about the image to read
	vigra::ImageImportInfo info(filename.c_str());

	// abort if image is not grayscale
	if (!info.isGrayscale()) {

		UTIL_THROW_EXCEPTION(
				IOError,
				filename << " is not a gray-scale image!");
	}

	vigra::MultiArray<2, float> image(info.shape());

	// read image
	importImage(info, vigra::destImage(image));

	return image;
}

int main(int optionc, char** optionv) {

	try {

		/********
		 * INIT *
		 ********/

		// init command line parser
		util::ProgramOptions::init(optionc, optionv);

		// init logger
		logger::LogManager::init();

		// read image
		vigra::MultiArray<2, float> image = readImage(optionSourceImage);

		// perform watersheds
		vigra::MultiArray<2, int> initialRegions(image.shape());
		unsigned int maxLabel = vigra::watershedsMultiArray(
				image,
				initialRegions,
				vigra::DirectNeighborhood,
				vigra::WatershedOptions().seedOptions(vigra::SeedOptions().extendedMinima()));

		LOG_USER(logger::out) << "found " << maxLabel << " watershed regions" << std::endl;

		// extract merge tree
		IterativeRegionMerging merging(initialRegions);


		MedianEdgeIntensity mei(image);
		merging.createMergeTree(mei);

		vigra::exportImage(merging.getMergeTree(), vigra::ImageExportInfo(optionMergeTreeImage.as<std::string>().c_str()));

	} catch (Exception& e) {

		handleException(e, std::cerr);
	}
}

