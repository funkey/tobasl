#ifndef MULTI2CUT_FEATURES_FEATURE_EXTRACTOR_H__
#define MULTI2CUT_FEATURES_FEATURE_EXTRACTOR_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/Slices.h>
#include "Features.h"

class FeatureExtractor : public pipeline::SimpleProcessNode<> {

public:

	FeatureExtractor();

private:

	void updateOutputs();

	pipeline::Input<Slices>    _slices;
	pipeline::Input<Image>     _image;
	pipeline::Output<Features> _features;
};

#endif // MULTI2CUT_FEATURES_FEATURE_EXTRACTOR_H__

