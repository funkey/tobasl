#include <vigra/impex.hxx>
#include <util/Logger.h>
#include "SolutionWriter.h"

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

	vigra::exportImage(vigra::srcImageRange(segmentation), vigra::ImageExportInfo(_filename.c_str()));
}
