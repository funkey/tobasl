#ifndef MULTI2CUT_IO_FEATURE_WEIGHTS_READER_H__
#define MULTI2CUT_IO_FEATURE_WEIGHTS_READER_H__

#include <pipeline/SimpleProcessNode.h>
#include <features/FeatureWeights.h>

class FeatureWeightsReader : public pipeline::SimpleProcessNode<> {

public:

	FeatureWeightsReader();

private:

	void updateOutputs();

	pipeline::Output<FeatureWeights> _weights;
};

#endif // MULTI2CUT_IO_FEATURE_WEIGHTS_READER_H__

