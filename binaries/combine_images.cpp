#include <iostream>
#include <vigra/impex.hxx>
#include <vigra/multi_array.hxx>

int main(int argc, char** argv) {

	std::vector<vigra::MultiArray<2, float> > images;

	unsigned int width  = 0;
	unsigned int height = 0;
	for (int arg = 1; arg < argc - 1; arg++) {

		// get information about the image to read
		vigra::ImageImportInfo info(argv[arg]);

		// create new image
		images.push_back(vigra::MultiArray<2, float>(vigra::Shape2(info.width(), info.height())));

		// read image
		importImage(info, vigra::destImage(images.back()));

		width += info.width();
		height = info.height();
	}

	vigra::MultiArray<2, float> combined(vigra::Shape2(width, height));

	unsigned int offset = 0;
	for (unsigned int i = 0; i < images.size(); i++) {

		combined.subarray(
				vigra::Shape2(offset, 0),
				vigra::Shape2(offset + images[i].width(), height)) = images[i];

		offset += images[i].width();
	}

	vigra::exportImage(vigra::srcImageRange(combined), vigra::ImageExportInfo(argv[argc-1]));
}
