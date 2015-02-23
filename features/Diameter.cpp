#include <vigra/polygon.hxx>
#include <vigra/impex.hxx>
#include <imageprocessing/ConnectedComponent.h>
#include <util/Logger.h>
#include "Diameter.h"

logger::LogChannel diameterlog("diameterlog", "[Diameter] ");

double
Diameter::operator()(const Slice& slice) {

	LOG_ALL(diameterlog)
			<< "processing slice "
			<< slice.getId()
			<< std::endl;

	return operator()(*slice.getComponent());
}

double
Diameter::operator()(const ConnectedComponent& component) {

	if (component.getSize() == 0)
		return 0;

	const ConnectedComponent::bitmap_type& image = component.getBitmap();

	// find an anchor point
	vigra::Shape2 anchor;
	for (unsigned int x = 0; x < image.width(); x++)
	for (unsigned int y = 0; y < image.height(); y++) {
		if (image(x, y) == 1) {

			anchor = vigra::Shape2(x + component.getBoundingBox().minX, y + component.getBoundingBox().minY);
		}
	}

	vigra::exportImage(
			image,
			vigra::ImageExportInfo("slice.png"));

	LOG_ALL(diameterlog)
			<< "anchor point is "
			<< anchor
			<< std::endl;

	// extract contour
	vigra::Polygon<vigra::TinyVector<double, 2> > contour;
	vigra::extractContour(image, anchor, contour);

	LOG_ALL(diameterlog) << "contour hull is:" << std::endl;
	for (unsigned int i = 0; i < contour.size(); i++)
		LOG_ALL(diameterlog) << "\t" << contour[i] << std::endl;

	// get convex hull
	vigra::Polygon<vigra::TinyVector<double, 2> > hull;
	vigra::convexHull(contour, hull);

	LOG_ALL(diameterlog) << "convex hull is:" << std::endl;
	for (unsigned int i = 0; i < hull.size(); i++)
		LOG_ALL(diameterlog) << "\t" << hull[i] << std::endl;

	// find max distance between any pair of corner points
	double maxDistance2 = 0;
	unsigned int maxi = 0;
	unsigned int maxj = 0;
	for (unsigned int i = 0; i < hull.size(); i++)
		for (unsigned int j = i+1; j < hull.size(); j++) {

			double distance2 =
					pow(hull[i][0] - hull[j][0], 2) +
					pow(hull[i][1] - hull[j][1], 2);

			if (maxDistance2 < distance2) {

				maxDistance2 = distance2;
				maxi = i;
				maxj = j;
			}
		}

	LOG_ALL(diameterlog)
			<< "max diameter " << sqrt(maxDistance2)
			<< std::endl;

	LOG_ALL(diameterlog)
			<< "from point " << hull[maxi]
			<< " to " << hull[maxj] << std::endl;

	return sqrt(maxDistance2);
}
