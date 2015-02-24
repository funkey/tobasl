/**
 * multi2cut main file. Initializes all objects.
 */

#include <iostream>
#include <fstream>
#include <pipeline/Process.h>
#include <pipeline/Value.h>
#include <util/ProgramOptions.h>
#include <util/Logger.h>
#include <util/exceptions.h>
#include <util/helpers.hpp>

#include <features/FeatureExtractor.h>
#include <io/ReadMergeTreePipeline.h>
#include <io/FeatureWeightsReader.h>
#include <io/SolutionWriter.h>
#include <io/SlicesWriter.h>
#include <io/LearningProblemWriter.h>
#include <slices/SlicesCollector.h>
#include <inference/LinearSliceCostFunction.h>
#include <inference/ProblemAssembler.h>
#include <inference/LinearSolver.h>
#include <inference/Reconstructor.h>
#include <loss/TopologicalLoss.h>
#include <loss/HammingLoss.h>
#include <loss/SliceDistanceLoss.h>
#include <loss/ContourDistanceLoss.h>
#include <loss/OverlapLoss.h>
#include <loss/LossCollector.h>

using namespace logger;

util::ProgramOption optionMergeTreeImage(
		util::_long_name        = "mergeTreeImage",
		util::_short_name       = "i",
		util::_description_text = "A single image file representing the merge tree or a directory of images for merging of multiple merge trees.",
		util::_default_value    = "mergetree.png");

util::ProgramOption optionRawImage(
		util::_long_name        = "rawImage",
		util::_short_name       = "r",
		util::_description_text = "The raw image for feature extraction.",
		util::_default_value    = "raw.png");

util::ProgramOption optionProbabilityImage(
		util::_long_name        = "probabilityImage",
		util::_short_name       = "p",
		util::_description_text = "The membrane probability image for feature extraction.",
		util::_default_value    = "probability.png");

util::ProgramOption optionGroundTruth(
		util::_long_name        = "groundTruth",
		util::_short_name       = "g",
		util::_description_text = "An image representing the ground truth segmentation.");

util::ProgramOption optionWriteLearningProblem(
		util::_long_name        = "writeLearningProblem",
		util::_short_name       = "l",
		util::_description_text = "Instead of performing inference, write a learning problem for sbmrm.");

util::ProgramOption optionSliceLoss(
		util::_long_name        = "sliceLoss",
		util::_description_text = "The candidate loss function to use as Δ for learning. Valid values are: "
		                          "'topological' (default), 'hamming', 'contourdistance', 'overlap', and 'slicedistance'.",
		util::_default_value    = "topological");

util::ProgramOption optionDumpSlices(
		util::_long_name        = "dumpSlices",
		util::_description_text = "Store images and offset positions of all extracted candidates (slices).");

