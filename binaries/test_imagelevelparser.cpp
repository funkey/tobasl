#include <iostream>
#include <pipeline/Process.h>
#include <pipeline/Value.h>
#include <util/ProgramOptions.h>
#include <util/Logger.h>
#include <util/helpers.hpp>

#include <imageprocessing/io/ImageReader.h>
#include <imageprocessing/ComponentTreeExtractor.h>
#include <imageprocessing/ComponentTreeDownSampler.h>
#include <imageprocessing/gui/ComponentTreeView.h>

#include <gui/RotateView.h>
#include <gui/ZoomView.h>
#include <gui/Window.h>

using namespace logger;

util::ProgramOption optionMergeTreeImage(
		util::_long_name        = "mergeTreeImage",
		util::_short_name       = "i",
		util::_description_text = "An image representing the merge tree.",
		util::_default_value    = "mergetree.png");

util::ProgramOption optionVisualize(
		util::_long_name        = "visualize",
		util::_description_text = "Show a visualization of the component tree.");

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

		if (optionVisualize) {

			pipeline::Process<ComponentTreeDownSampler> downsampler;

			pipeline::Process<ComponentTreeView> componentTreeView;
			pipeline::Process<gui::RotateView>   rotateView;
			pipeline::Process<gui::ZoomView>     zoomView;
			pipeline::Process<gui::Window>       window("imagelevelparser");

			downsampler->setInput(componentTreeExtractor->getOutput());
			componentTreeView->setInput(downsampler->getOutput());
			rotateView->setInput(componentTreeView->getOutput());
			zoomView->setInput(rotateView->getOutput());
			window->setInput(zoomView->getOutput());

			window->processEvents();
		}

	} catch (Exception& e) {

		handleException(e, std::cerr);
	}
}

