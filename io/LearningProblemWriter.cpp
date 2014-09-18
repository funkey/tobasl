#include <fstream>
#include "LearningProblemWriter.h"
#include <util/Logger.h>

logger::LogChannel learningproblemwriterlog("learningproblemwriterlog", "[LearningProblemWriter] ");

LearningProblemWriter::LearningProblemWriter() {

	registerInput(_slices, "slices");
	registerInput(_conflictSets, "conflict sets");
	registerInput(_features, "features");
}

void
LearningProblemWriter::write(const std::string& filename) {

	updateInputs();

	LOG_USER(learningproblemwriterlog) << "writing learning problem to " << filename << std::endl;

	std::ofstream featuresFile("features.txt");

	std::map<unsigned int, unsigned int> variableNums;
	unsigned int nextVarNum = 0;

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		variableNums[slice->getId()] = nextVarNum;
		nextVarNum++;

		std::vector<double> features = _features->getFeatures(slice->getId());

		foreach (double f, features) {

			featuresFile << f << " ";
		}

		featuresFile << std::endl;
	}

	std::ofstream constraintsFile("constraints.txt");

	foreach (ConflictSet& conflictSet, *_conflictSets) {

		foreach (unsigned int sliceId, conflictSet.getSlices()) {

			unsigned int varNum = variableNums[sliceId];

			constraintsFile << "1*" << varNum << " ";
		}

		constraintsFile << "<= 1" << std::endl;
	}
}