boost::shared_ptr<pipeline::SimpleProcessNode<> >
getLoss(
		std::string name,
		bool multiMergeTrees,
		boost::shared_ptr<pipeline::ProcessNode> mergeTreeReader,
		boost::shared_ptr<pipeline::ProcessNode> gtSliceExtractor,
		boost::shared_ptr<pipeline::ProcessNode> bestEffortReconstructor = boost::shared_ptr<pipeline::ProcessNode>()) {

	boost::shared_ptr<pipeline::SimpleProcessNode<> > loss;

	if (name == "topological") {

		if (multiMergeTrees)
			UTIL_THROW_EXCEPTION(
					UsageError,
					"The topological loss can only be used with single merge trees.");

		if (!bestEffortReconstructor)
			UTIL_THROW_EXCEPTION(
					UsageError,
					"The topological loss needs a best-effort solution (and can therefore not be used to find one).");

		LOG_USER(out) << "[main] using slice loss TopologicalLoss" << std::endl;
		loss = boost::make_shared<TopologicalLoss>();

		// only for SliceTrees!
		loss->addInput("slices", mergeTreeReader->getOutput("slices"));
		loss->addInput("best effort", bestEffortReconstructor->getOutput("reconstruction"));

	} else if (name == "hamming") {

		if (!bestEffortReconstructor)
			UTIL_THROW_EXCEPTION(
					UsageError,
					"The hamming loss needs a best-effort solution (and can therefore not be used to find one).");

		LOG_USER(out) << "[main] using slice loss HammingLoss" << std::endl;
		loss = boost::make_shared<HammingLoss>();

		loss->setInput("slices", mergeTreeReader->getOutput("slices"));
		loss->setInput("best effort", bestEffortReconstructor->getOutput("reconstruction"));

	} else if (name == "contourdistance") {

		LOG_USER(out) << "[main] using slice loss ContourDistanceLoss" << std::endl;
		loss = boost::make_shared<ContourDistanceLoss>();

		loss->setInput("slices", mergeTreeReader->getOutput("slices"));
		loss->setInput("ground truth", gtSliceExtractor->getOutput("slices"));

	} else if (name == "overlap") {

		LOG_USER(out) << "[main] using slice loss OverlapLoss" << std::endl;
		loss = boost::make_shared<OverlapLoss>();

		loss->setInput("slices", mergeTreeReader->getOutput("slices"));
		loss->setInput("ground truth", gtSliceExtractor->getOutput("slices"));

	} else if (name == "slicedistance") {

		LOG_USER(out) << "[main] using slice loss SliceDistanceLoss" << std::endl;
		loss = boost::make_shared<SliceDistanceLoss>();

		loss->setInput("slices", mergeTreeReader->getOutput("slices"));
		loss->setInput("ground truth", gtSliceExtractor->getOutput("slices"));

	} else if (name == "slicedistance+hamming") {

		LOG_USER(out) << "[main] using slice loss SliceDistanceLoss" << std::endl;
		loss = boost::make_shared<HammingLoss>();

		pipeline::Process<SliceDistanceLoss> sliceDistanceLoss;

		sliceDistanceLoss->setInput("slices", mergeTreeReader->getOutput("slices"));
		sliceDistanceLoss->setInput("ground truth", gtSliceExtractor->getOutput("slices"));

		// combine the slice distance loss with Hamming (otherwise, 
		// selecting nothing minimizes the loss)
		loss->setInput("slices", mergeTreeReader->getOutput("slices"));
		loss->setInput("best effort", bestEffortReconstructor->getOutput("reconstruction"));
		loss->setInput("base loss function", sliceDistanceLoss->getOutput());

	} else {

		UTIL_THROW_EXCEPTION(
				UsageError,
				"unknown slice loss '" << name << "'");
	}

	return loss;
}

