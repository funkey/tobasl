#ifndef MULTI2CUT_INFERENCE_SLICE_COSTS_H__
#define MULTI2CUT_INFERENCE_SLICE_COSTS_H__

#include <pipeline/Data.h>

class SliceCosts {

public:

	void setCosts(unsigned int sliceId, double costs) {

		_costs[sliceId] = costs;
	}

	double getCosts(unsigned int sliceId) {

		if (_costs.count(sliceId) == 0)
			return 0;

		return _costs[sliceId];
	}

	void clear() {

		_costs.clear();
	}

private:

	std::map<unsigned int, double> _costs;
};

#endif // MULTI2CUT_INFERENCE_SLICE_COSTS_H__

