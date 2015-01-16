#ifndef MULTI2CUT_FEATURES_FEATURES_H__
#define MULTI2CUT_FEATURES_FEATURES_H__

#include <pipeline/Data.h>
#include <util/exceptions.h>

class Features : public pipeline::Data {

public:

	Features() {}

	void setFeatures(unsigned int sliceId, const std::vector<double>& features) {

		_features[sliceId] = features;
	}

	void addFeatures(unsigned int sliceId, const std::vector<double>& features) {

		std::copy(features.begin(), features.end(), std::back_inserter(_features[sliceId]));
	}

	const std::vector<double>& getFeatures(unsigned int sliceId) const {

		return _features.at(sliceId);
	}

	std::vector<double>& getFeatures(unsigned int sliceId) {

		return _features[sliceId];
	}

	inline void append(unsigned int sliceId, double value) {

		// nan -> 0
		if (value != value)
			value = 0;

		getFeatures(sliceId).push_back(value);
	}

	/**
	 * Remove all features.
	 */
	void clear() { _features.clear(); }

	/**
	 * Normalize the features such that each component is in the interval [0,1].
	 */
	void normalize() {

		findMinMax();
		normalizeMinMax(_min, _max);
	}

	/**
	 * Normalize the features such that each component is in the interval [0,1].  
	 * Instead of computing the min and max from this features, use the ones 
	 * provided.
	 */
	void normalize(const std::vector<double>& min, const std::vector<double>& max) {

		if (min.size() != max.size())
			UTIL_THROW_EXCEPTION(
					UsageError,
					"provided min and max have different sizes");

		if (_features.size() == 0)
			return;

		if (min.size() != _features.begin()->second.size())
			UTIL_THROW_EXCEPTION(
					UsageError,
					"provided min and max have different size " << min.size() << " than features " << _features.begin()->second.size());

		normalizeMinMax(min, max);
	}

	/**
	 * Get the minimal values of the features.
	 */
	const std::vector<double>& getMin() {

		if (_min.size() == 0)
			findMinMax();

		return _min;
	}

	/**
	 * Get the maximal values of the features.
	 */
	const std::vector<double>& getMax() {

		if (_max.size() == 0)
			findMinMax();

		return _max;
	}

private:

	void findMinMax() {

		_min.clear();
		_max.clear();

		typedef std::map<unsigned int, std::vector<double> >::value_type pair_t;
		foreach (pair_t& pair, _features) {

			const std::vector<double>& features = pair.second;

			if (_min.size() == 0) {

				_min = features;
				_max = features;

			} else {

				for (unsigned int i = 0; i < _min.size(); i++) {

					_min[i] = std::min(_min[i], features[i]);
					_max[i] = std::max(_max[i], features[i]);
				}
			}
		}
	}

	void normalizeMinMax(const std::vector<double>& min, const std::vector<double>& max) {

		typedef std::map<unsigned int, std::vector<double> >::value_type pair_t;
		foreach (pair_t& pair, _features) {

			std::vector<double>& features = pair.second;

			if (features.size() != min.size())
				UTIL_THROW_EXCEPTION(
						UsageError,
						"encountered a feature with differnt size " << features.size() << " than given min " << min.size());

			for (unsigned int i = 0; i < min.size(); i++) {

				if (max[i] - min[i] <= 1e-10)
					continue;

				features[i] = (features[i] - min[i])/(max[i] - min[i]);
			}
		}
	}

	std::map<unsigned int, std::vector<double> > _features;

	std::vector<double> _min;
	std::vector<double> _max;
};

#endif // MULTI2CUT_FEATURES_FEATURES_H__

