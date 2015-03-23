#include <iostream>
#include <map>
#include <vigra/impex.hxx>
#include <vigra/multi_array.hxx>

void printUsage() {

	std::cout << std::endl;
	std::cout << "shrink_labels <label_image> <out> <amount>" << std::endl;
}

int main(int argc, char** argv) {

	if (argc != 4) {

		printUsage();

		return 1;
	}
	char* labelFile = argv[1];
	char* outFile   = argv[2];

	int amount = atoi(argv[3]);

	vigra::ImageImportInfo info(labelFile);
	vigra::MultiArray<2, float> labels(vigra::Shape2(info.width(), info.height()));
	vigra::MultiArray<2, float> shrink(vigra::Shape2(info.width(), info.height()));
	importImage(info, vigra::destImage(labels));
	shrink = labels;

	unsigned int width  = info.width();
	unsigned int height = info.height();


	for (int i = 0; i < amount; i++) {

		for (unsigned int y = 0; y < labels.height(); y++)
			for (unsigned int x = 0; x < labels.width(); x++) {

				bool zeroLabel = false;

				// check neighbors for zero-label
				for (int dy = (y == 0 ? 0 : -1); dy <= (y == height - 1 ? 0 : 1) && !zeroLabel; dy++) {
					for (int dx = (x == 0 ? 0 : -1); dx <= (x == width - 1 ? 0 : 1) && !zeroLabel; dx++) {

						if (dx == 0 && dy == 0)
							continue;

						if (labels(x + dx, y + dy) == 0) {
							zeroLabel = true;
							break;
						}
					}
				}

				if (zeroLabel)
					shrink(x, y) = 0;
			}

		labels = shrink;
	}

	vigra::exportImage(vigra::srcImageRange(labels), vigra::ImageExportInfo(outFile));
}


