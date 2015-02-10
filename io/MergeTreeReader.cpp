#include "MergeTreeReader.h"

MergeTreeReader::MergeTreeReader(std::string mergeTreeImage) :
	_imageReader(mergeTreeImage),
	_sliceExtractor(0, true /* downsample */) {

	registerOutput(_sliceExtractor->getOutput("slices"), "slices");
	registerOutput(_sliceExtractor->getOutput("conflict sets"), "conflict sets");

	_sliceExtractor->setInput(_imageReader->getOutput());
}
