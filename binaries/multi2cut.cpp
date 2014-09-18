/**
 * multi2cut main file. Initializes all objects.
 */

#include <iostream>
#include <fstream>
#include <pipeline/Process.h>
#include <pipeline/Value.h>
#include <util/ProgramOptions.h>
#include <util/Logger.h>
#include <util/helpers.hpp>

#include <imageprocessing/io/ImageReader.h>
#include <imageprocessing/MserParameters.h>
#include <slices/SliceExtractor.h>
#include <features/FeatureExtractor.h>
#include <io/LearningProblemWriter.h>

using namespace logger;

util::ProgramOption optionMergeTreeImage(
		util::_long_name        = "mergeTreeImage",
		util::_short_name       = "i",
		util::_description_text = "An image representing the merge tree.",
		util::_default_value    = "mergetree.png");

int main(int optionc, char** optionv) {

	try {

		/********
		 * INIT *
		 ********/

		// init command line parser
		util::ProgramOptions::init(optionc, optionv);

		// init logger
		LogManager::init();

		LOG_USER(out) << "[main] starting..." << std::endl;

		pipeline::Process<ImageReader>                    imageReader(optionMergeTreeImage.as<std::string>());
		pipeline::Process<SliceExtractor<unsigned char> > sliceExtractor(0, false);
		pipeline::Process<FeatureExtractor>               featureExtractor;
		pipeline::Process<LearningProblemWriter>          writer;

		pipeline::Value<MserParameters> parameters;

		sliceExtractor->setInput("membrane", imageReader->getOutput());
		sliceExtractor->setInput("mser parameters", parameters);

		featureExtractor->setInput("slices", sliceExtractor->getOutput("slices"));

		writer->setInput("slices", sliceExtractor->getOutput("slices"));
		writer->setInput("conflict sets", sliceExtractor->getOutput("conflict sets"));
		writer->setInput("features", featureExtractor->getOutput());

		writer->write("foo");

	} catch (Exception& e) {

		handleException(e, std::cerr);
	}
}

