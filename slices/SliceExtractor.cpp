#include <util/ProgramOptions.h>
#include <imageprocessing/ComponentTree.h>
#include <imageprocessing/ComponentTreeDownSampler.h>
#include <imageprocessing/ComponentTreePruner.h>
#include <imageprocessing/ComponentTreeExtractor.h>
#include <pipeline/Value.h>
#include "ComponentTreeConverter.h"
#include "SliceExtractor.h"

static logger::LogChannel sliceextractorlog("sliceextractorlog", "[SliceExtractor] ");

util::ProgramOption optionInvertSliceMaps(
		util::_long_name        = "invertSliceMaps",
		util::_description_text = "Invert the meaning of the slice map. The default "
		                          "(not inverting) is: bright area = neuron hypotheses.");

util::ProgramOption optionMinSliceSize(
		util::_long_name        = "minSliceSize",
		util::_description_text = "The minimal size of a neuron slice in pixels.",
		util::_default_value    = 10);

util::ProgramOption optionMaxSliceSize(
		util::_long_name        = "maxSliceSize",
		util::_description_text = "The maximal size of a neuron slice in pixels.",
		util::_default_value    = 100*100);

util::ProgramOption optionMaxSliceMerges(
		util::_long_name        = "maxSliceMerges",
		util::_description_text = "Limit the height of the slice component tree, counting the height from the leafs.",
		util::_default_value    = 3);

util::ProgramOption optionSpacedEdgeImage(
		util::_long_name        = "spacedEdgeImage",
		util::_description_text = "Indicate that the slice maps are spaced edge images (if they are, you would know).");

template <typename Precision>
SliceExtractor<Precision>::SliceExtractor(unsigned int section, bool downsample) :
	_componentTreeExtractor(boost::make_shared<ComponentTreeExtractor<Precision> >()),
	_defaultComponentTreeExtractorParameters(boost::make_shared<ComponentTreeExtractorParameters>()),
	_downSampler(boost::make_shared<ComponentTreeDownSampler>()),
	_pruner(boost::make_shared<ComponentTreePruner>()),
	_converter(boost::make_shared<ComponentTreeConverter>(section)) {

	registerInput(_componentTreeExtractor->getInput("image"), "membrane");
	registerOutput(_converter->getOutput("slices"), "slices");
	registerOutput(_converter->getOutput("conflict sets"), "conflict sets");

	// set default componentTreeExtractor parameters from program options
	_defaultComponentTreeExtractorParameters->darkToBright    =  optionInvertSliceMaps;
	_defaultComponentTreeExtractorParameters->minSize         =  optionMinSliceSize;
	_defaultComponentTreeExtractorParameters->maxSize         =  optionMaxSliceSize;
	_defaultComponentTreeExtractorParameters->spacedEdgeImage =  optionSpacedEdgeImage;

	LOG_DEBUG(sliceextractorlog)
			<< "extracting slices with min size " << optionMinSliceSize.as<int>()
			<< ", max size " << optionMaxSliceSize.as<int>()
			<< ", and max tree depth " << optionMaxSliceMerges.as<int>()
			<< std::endl;

	// setup internal pipeline
	_componentTreeExtractor->setInput("parameters", _defaultComponentTreeExtractorParameters);

	if (downsample) {

		_downSampler->setInput(_componentTreeExtractor->getOutput());
		_pruner->setInput("component tree", _downSampler->getOutput());

	} else {

		_pruner->setInput("component tree", _componentTreeExtractor->getOutput());
	}
	_pruner->setInput("max height", pipeline::Value<int>(optionMaxSliceMerges.as<int>()));
	_converter->setInput(_pruner->getOutput());
}

// explicit template instantiations
template class SliceExtractor<unsigned char>;
template class SliceExtractor<unsigned short>;
