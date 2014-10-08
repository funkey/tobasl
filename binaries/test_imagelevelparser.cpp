#include <iostream>
#include <pipeline/Process.h>
#include <pipeline/Value.h>
#include <util/ProgramOptions.h>
#include <util/Logger.h>
#include <util/helpers.hpp>

#include <imageprocessing/io/ImageReader.h>
#include <imageprocessing/ComponentTreeExtractor.h>

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

		pipeline::Process<ImageReader>            imageReader(optionMergeTreeImage.as<std::string>());
		pipeline::Process<ComponentTreeExtractor<unsigned char> > componentTreeExtractor;

		componentTreeExtractor->setInput("image", imageReader->getOutput());

		pipeline::Value<ComponentTree> componentTree = componentTreeExtractor->getOutput("component tree");

		std::cout << "extracted " << componentTree->size() << " components" << std::endl;

	} catch (Exception& e) {

		handleException(e, std::cerr);
	}
}

