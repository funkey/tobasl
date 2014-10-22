/**
 * multi2cut main file. Initializes all objects.
 */

#include <iostream>
#include <fstream>
#include <pipeline/Process.h>
#include <pipeline/Value.h>
#include <util/ProgramOptions.h>
#include <util/Logger.h>
#include <util/helpers.hpp>

#include <imageprocessing/io/ImageReader.h>
#include <slices/SliceExtractor.h>
#include <features/FeatureExtractor.h>
#include <io/LearningProblemWriter.h>
#include <io/FeatureWeightsReader.h>
#include <io/SolutionWriter.h>
#include <inference/LinearSliceCostFunction.h>
#include <inference/OverlapSliceCostFunction.h>
#include <inference/ProblemAssembler.h>
#include <inference/LinearSolver.h>
#include <inference/Reconstructor.h>

// debug
#include <vigra/impex.hxx>

using namespace logger;

util::ProgramOption optionMergeTreeImage(
		util::_long_name        = "mergeTreeImage",
		util::_short_name       = "i",
		util::_description_text = "An image representing the merge tree.",
		util::_default_value    = "mergetree.png");

util::ProgramOption optionRawImage(
		util::_long_name        = "rawImage",
		util::_short_name       = "r",
		util::_description_text = "The raw image for feature extraction.",
		util::_default_value    = "raw.png");

util::ProgramOption optionGroundTruth(
		util::_long_name        = "groundTruth",
		util::_short_name       = "g",
		util::_description_text = "An image representing the ground truth segmentation.",
		util::_default_value    = "groundtruth.png");

util::ProgramOption optionWriteLearningProblem(
		util::_long_name        = "writeLearningProblem",
		util::_short_name       = "l",
		util::_description_text = "Instead of performing inference, write a learning problem for sbmrm.");

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

		pipeline::Process<ImageReader>                    imageReader(optionMergeTreeImage.as<std::string>());
		pipeline::Process<ImageReader>                    rawImageReader(optionRawImage.as<std::string>());
		pipeline::Process<SliceExtractor<unsigned char> > sliceExtractor(0, true /* downsample */);
		pipeline::Process<FeatureExtractor>               featureExtractor;

		pipeline::Value<Image> image = imageReader->getOutput();
		unsigned int width  = image->width();
		unsigned int height = image->height();

		sliceExtractor->setInput("membrane", imageReader->getOutput());

		featureExtractor->setInput("slices", sliceExtractor->getOutput("slices"));
		featureExtractor->setInput("raw image", rawImageReader->getOutput());

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
			pipeline::Process<SolutionWriter>                 solutionWriter(width, height, "best-effort.png");

			gtSliceExtractor->setInput("membrane", groundTruthReader->getOutput());
			overlapSliceCostFunction->setInput("ground truth", gtSliceExtractor->getOutput("slices"));
			overlapSliceCostFunction->setInput("slices", sliceExtractor->getOutput("slices"));
			bestEffortProblem->setInput("slices", sliceExtractor->getOutput("slices"));
			bestEffortProblem->setInput("conflict sets", sliceExtractor->getOutput("conflict sets"));
			bestEffortProblem->setInput("slice costs", overlapSliceCostFunction->getOutput());

			pipeline::Value<LinearSolverParameters> linearSolverParameters;
			linearSolverParameters->setVariableType(Binary);
			bestEffortSolver->setInput("objective", bestEffortProblem->getOutput("objective"));
			bestEffortSolver->setInput("linear constraints", bestEffortProblem->getOutput("linear constraints"));
			bestEffortSolver->setInput("parameters", linearSolverParameters);

			bestEffortReconstructor->setInput("slices", sliceExtractor->getOutput());
			bestEffortReconstructor->setInput("slice variable map", bestEffortProblem->getOutput("slice variable map"));
			bestEffortReconstructor->setInput("solution", bestEffortSolver->getOutput("solution"));

			pipeline::Process<LearningProblemWriter> writer;

			writer->setInput("slices", sliceExtractor->getOutput("slices"));
			writer->setInput("conflict sets", sliceExtractor->getOutput("conflict sets"));
			writer->setInput("features", featureExtractor->getOutput());
			writer->setInput("best effort", bestEffortReconstructor->getOutput());

			writer->write();

			// show the best effort solution
			solutionWriter->setInput("solution", bestEffortReconstructor->getOutput());
			solutionWriter->write();

			// debug output

			pipeline::Value<Slices> slices = sliceExtractor->getOutput("slices");

			foreach (boost::shared_ptr<Slice> slice, *slices) {

				std::string imageFilename = "slices/slice_" + boost::lexical_cast<std::string>(slice->getId()) + ".png";

				const ConnectedComponent::bitmap_type& bitmap = slice->getComponent()->getBitmap();

				// store the image
				vigra::exportImage(
						vigra::srcImageRange(bitmap),
						vigra::ImageExportInfo(imageFilename.c_str()));
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
			pipeline::Process<SolutionWriter>          solutionWriter(width, height);

			sliceCostFunction->setInput("slices", sliceExtractor->getOutput("slices"));
			sliceCostFunction->setInput("features", featureExtractor->getOutput());
			sliceCostFunction->setInput("feature weights", featureWeightsReader->getOutput());

			problemAssembler->setInput("slices", sliceExtractor->getOutput("slices"));
			problemAssembler->setInput("conflict sets", sliceExtractor->getOutput("conflict sets"));
			problemAssembler->setInput("slice costs", sliceCostFunction->getOutput());

			pipeline::Value<LinearSolverParameters> linearSolverParameters;
			linearSolverParameters->setVariableType(Binary);
			linearSolver->setInput("objective", problemAssembler->getOutput("objective"));
			linearSolver->setInput("linear constraints", problemAssembler->getOutput("linear constraints"));
			linearSolver->setInput("parameters", linearSolverParameters);

			reconstructor->setInput("slices", sliceExtractor->getOutput());
			reconstructor->setInput("slice variable map", problemAssembler->getOutput("slice variable map"));
			reconstructor->setInput("solution", linearSolver->getOutput("solution"));

			solutionWriter->setInput("solution", reconstructor->getOutput());
			solutionWriter->write();
		}

	} catch (Exception& e) {

		handleException(e, std::cerr);
	}
}

