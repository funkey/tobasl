#include "LinearSliceCostFunction.h"

LinearSliceCostFunction::LinearSliceCostFunction() {

	registerInput(_slices, "slices");
	registerInput(_features, "features");
	registerInput(_featureWeights, "feature weights");

	registerOutput(_costs, "slice costs");
}

void
LinearSliceCostFunction::updateOutputs() {

	_costs = new SliceCosts();

	std::vector<double> featureWeights = _featureWeights->getWeights();

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		const std::vector<double>& features = _features->getFeatures(slice->getId());

		double value = 0;
		for (unsigned int i = 0; i < featureWeights.size(); i++)
			value += featureWeights[i]*features[i];

		_costs->setCosts(slice->getId(), value);
	}
}
