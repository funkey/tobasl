#include <fstream>
#include "FeatureWeightsReader.h"

FeatureWeightsReader::FeatureWeightsReader() {

	registerOutput(_weights, "feature weights");
}

void
FeatureWeightsReader::updateOutputs() {

	if (!_weights)
		_weights = new FeatureWeights();

	std::ifstream weightsFile("feature_weights.txt");

	std::vector<double> weights;

	double w;
	while (weightsFile >> w)
		weights.push_back(w);

	_weights->setWeights(weights);
}
