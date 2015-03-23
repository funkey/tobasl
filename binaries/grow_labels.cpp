#include <iostream>
#include <map>
#include <vigra/impex.hxx>
#include <vigra/multi_array.hxx>

void printUsage() {

	std::cout << std::endl;
	std::cout << "grow_labels <label_image> <out> <max_threshold> <min_threshold>" << std::endl;
}

int main(int argc, char** argv) {

	if (argc != 5) {

		printUsage();

		return 1;
	}
	char* labelFile = argv[1];
	char* outFile   = argv[2];

	int maxThreshold = atoi(argv[3]);
	int minThreshold = atoi(argv[4]);

	vigra::ImageImportInfo info(labelFile);
	vigra::MultiArray<2, float> labels(vigra::Shape2(info.width(), info.height()));
	importImage(info, vigra::destImage(labels));

	unsigned int width  = info.width();
	unsigned int height = info.height();

	bool changed = true;

	while (changed) {

		changed = false;

		for (unsigned int y = 0; y < labels.height(); y++)
			for (unsigned int x = 0; x < labels.width(); x++) {

				float label = 0;
				bool  moreThanOne = false;

				// check neighbors for more then one non-zero label
				for (int dy = (y == 0 ? 0 : -1); dy <= (y == height - 1 ? 0 : 1) && !moreThanOne; dy++) {
					for (int dx = (x == 0 ? 0 : -1); dx <= (x == width - 1 ? 0 : 1) && !moreThanOne; dx++) {

						if (dx == 0 && dy == 0)
							continue;

						float l = labels(x + dx, y + dy);

						if (l == 0)
							continue;

						// found a second label in the neighborhood
						if (label != 0 && label != l) {

							moreThanOne = true;
							break;
						}

						label = l;
					}
				}

				// there is only one label in our neighborhood
				if (!moreThanOne && label != 0 && labels(x, y) == 0) {

					labels(x, y) = label;
					changed = true;
				}
			}
	}

	// remove regions that are too large or too small
	std::map<float, unsigned int> regionSizes;
	for (unsigned int y = 0; y < labels.height(); y++)
		for (unsigned int x = 0; x < labels.width(); x++)
			regionSizes[labels(x, y)]++;

	for (std::map<float, unsigned int>::const_iterator i = regionSizes.begin(); i != regionSizes.end(); i++) {

		if (i->second >= maxThreshold || i->second <= minThreshold)
			for (unsigned int y = 0; y < labels.height(); y++)
				for (unsigned int x = 0; x < labels.width(); x++)
					if (labels(x, y) == i->first)
						labels(x, y) = 0;
	}

	vigra::exportImage(vigra::srcImageRange(labels), vigra::ImageExportInfo(outFile));
}

