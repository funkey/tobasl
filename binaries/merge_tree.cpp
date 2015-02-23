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
#include <vigra/slic.hxx>
#include <vigra/multi_convolution.hxx>
#include <mergetree/IterativeRegionMerging.h>
#include <mergetree/MedianEdgeIntensity.h>
#include <mergetree/SmallFirst.h>
#include <mergetree/MultiplyMinRegionSize.h>
#include <mergetree/RandomPerturbation.h>

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

util::ProgramOption optionSuperpixelImage(
		util::_long_name        = "superpixelImage",
		util::_description_text = "Image with the initial superpixels.",
		util::_default_value    = "superpixels.png");

util::ProgramOption optionSuperpixelWithBordersImage(
		util::_long_name        = "superpixelWithBordersImage",
		util::_description_text = "Image with the initial superpixels.",
		util::_default_value    = "superpixels_borders.png");

util::ProgramOption optionRagFile(
		util::_long_name        = "ragFile",
		util::_description_text = "A file to write the region adjacency graph for the initial superpixels.");

util::ProgramOption optionSmooth(
		util::_long_name        = "smooth",
		util::_description_text = "Smooth the input image with a Gaussian kernel of the given stddev.");

util::ProgramOption optionSlicSuperpixels(
		util::_long_name        = "slicSuperpixels",
		util::_description_text = "Use SLIC superpixels instead of watersheds to obtain initial regions.");

util::ProgramOption optionSlicIntensityScaling(
		util::_long_name        = "slicIntensityScaling",
		util::_description_text = "How to scale the image intensity for comparison to spatial distance. Default is 1.0.",
		util::_default_value    = 1.0);

util::ProgramOption optionSliceSize(
		util::_long_name        = "slicSize",
		util::_description_text = "An upper limit on the SLIC superpixel size. Default is 10.",
		util::_default_value    = 10);

util::ProgramOption optionMergeSmallRegionsFirst(
		util::_long_name        = "mergeSmallRegionsFirst",
		util::_description_text = "Merge small regions first. For parameters, see smallRegionThreshold1, smallRegionThreshold2, and intensityThreshold.");

util::ProgramOption optionRandomPerturbation(
		util::_long_name        = "randomPerturbation",
		util::_short_name       = "r",
		util::_description_text = "Randomly (normally distributed) perturb the edge scores for merging.");

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

		if (optionSmooth)
			vigra::gaussianSmoothMultiArray(
					image,
					image,
					optionSmooth.as<double>());

		// perform watersheds or find SLIC superpixels
		vigra::MultiArray<2, int> initialRegions(image.shape());

		if (optionSlicSuperpixels) {

			unsigned int maxLabel = vigra::slicSuperpixels(
					image,
					initialRegions,
					optionSlicIntensityScaling.as<double>(),
					optionSliceSize.as<double>(),
					vigra::SlicOptions().iterations(100));

			LOG_USER(logger::out) << "found " << maxLabel << " SLIC superpixels" << std::endl;

		} else {

			unsigned int maxLabel = vigra::watershedsMultiArray(
					image,
					initialRegions,
					vigra::DirectNeighborhood,
					vigra::WatershedOptions().seedOptions(vigra::SeedOptions().extendedMinima()));

			LOG_USER(logger::out) << "found " << maxLabel << " watershed regions" << std::endl;

			vigra::exportImage(
					initialRegions,
					vigra::ImageExportInfo(optionSuperpixelImage.as<std::string>().c_str()));

			vigra::MultiArray<2, int> initialRegionsWithBorders(image.shape());
			initialRegionsWithBorders = initialRegions;

			for (unsigned int y = 0; y < initialRegions.height() - 1; y++)
				for (unsigned int x = 0; x < initialRegions.width() - 1; x++) {

					float value = initialRegions(x, y);
					float right = initialRegions(x+1, y);
					float down  = initialRegions(x, y+1);
					float diag  = initialRegions(x+1, y+1);

					if (value != right || value != down || value != diag)
						initialRegionsWithBorders(x, y) = 0;
				}

			vigra::exportImage(
					initialRegionsWithBorders,
					vigra::ImageExportInfo(optionSuperpixelWithBordersImage.as<std::string>().c_str()));
		}

		// extract merge tree
		IterativeRegionMerging merging(initialRegions);

		MedianEdgeIntensity mei(image);

		// create the RAG description for the median edge intensities
		if (optionRagFile)
			merging.storeRag(optionRagFile.as<std::string>(), mei);

		if (optionMergeSmallRegionsFirst) {

			SmallFirst<MedianEdgeIntensity> scoringFunction(
					merging.getRag(),
					image,
					initialRegions,
					mei);

			if (optionRandomPerturbation) {

				RandomPerturbation<SmallFirst<MedianEdgeIntensity> > rp(scoringFunction);
				merging.createMergeTree(rp);

			} else {

				merging.createMergeTree(scoringFunction);
			}

		} else {

			MultiplyMinRegionSize<MedianEdgeIntensity> scoringFunction(
					merging.getRag(),
					image,
					initialRegions,
					mei);

			if (optionRandomPerturbation) {

				RandomPerturbation<MultiplyMinRegionSize<MedianEdgeIntensity> > rp(scoringFunction);
				merging.createMergeTree(rp);

			} else {

				merging.createMergeTree(scoringFunction);
			}
		}

		LOG_USER(logger::out) << "writing merge tree..." << std::endl;

		vigra::exportImage(merging.getMergeTree(), vigra::ImageExportInfo(optionMergeTreeImage.as<std::string>().c_str()));

	} catch (Exception& e) {

		handleException(e, std::cerr);
	}
}

