#include <iostream>
#include <vigra/impex.hxx>
#include <vigra/multi_array.hxx>

void printUsage() {

	std::cout << std::endl;
	std::cout << "combine_images [-s|-s0] [-l] <image_1> ... <image_n> <out>" << std::endl;
	std::cout << std::endl;
	std::cout << "  -s  Put a seperating line between each pair of images." << std::endl;
	std::cout << "      The intensity of the line will be the maximal intensity " << std::endl;
	std::cout << "      found in any input image." << std::endl;
	std::cout << "  -s0 Put a seperating line between each pair of images." << std::endl;
	std::cout << "      The intensity of the line will be the 0." << std::endl;
	std::cout << "  -l  Assume that the images contain label ids. When combining," << std::endl;
	std::cout << "      make sure the ids are still unique." << std::endl;
}

int main(int argc, char** argv) {

	if (argc == 1) {

		std::cerr << "no arguments given" << std::endl;
		printUsage();

		return 1;
	}

	bool addMaxSeperator  = (std::string(argv[1]) == "-s");
	bool addZeroSeperator = (std::string(argv[1]) == "-s0");
	bool addSeperator     = addMaxSeperator || addZeroSeperator;

	bool labelImages = (std::string(argv[addSeperator?2:1]) == "-l");

	int  firstInputImage = 1;
	if (addSeperator)
		firstInputImage++;
	if (labelImages)
		firstInputImage++;
	int  lastInputImage   = argc - 2;
	int  numInputImages   = lastInputImage - firstInputImage + 1;

	// to be ready for spaced edge images
	int seperatorWidth = 2;

	std::vector<vigra::MultiArray<2, float> > images;

	unsigned int width  = 0;
	unsigned int height = 0;
	std::string inputPixelType;

	float labelOffset = 0;
	for (int arg = firstInputImage; arg <= lastInputImage; arg++) {

		// get information about the image to read
		vigra::ImageImportInfo info(argv[arg]);
		if (inputPixelType.empty())
			inputPixelType = info.getPixelType();

		// create new image
		images.push_back(vigra::MultiArray<2, float>(vigra::Shape2(info.width(), info.height())));

		// read image
		importImage(info, vigra::destImage(images.back()));

		if (labelImages) {

			// get the max value in this image
			float min, max;
			images.back().minmax(&min, &max);

			// add the current label offset
			for (int y = 0; y < info.height(); y++)
				for (int x = 0; x < info.width(); x++)
					if (images.back()(x, y) != 0)
						images.back()(x, y) += labelOffset;

			// increase the label offset
			labelOffset += max;
		}

		width += info.width();
		height = info.height();
	}

	if (addSeperator)
		width += seperatorWidth*(numInputImages - 1);

	vigra::MultiArray<2, float> combined(vigra::Shape2(width, height));

	if (addMaxSeperator) {

		// get max intensity
		float maxIntensity = 0;
		for (unsigned int i = 0; i < images.size(); i++) {

			float _, max;
			images[i].minmax(&_, &max);
			maxIntensity = std::max(max, maxIntensity);
		}

		std::cout << "adding separators with intensity " << maxIntensity << std::endl;

		// intialize combined image with max intensity
		combined = maxIntensity;

	} else {

		combined = 0;
	}

	unsigned int offset = 0;
	for (unsigned int i = 0; i < images.size(); i++) {

		combined.subarray(
				vigra::Shape2(offset, 0),
				vigra::Shape2(offset + images[i].width(), height)) = images[i];

		offset += images[i].width() + (addSeperator ? seperatorWidth : 0);
	}

	float min, max;
	combined.minmax(&min, &max);
	std::cout << "range of combined image: " << min << " - " << max << std::endl;

	vigra::exportImage(vigra::srcImageRange(combined), vigra::ImageExportInfo(argv[argc-1]).setPixelType(inputPixelType.c_str()));
}
