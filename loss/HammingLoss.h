#ifndef MULTI2CUT_LOSS_HAMMING_LOSS_H__
#define MULTI2CUT_LOSS_HAMMING_LOSS_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/Slices.h>
#include "LossFunction.h"

/**
 * Good old Hamming loss, given a best-effort solution.
 */
class HammingLoss : public pipeline::SimpleProcessNode<> {

public:

	HammingLoss();

private:

	void updateOutputs();

	pipeline::Input<Slices>        _slices;
	pipeline::Input<Slices>        _bestEffort;
	pipeline::Output<LossFunction> _lossFunction;
};


#endif // MULTI2CUT_LOSS_HAMMING_H__

