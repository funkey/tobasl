#ifndef MULTI2CUT_LOSS_LOSS_COLLECTOR_H__
#define MULTI2CUT_LOSS_LOSS_COLLECTOR_H__

#include <pipeline/SimpleProcessNode.h>
#include "LossFunction.h"

class LossCollector : public pipeline::SimpleProcessNode<> {

public:

	LossCollector() {

		registerInputs(_losses, "loss functions");
		registerOutput(_loss, "loss function");
	}

private:

	void updateOutputs() {

		_loss = new LossFunction();

		unsigned int id;
		double costs;

		for (unsigned int i = 0; i < _losses.size(); i++)
			foreach (boost::tie(id, costs), *_losses[i])
				(*_loss)[id] = costs;
	}

	pipeline::Inputs<LossFunction> _losses;
	pipeline::Output<LossFunction> _loss;
};

#endif // MULTI2CUT_LOSS_LOSS_COLLECTOR_H__

