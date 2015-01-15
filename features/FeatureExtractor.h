#ifndef MULTI2CUT_FEATURES_FEATURE_EXTRACTOR_H__
#define MULTI2CUT_FEATURES_FEATURE_EXTRACTOR_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/Slices.h>
#include "Features.h"

class FeatureExtractor : public pipeline::SimpleProcessNode<> {

public:

	FeatureExtractor();

private:

	/**
	 * Adaptor to be used with RegionFeatures, such that mapping to the correct 
	 * slice id is preserved.
	 */
	class FeatureIdAdaptor {

	public:
		FeatureIdAdaptor(unsigned int id, Features& features) : _id(id), _features(features) {}

		inline void append(unsigned int /*ignored*/, double value) { _features.append(_id, value); }

	private:

		unsigned int _id;
		Features&    _features;
	};

	void updateOutputs();

	pipeline::Input<Slices>    _slices;
	pipeline::Input<Image>     _rawImage;
	pipeline::Input<Image>     _probabilityImage;
	pipeline::Output<Features> _features;
};

#endif // MULTI2CUT_FEATURES_FEATURE_EXTRACTOR_H__

