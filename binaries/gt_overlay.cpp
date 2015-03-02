#include <iostream>
#include <vigra/impex.hxx>
#include <vigra/multi_array.hxx>

void printUsage() {

	std::cout << std::endl;
	std::cout << "gt_overlay <raw_image> <label_image> <out>" << std::endl;
	std::cout << std::endl;
	std::cout << "  -d  Let the regions with a zero label appear dark." << std::endl;
}

void
hsvToRgb(double h, double s, double v, unsigned char& r, unsigned char& g, unsigned char& b) {

	if(s < 0) s = 0;
	if(s > 1) s = 1;
	if(v < 0) v = 0;
	if(v > 1) v = 1;

	if(s == 0) {
		r = (unsigned char)255.0*v;
		g = (unsigned char)255.0*v;
		b = (unsigned char)255.0*v;
	}

	h = fmod(h, 1.0); // want h to be in 0..1

	unsigned int i = h*6;
	double f = (h*6) - i;
	double p = v*(1.0f - s); 
	double q = v*(1.0f - s*f);
	double t = v*(1.0f - s*(1.0f-f));
	switch(i%6) {
	case 0:
		r = (unsigned char)255.0*v;
		g = (unsigned char)255.0*t;
		b = (unsigned char)255.0*p;
		return;
	case 1:
		r = (unsigned char)255.0*q;
		g = (unsigned char)255.0*v;
		b = (unsigned char)255.0*p;
		return;
	case 2:
		r = (unsigned char)255.0*p;
		g = (unsigned char)255.0*v;
		b = (unsigned char)255.0*t;
		return;
	case 3:
		r = (unsigned char)255.0*p;
		g = (unsigned char)255.0*q;
		b = (unsigned char)255.0*v;
		return;
	case 4:
		r = (unsigned char)255.0*t;
		g = (unsigned char)255.0*p;
		b = (unsigned char)255.0*v;
		return;
	case 5:
		r = (unsigned char)255.0*v;
		g = (unsigned char)255.0*p;
		b = (unsigned char)255.0*q;
		return;
	}
}

void
idToRgb(unsigned int id, unsigned char& r, unsigned char& g, unsigned char& b) {

	float h = fmod(static_cast<float>(id)*M_PI, 1.0);
	float s = 0.5 + fmod(static_cast<float>(id)*M_PI*2, 0.5);
	float v = (id == 0 ? 0.0 : 1.0);
	hsvToRgb(h, s, v, r, g, b);
}

int main(int argc, char** argv) {

	if (argc < 4) {

		printUsage();

		return 1;
	}

	bool darkBackground = false;

	std::string arg1 = argv[1];
	if (arg1 == "-d")
		darkBackground = true;

	char* rawFile   = argv[1 + darkBackground];
	char* labelFile = argv[2 + darkBackground];
	char* outFile   = argv[3 + darkBackground];

	vigra::ImageImportInfo info(rawFile);
	vigra::MultiArray<2, float> raw(vigra::Shape2(info.width(), info.height()));
	importImage(info, vigra::destImage(raw));

	float min, max;
	raw.minmax(&min, &max);
	std::cout << "raw image values are in range " << min << " - " << max << std::endl;

	float normFactor = 1.0;
	if (max > 1.0 && max < 256.0) {

		std::cout << "assuming unsigned char intensities" << std::endl;
		normFactor = 1.0/255.0;
	}

	info = vigra::ImageImportInfo(labelFile);
	vigra::MultiArray<2, float> labels(vigra::Shape2(info.width(), info.height()));
	importImage(info, vigra::destImage(labels));

	labels.minmax(&min, &max);
	std::cout << "label image values are in range " << min << " - " << max << std::endl;

	vigra::MultiArray<2, vigra::TinyVector<unsigned char, 3> > combined(vigra::Shape2(info.width(), info.height()));

	// amount of colorization
	double alpha = 0.75;

	for (unsigned int y = 0; y < labels.height(); y++)
		for (unsigned int x = 0; x < labels.width(); x++) {

			float        intensity = raw(x, y);
			unsigned int label     = labels(x, y);

			unsigned char r = 255;
			unsigned char g = 255;
			unsigned char b = 255;

			if (label != 0 || darkBackground)
				idToRgb(label, r, g, b);

			combined(x, y)[0] = (1.0 - alpha)*intensity + alpha*intensity*normFactor*r;
			combined(x, y)[1] = (1.0 - alpha)*intensity + alpha*intensity*normFactor*g;
			combined(x, y)[2] = (1.0 - alpha)*intensity + alpha*intensity*normFactor*b;
		}

	vigra::exportImage(vigra::srcImageRange(combined), vigra::ImageExportInfo(outFile));
}