int main(int optionc, char** optionv) {

	try {

		/********
		 * INIT *
		 ********/

		// init command line parser
		util::ProgramOptions::init(optionc, optionv);

		// init logger
		LogManager::init();

		LOG_USER(out) << "[main] starting..." << std::endl;

		boost::filesystem::path mergeTree(optionMergeTreeImage.as<std::string>());
		std::vector<boost::filesystem::path> mergeTreeFiles;

		if (boost::filesystem::is_directory(mergeTree)) {

			LOG_USER(out) << "[main] reading multiple merge tree images" << std::endl;

			std::copy(
					boost::filesystem::directory_iterator(mergeTree),
					boost::filesystem::directory_iterator(),
					back_inserter(mergeTreeFiles));
			std::sort(mergeTreeFiles.begin(), mergeTreeFiles.end());

		} else {

			LOG_USER(out) << "[main] reading a single merge tree image" << std::endl;

			mergeTreeFiles.push_back(mergeTree);;
		}

		std::vector<pipeline::Process<ReadMergeTreePipeline> > mergeTreeReaders;

		// create a ground truth slice extractor, if ground truth was given
		pipeline::Process<SliceExtractor<unsigned char> > gtSliceExtractor(0, false /* downsample */, true /* brightToDark */);
		if (optionGroundTruth) {

			pipeline::Process<ImageReader> groundTruthReader(optionGroundTruth.as<std::string>());
			gtSliceExtractor->setInput("membrane", groundTruthReader->getOutput());
		}

		// create a collector for all merge tree slices
		boost::shared_ptr<pipeline::ProcessNode> slicesCollector;

		if (mergeTreeFiles.size() > 1)
			slicesCollector = boost::make_shared<SlicesCollector>();

		// for each merge tree file create a read pipeline
		foreach (boost::filesystem::path mergeTreeFile, mergeTreeFiles) {

			if (boost::filesystem::is_directory(mergeTreeFile))
				continue;

			pipeline::Process<ReadMergeTreePipeline> mergeTreeReader(
					mergeTreeFile.string(),
					optionGroundTruth);

			if (optionGroundTruth)
				mergeTreeReader->setInput("ground truth slices", gtSliceExtractor->getOutput());

			mergeTreeReaders.push_back(mergeTreeReader);

			if (mergeTreeFiles.size() > 1) {

				slicesCollector->addInput("slices", mergeTreeReader->getOutput("slices"));
				slicesCollector->addInput("conflict sets", mergeTreeReader->getOutput("conflict sets"));

			} else {

				slicesCollector = mergeTreeReader.getOperator();
			}
		}

		pipeline::Process<ImageReader>      rawImageReader(optionRawImage.as<std::string>());
		pipeline::Process<ImageReader>      probabilityImageReader(optionProbabilityImage.as<std::string>());
		pipeline::Process<FeatureExtractor> featureExtractor;

		pipeline::Value<Image> image = rawImageReader->getOutput();
		unsigned int width  = image->width();
		unsigned int height = image->height();

		featureExtractor->setInput("slices", slicesCollector->getOutput("slices"));
		featureExtractor->setInput("raw image", rawImageReader->getOutput());
		featureExtractor->setInput("probability image", probabilityImageReader->getOutput());

		if (optionWriteLearningProblem) {

			/*********************
			 * LEARNING PIPELINE *
			 *********************/

			// create a single best-effort solution (needed for structured 
			// learning)

			// the loss will be the combined loss from all merge tree readers
			pipeline::Process<LossCollector> bestEffortLossFunction;
			foreach (pipeline::Process<ReadMergeTreePipeline> mergeTreeReader, mergeTreeReaders)
				bestEffortLossFunction->addInput(mergeTreeReader->getOutput("best effort loss function"));

			pipeline::Process<ProblemAssembler> bestEffortProblem;
			bestEffortProblem->setInput("slices", slicesCollector->getOutput("slices"));
			bestEffortProblem->setInput("conflict sets", slicesCollector->getOutput("conflict sets"));
			bestEffortProblem->setInput("slice loss", bestEffortLossFunction->getOutput());

			pipeline::Value<LinearSolverParameters> linearSolverParameters;
			linearSolverParameters->setVariableType(Binary);
			pipeline::Process<LinearSolver> bestEffortSolver;
			bestEffortSolver->setInput("objective", bestEffortProblem->getOutput("objective"));
			bestEffortSolver->setInput("linear constraints", bestEffortProblem->getOutput("linear constraints"));
			bestEffortSolver->setInput("parameters", linearSolverParameters);

			pipeline::Process<Reconstructor> bestEffortReconstructor;
			bestEffortReconstructor->setInput("slices", slicesCollector->getOutput());
			bestEffortReconstructor->setInput("slice variable map", bestEffortProblem->getOutput("slice variable map"));
			bestEffortReconstructor->setInput("solution", bestEffortSolver->getOutput("solution"));

			// create a learning loss function
			boost::shared_ptr<pipeline::ProcessNode> loss;

			LOG_USER(out) << "creating slice loss..." << std::endl;
			if (optionSliceLoss.as<std::string>() == "topological" && mergeTreeReaders.size() > 1) {

				LOG_USER(out) << "using loss TopologicalLoss, once for each merge tree" << std::endl;

				loss = boost::make_shared<TopologicalLoss>();
				foreach (pipeline::Process<ReadMergeTreePipeline> mergeTreeReader, mergeTreeReaders) {

					loss->addInput("slices", mergeTreeReader->getOutput("slices"));
					loss->addInput("best effort", mergeTreeReader->getOutput("best effort slices"));
				}

			} else {

				loss =
					getLoss(
							optionSliceLoss,
							(mergeTreeReaders.size() > 1),
							slicesCollector,
							gtSliceExtractor.getOperator(),
							bestEffortReconstructor.getOperator());
			}

			pipeline::Process<LearningProblemWriter> writer("learning_problem");

			writer->setInput("slices", slicesCollector->getOutput("slices"));
			writer->setInput("conflict sets", slicesCollector->getOutput("conflict sets"));
			writer->setInput("features", featureExtractor->getOutput());
			writer->setInput("best effort", bestEffortReconstructor->getOutput());
			writer->setInput("loss function", loss->getOutput());

			// write the learning problem
			writer->write();

			// prepare the output image directory
			boost::filesystem::path directory("output_images");
			if (!boost::filesystem::exists(directory))
				boost::filesystem::create_directory(directory);
			 else if (!boost::filesystem::is_directory(directory))
				UTIL_THROW_EXCEPTION(
						IOError,
						"\"" << directory << "\" is not a directory");
			directory = directory / "slices";
			if (!boost::filesystem::exists(directory))
				boost::filesystem::create_directory(directory);
			 else if (!boost::filesystem::is_directory(directory))
				UTIL_THROW_EXCEPTION(
						IOError,
						"\"" << directory << "\" is not a directory");

			// save the indvidual best effort solutions
			unsigned int i = 0;
			foreach (pipeline::Process<ReadMergeTreePipeline> mergeTreeReader, mergeTreeReaders) {

				std::string filename = std::string("output_images/best-effort_") + boost::lexical_cast<std::string>(i) + ".tif";
				pipeline::Process<SolutionWriter> solutionWriter(width, height, filename);
				solutionWriter->setInput("solution", mergeTreeReader->getOutput("best effort slices"));
				solutionWriter->write();
				i++;
			}

			// save the overall best-effort solution
			std::string filename = "output_images/best-effort.tif";
			pipeline::Process<SolutionWriter> solutionWriter(width, height, filename);
			solutionWriter->setInput("solution", bestEffortReconstructor->getOutput());
			solutionWriter->write();

			if (optionDumpSlices) {

				// store slices and their offsets
				pipeline::Process<SlicesWriter> slicesWriter("output_images/slices");
				slicesWriter->setInput(slicesCollector->getOutput("slices"));
				slicesWriter->write();
			}

		} else {

			/**********************
			 * INFERENCE PIPELINE *
			 **********************/

			pipeline::Process<FeatureWeightsReader>    featureWeightsReader;
			pipeline::Process<LinearSliceCostFunction> sliceCostFunction;
			pipeline::Process<ProblemAssembler>        problemAssembler;
			pipeline::Process<LinearSolver>            linearSolver;
			pipeline::Process<Reconstructor>           reconstructor;
			pipeline::Process<SolutionWriter>          solutionWriter(width, height, "output_images/solution.tif");

			sliceCostFunction->setInput("slices", slicesCollector->getOutput("slices"));
			sliceCostFunction->setInput("features", featureExtractor->getOutput());
			sliceCostFunction->setInput("feature weights", featureWeightsReader->getOutput());

			problemAssembler->setInput("slices", slicesCollector->getOutput("slices"));
			problemAssembler->setInput("conflict sets", slicesCollector->getOutput("conflict sets"));
			problemAssembler->setInput("slice costs", sliceCostFunction->getOutput());

			pipeline::Value<LinearSolverParameters> linearSolverParameters;
			linearSolverParameters->setVariableType(Binary);
			linearSolver->setInput("objective", problemAssembler->getOutput("objective"));
			linearSolver->setInput("linear constraints", problemAssembler->getOutput("linear constraints"));
			linearSolver->setInput("parameters", linearSolverParameters);

			reconstructor->setInput("slices", slicesCollector->getOutput());
			reconstructor->setInput("slice variable map", problemAssembler->getOutput("slice variable map"));
			reconstructor->setInput("solution", linearSolver->getOutput("solution"));

			// prepare the output image directory
			boost::filesystem::path directory("output_images");
			if (!boost::filesystem::exists(directory))
				boost::filesystem::create_directory(directory);
			 else if (!boost::filesystem::is_directory(directory))
				UTIL_THROW_EXCEPTION(
						IOError,
						"\"" << directory << "\" is not a directory");

			solutionWriter->setInput("solution", reconstructor->getOutput());
			solutionWriter->write();
		}

	} catch (Exception& e) {

		handleException(e, std::cerr);
	}
}

