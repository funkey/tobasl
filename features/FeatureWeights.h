#ifndef MULTI2CUT_FEATURE_WEIGHTS_H__
#define MULTI2CUT_FEATURE_WEIGHTS_H__

#include <pipeline/Data.h>

class FeatureWeights : public pipeline::Data {

public:

	void setWeights(const std::vector<double>& weights) {

		_weights =  weights;
	}

	const std::vector<double>& getWeights() const {

		return _weights;
	}

private:

	std::vector<double> _weights;
};

#endif // MULTI2CUT_FEATURE_WEIGHTS_H__

