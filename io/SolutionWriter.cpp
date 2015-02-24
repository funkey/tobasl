#include <vigra/impex.hxx>
#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include "SolutionWriter.h"

util::ProgramOption optionSolutionWithBorders(
		util::_long_name        = "solutionWithBorders",
		util::_description_text = "Ensure an at least 1-pixel wide background border between each pair of found regions in the solution image.");

logger::LogChannel solutionwriterlog("solutionwriterlog", "[SolutionWriter] ");

SolutionWriter::SolutionWriter(unsigned int width, unsigned int height, const std::string& filename) :
	_width(width),
	_height(height),
	_filename(filename) {

	registerInput(_solution, "solution");
}

void
SolutionWriter::write() {

	updateInputs();

	Image segmentation(_width, _height);

	foreach (boost::shared_ptr<Slice> slice, *_solution) {

		foreach (const util::point<unsigned int>& p, slice->getComponent()->getPixels()) {

			if (segmentation(p.x, p.y) != 0)
				LOG_ERROR(solutionwriterlog)
						<< "inconsistency in solution: pixel " << p
						<< " is part of at least two slices: "
						<< segmentation(p.x, p.y) << " and "
						<< slice->getId() << std::endl;

			segmentation(p.x, p.y) = slice->getId();
		}
	}

	if (optionSolutionWithBorders) {

		for (unsigned int y = 0; y < segmentation.height() - 1; y++)
			for (unsigned int x = 0; x < segmentation.width() - 1; x++) {

				float value = segmentation(x, y);
				float right = segmentation(x+1, y);
				float down  = segmentation(x, y+1);
				float diag  = segmentation(x+1, y+1);

				if (value != right || value != down || value != diag)
					segmentation(x, y) = 0;
			}
	}

	vigra::exportImage(vigra::srcImageRange(segmentation), vigra::ImageExportInfo(_filename.c_str()));
}
