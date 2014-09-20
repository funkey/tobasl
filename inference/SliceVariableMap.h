#ifndef MULTI2CUT_INFERENCE_SLICE_VARIABLE_MAP_H__
#define MULTI2CUT_INFERENCE_SLICE_VARIABLE_MAP_H__

#include <map>

class SliceVariableMap {

public:

	void associate(unsigned int sliceId, unsigned int variableNum) {

		_varToSlice[variableNum] = sliceId;
		_sliceToVar[sliceId] = variableNum;
	}

	unsigned int getVariableNum(unsigned int sliceId) {

		return _sliceToVar[sliceId];
	}

	unsigned int getSliceId(unsigned int variableNum) {

		return _varToSlice[variableNum];
	}

	void clear() {

		_varToSlice.clear();
		_sliceToVar.clear();
	}

private:

	std::map<unsigned int, unsigned int> _varToSlice;
	std::map<unsigned int, unsigned int> _sliceToVar;
};

#endif // MULTI2CUT_INFERENCE_SLICE_VARIABLE_MAP_H__

