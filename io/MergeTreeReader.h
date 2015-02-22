#ifndef MULTI2CUT_IO_MERGE_TREE_READER_H__
#define MULTI2CUT_IO_MERGE_TREE_READER_H__

#include <pipeline/SimpleProcessNode.h>
#include <pipeline/Process.h>
#include <imageprocessing/io/ImageReader.h>
#include <slices/SliceExtractor.h>

class MergeTreeReader : public pipeline::SimpleProcessNode<> {

public:

	MergeTreeReader(std::string mergeTreeFile);

private:

	void updateOutputs() {}

	pipeline::Process<ImageReader>                     _imageReader;
	pipeline::Process<SliceExtractor<unsigned short> > _sliceExtractor;
};

#endif // MULTI2CUT_IO_MERGE_TREE_READER_H__

