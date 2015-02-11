#ifndef MULTI2CUT_FEATURES_DIAMETER_H__
#define MULTI2CUT_FEATURES_DIAMETER_H__

#include <slices/Slice.h>

class Diameter {

public:

	/**
	 * Compute the maximum diameter of the given slice, i.e., the length of the 
	 * longest line connecting two boundary points.
	 */
	double operator()(const Slice& slice);

	/**
	 * Compute the maximum diameter of the given connected component, i.e., the 
	 * length of the longest line connecting two boundary points.
	 */
	double operator()(const ConnectedComponent& component);
};

#endif // MULTI2CUT_FEATURES_DIAMETER_H__

