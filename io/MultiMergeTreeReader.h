#ifndef MULTI2CUT_IO_MULTI_MERGE_TREE_READER_H__
#define MULTI2CUT_IO_MULTI_MERGE_TREE_READER_H__

#include <pipeline/SimpleProcessNode.h>
#include <pipeline/Process.h>
#include <slices/SlicesCollector.h>

class MultiMergeTreeReader : public pipeline::SimpleProcessNode<> {

public:

	MultiMergeTreeReader(std::string mergeTreesDirectory);

private:

	void updateOutputs() {}

	pipeline::Process<SlicesCollector> _slicesCollector;
};

#endif // MULTI2CUT_IO_MULTI_MERGE_TREE_READER_H__

