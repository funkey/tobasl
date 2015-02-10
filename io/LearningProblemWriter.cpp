#include <fstream>
#include <inference/SliceVariableMap.h>
#include "LearningProblemWriter.h"
#include <util/Logger.h>

logger::LogChannel learningproblemwriterlog("learningproblemwriterlog", "[LearningProblemWriter] ");

LearningProblemWriter::LearningProblemWriter(std::string directory) :
	_directory(directory) {

	registerInput(_slices, "slices");
	registerInput(_conflictSets, "conflict sets");
	registerInput(_features, "features");
	registerInput(_bestEffort, "best effort");
	registerInput(_lossFunction, "loss function");
}

void
LearningProblemWriter::write() {

	updateInputs();

	LOG_USER(learningproblemwriterlog) << "writing learning problem" << std::endl;

	// prepare the output directory
	boost::filesystem::path directory(_directory);

	if (!boost::filesystem::exists(directory)) {

		boost::filesystem::create_directory(directory);

	} else if (!boost::filesystem::is_directory(directory)) {

		UTIL_THROW_EXCEPTION(
				IOError,
				"\"" << _directory << "\" is not a directory");
	}

	std::ofstream featuresFile((_directory + "/features.txt").c_str());

	SliceVariableMap sliceVariableMap;
	unsigned int nextVarNum = 0;

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		sliceVariableMap.associate(slice->getId(), nextVarNum);
		nextVarNum++;

		const std::vector<double>& features = _features->getFeatures(slice->getId());

		foreach (double f, features) {

			featuresFile << f << " ";
		}

		featuresFile << std::endl;
	}

	std::ofstream featuresMinMaxFile((_directory + "/features_minmax.txt").c_str());

	const std::vector<double>& min = _features->getMin();
	const std::vector<double>& max = _features->getMax();

	foreach (double f, min)
		featuresMinMaxFile << f << " ";
	featuresMinMaxFile << std::endl;
	foreach (double f, max)
		featuresMinMaxFile << f << " ";
	featuresMinMaxFile << std::endl;

	std::ofstream constraintsFile((_directory + "/constraints.txt").c_str());

	foreach (ConflictSet& conflictSet, *_conflictSets) {

		foreach (unsigned int sliceId, conflictSet.getSlices()) {

			unsigned int varNum = sliceVariableMap.getVariableNum(sliceId);

			constraintsFile << "1*" << varNum << " ";
		}

		constraintsFile << "<= 1" << std::endl;
	}

	std::ofstream labelsFile((_directory + "/labels.txt").c_str());

	std::set<unsigned int> bestEffortSliceIds;
	foreach (boost::shared_ptr<Slice> slice, *_bestEffort)
		bestEffortSliceIds.insert(slice->getId());

	for (unsigned int varNum = 0; varNum < nextVarNum; varNum++) {

		unsigned int sliceId = sliceVariableMap.getSliceId(varNum);

		if (bestEffortSliceIds.count(sliceId))
			labelsFile << 1 << std::endl;
		else
			labelsFile << 0 << std::endl;
	}

	std::ofstream costFunctionFile((_directory + "/cost_function.txt").c_str());

	costFunctionFile << "numVar = " << nextVarNum << std::endl;

	foreach (boost::shared_ptr<Slice> slice, *_slices) {

		costFunctionFile
				<< "var" << sliceVariableMap.getVariableNum(slice->getId()) << " "
				<< (*_lossFunction)[slice->getId()] << std::endl;
	}
}
