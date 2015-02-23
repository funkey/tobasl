#include "MergeTreeReader.h"
#include <util/ProgramOptions.h>

util::ProgramOption optionSpacedEdgeImage(
		util::_long_name        = "spacedEdgeImage",
		util::_description_text = "Indicate that the merge tree image(s) are spaced edge images (if they are, you would know).");

MergeTreeReader::MergeTreeReader(std::string mergeTreeImage) :
	_imageReader(mergeTreeImage),
	_sliceExtractor(0, true /* downsample */, false /* brightToDark */, optionSpacedEdgeImage) {

	registerOutput(_sliceExtractor->getOutput("slices"), "slices");
	registerOutput(_sliceExtractor->getOutput("conflict sets"), "conflict sets");

	_sliceExtractor->setInput(_imageReader->getOutput());
}
