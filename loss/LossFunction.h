#ifndef MULTI2CUT_LOSS_LOSS_FUNCTION_H__
#define MULTI2CUT_LOSS_LOSS_FUNCTION_H__

class LossFunction {

public:

	LossFunction() :
		_constant(0) {}

	typedef std::map<unsigned int, double>                 losses_type;
	typedef std::map<unsigned int, double>::iterator       iterator;
	typedef std::map<unsigned int, double>::const_iterator const_iterator;

	const double& operator[](unsigned int id) const { return _losses.at(id); }

	double& operator[](unsigned int id) { return _losses[id]; }

	void setConstant(double constant) { _constant = constant; }

	double getConstant() { return _constant; }

	losses_type::iterator begin() { return _losses.begin(); }
	losses_type::const_iterator begin() const { return _losses.begin(); }
	losses_type::iterator end() { return _losses.end(); }
	losses_type::const_iterator end() const { return _losses.end(); }

	void clear() { _losses.clear(); _constant = 0; }

private:

	losses_type _losses;

	double _constant;
};

#endif // MULTI2CUT_LOSS_LOSS_FUNCTION_H__

