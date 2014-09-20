#include <vigra/impex.hxx>
#include "SolutionWriter.h"

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

			segmentation(p.x, p.y) = 1.0;
		}
	}

	vigra::exportImage(vigra::srcImageRange(segmentation), vigra::ImageExportInfo(_filename.c_str()));
}
