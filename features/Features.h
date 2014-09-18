#ifndef MULTI2CUT_FEATURES_FEATURES_H__
#define MULTI2CUT_FEATURES_FEATURES_H__

#include <pipeline/Data.h>

class Features : public pipeline::Data {

public:

	Features(unsigned int numFeatures) :
		_numFeatures(numFeatures) {}

	void setFeatures(unsigned int sliceId, const std::vector<double>& features) {

		_features[sliceId] = features;
	}

	std::vector<double> getFeatures(unsigned int sliceId) {

		if (_features.count(sliceId) == 0)
			return std::vector<double>(0, _numFeatures);

		return _features[sliceId];
	}

private:

	unsigned int _numFeatures;

	std::map<unsigned int, std::vector<double> > _features;
};

#endif // MULTI2CUT_FEATURES_FEATURES_H__

