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
#include <io/MergeTreeReader.h>
#include <io/MultiMergeTreeReader.h>
#include <io/FeatureWeightsReader.h>
#include <io/SolutionWriter.h>
#include <io/SlicesWriter.h>
#include <io/LearningProblemWriter.h>
#include <inference/LinearSliceCostFunction.h>
#include <inference/OverlapSliceCostFunction.h>
#include <inference/ProblemAssembler.h>
#include <inference/LinearSolver.h>
#include <inference/Reconstructor.h>
#include <loss/TopologicalLoss.h>
#include <loss/HammingLoss.h>
#include <loss/SliceDistanceLoss.h>

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
		util::_description_text = "An image representing the ground truth segmentation.",
		util::_default_value    = "groundtruth.png");

util::ProgramOption optionWriteLearningProblem(
		util::_long_name        = "writeLearningProblem",
		util::_short_name       = "l",
		util::_description_text = "Instead of performing inference, write a learning problem for sbmrm.");

util::ProgramOption optionSliceLoss(
		util::_long_name        = "sliceLoss",
		util::_description_text = "The candidate loss function to use for learning. Valid values are: 'topological', 'hamming', and 'distance'.",
		util::_default_value    = "topological");

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
		bool multiMergeTrees = boost::filesystem::is_directory(mergeTree);

		if (multiMergeTrees) {

			LOG_USER(out) << "[main] reading multiple merge tree images" << std::endl;

		} else {

			LOG_USER(out) << "[main] reading a single merge tree image" << std::endl;
		}

		boost::shared_ptr<pipeline::SimpleProcessNode<> > mergeTreeReader;

		if (multiMergeTrees)
			mergeTreeReader = boost::make_shared<MultiMergeTreeReader>(optionMergeTreeImage.as<std::string>());
		else
			mergeTreeReader = boost::make_shared<MergeTreeReader>(optionMergeTreeImage.as<std::string>());

		pipeline::Process<ImageReader>      rawImageReader(optionRawImage.as<std::string>());
		pipeline::Process<ImageReader>      probabilityImageReader(optionProbabilityImage.as<std::string>());
		pipeline::Process<FeatureExtractor> featureExtractor;

		pipeline::Value<Image> image = rawImageReader->getOutput();
		unsigned int width  = image->width();
		unsigned int height = image->height();

		featureExtractor->setInput("slices", mergeTreeReader->getOutput("slices"));
		featureExtractor->setInput("raw image", rawImageReader->getOutput());
		featureExtractor->setInput("probability image", probabilityImageReader->getOutput());

		if (optionWriteLearningProblem) {

			/*********************
			 * LEARNING PIPELINE *
			 *********************/

			// get the best-effort solution

			pipeline::Process<ImageReader>                    groundTruthReader(optionGroundTruth.as<std::string>());
			pipeline::Process<SliceExtractor<unsigned char> > gtSliceExtractor(0, false);
			pipeline::Process<OverlapSliceCostFunction>       overlapSliceCostFunction;
			pipeline::Process<ProblemAssembler>               bestEffortProblem;
			pipeline::Process<LinearSolver>                   bestEffortSolver;
			pipeline::Process<Reconstructor>                  bestEffortReconstructor;
			pipeline::Process<SolutionWriter>                 solutionWriter(width, height, "output_images/best-effort.tif");

			gtSliceExtractor->setInput("membrane", groundTruthReader->getOutput());
			overlapSliceCostFunction->setInput("ground truth", gtSliceExtractor->getOutput("slices"));
			overlapSliceCostFunction->setInput("slices", mergeTreeReader->getOutput("slices"));
			bestEffortProblem->setInput("slices", mergeTreeReader->getOutput("slices"));
			bestEffortProblem->setInput("conflict sets", mergeTreeReader->getOutput("conflict sets"));
			bestEffortProblem->setInput("slice costs", overlapSliceCostFunction->getOutput());

			pipeline::Value<LinearSolverParameters> linearSolverParameters;
			linearSolverParameters->setVariableType(Binary);
			bestEffortSolver->setInput("objective", bestEffortProblem->getOutput("objective"));
			bestEffortSolver->setInput("linear constraints", bestEffortProblem->getOutput("linear constraints"));
			bestEffortSolver->setInput("parameters", linearSolverParameters);

			bestEffortReconstructor->setInput("slices", mergeTreeReader->getOutput());
			bestEffortReconstructor->setInput("slice variable map", bestEffortProblem->getOutput("slice variable map"));
			bestEffortReconstructor->setInput("solution", bestEffortSolver->getOutput("solution"));

			boost::shared_ptr<pipeline::SimpleProcessNode<> > loss;

			if (optionSliceLoss.as<std::string>() == "topological") {

				if (multiMergeTrees)
					UTIL_THROW_EXCEPTION(
							UsageError,
							"The topological loss can only be used with single merge trees.");

				LOG_USER(out) << "[main] using slice loss TopologicalLoss" << std::endl;
				loss = boost::make_shared<TopologicalLoss>();

				// only for SliceTrees!
				loss->setInput("slices", mergeTreeReader->getOutput("slices"));
				loss->setInput("best effort", bestEffortReconstructor->getOutput());

			} else if (optionSliceLoss.as<std::string>() == "hamming") {

				LOG_USER(out) << "[main] using slice loss HammingLoss" << std::endl;
				loss = boost::make_shared<HammingLoss>();

				loss->setInput("slices", mergeTreeReader->getOutput("slices"));
				loss->setInput("best effort", bestEffortReconstructor->getOutput());

			} else if (optionSliceLoss.as<std::string>() == "distance") {

				LOG_USER(out) << "[main] using slice loss SliceDistanceLoss" << std::endl;
				loss = boost::make_shared<SliceDistanceLoss>();

				loss->setInput("slices", mergeTreeReader->getOutput("slices"));
				loss->setInput("ground truth", gtSliceExtractor->getOutput("slices"));

			} else if (optionSliceLoss.as<std::string>() == "distance+hamming") {

				LOG_USER(out) << "[main] using slice loss SliceDistanceLoss" << std::endl;
				loss = boost::make_shared<HammingLoss>();

				pipeline::Process<SliceDistanceLoss> sliceDistanceLoss;

				sliceDistanceLoss->setInput("slices", mergeTreeReader->getOutput("slices"));
				sliceDistanceLoss->setInput("ground truth", gtSliceExtractor->getOutput("slices"));

				// combine the slice distance loss with Hamming (otherwise, 
				// selecting nothing minimizes the loss)
				loss->setInput("slices", mergeTreeReader->getOutput("slices"));
				loss->setInput("best effort", bestEffortReconstructor->getOutput());
				loss->setInput("base loss function", sliceDistanceLoss->getOutput());

			} else {

				UTIL_THROW_EXCEPTION(
						UsageError,
						"unknown slice loss '" << optionSliceLoss.as<std::string>() << "'");
			}

			pipeline::Process<LearningProblemWriter> writer("learning_problem");

			writer->setInput("slices", mergeTreeReader->getOutput("slices"));
			writer->setInput("conflict sets", mergeTreeReader->getOutput("conflict sets"));
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

			// show the best effort solution
			solutionWriter->setInput("solution", bestEffortReconstructor->getOutput());
			solutionWriter->write();


			// store slices and their offsets
			pipeline::Process<SlicesWriter> slicesWriter("output_images/slices");
			slicesWriter->setInput(mergeTreeReader->getOutput("slices"));
			slicesWriter->write();

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

			sliceCostFunction->setInput("slices", mergeTreeReader->getOutput("slices"));
			sliceCostFunction->setInput("features", featureExtractor->getOutput());
			sliceCostFunction->setInput("feature weights", featureWeightsReader->getOutput());

			problemAssembler->setInput("slices", mergeTreeReader->getOutput("slices"));
			problemAssembler->setInput("conflict sets", mergeTreeReader->getOutput("conflict sets"));
			problemAssembler->setInput("slice costs", sliceCostFunction->getOutput());

			pipeline::Value<LinearSolverParameters> linearSolverParameters;
			linearSolverParameters->setVariableType(Binary);
			linearSolver->setInput("objective", problemAssembler->getOutput("objective"));
			linearSolver->setInput("linear constraints", problemAssembler->getOutput("linear constraints"));
			linearSolver->setInput("parameters", linearSolverParameters);

			reconstructor->setInput("slices", mergeTreeReader->getOutput());
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

