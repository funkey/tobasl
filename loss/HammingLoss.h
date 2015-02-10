#ifndef MULTI2CUT_LOSS_HAMMING_LOSS_H__
#define MULTI2CUT_LOSS_HAMMING_LOSS_H__

#include <pipeline/SimpleProcessNode.h>
#include <slices/Slices.h>
#include "LossFunction.h"

/**
 * Good old Hamming loss, given a best-effort solution.
 *
 * You can optionally supply a base loss function, which is assumed to be 
 * positive only. This loss will be scaled in the interval [0,1) and added to 
 * the Hamming loss, such that between two solutions with the same Hamming loss 
 * the one minimizing the base loss will be prefered.
 */
class HammingLoss : public pipeline::SimpleProcessNode<> {

public:

	HammingLoss();

private:

	void updateOutputs();

	pipeline::Input<Slices>        _slices;
	pipeline::Input<Slices>        _bestEffort;
	pipeline::Input<LossFunction>  _baseLoss;
	pipeline::Output<LossFunction> _lossFunction;
};


#endif // MULTI2CUT_LOSS_HAMMING_H__

