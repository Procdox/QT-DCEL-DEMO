#pragma once
#include "DCEL/Grid_Point.h"

#include <vector>

class Tensor_Effect {
	Pgrd position;
	virtual Pgrd generate(const Pgrd &) const = 0;
public:
	Tensor_Effect(const Pgrd&);
	Pgrd AtPoint(const Pgrd &, const grd& decay) const;
};

class Radial_Effect : public Tensor_Effect {
	Pgrd generate(const Pgrd&) const override;

public:
	Radial_Effect(const Pgrd&, const grd&);
	
private:
	grd strength;
};
class Grid_Effect : public Tensor_Effect {
	Pgrd generate(const Pgrd&) const override;
public:
	Grid_Effect(const Pgrd&, const Pgrd&);

private:
	Pgrd strength;
};
class Tensor_Field {
	
public:
	Tensor_Field() {
		for (auto effect : tensor_effects) delete effect;
	}
	Tensor_Field(const grd&);
	Pgrd SumPoint(const Pgrd&) const;
  Pgrd SumPointContextAware(const Pgrd&, const Pgrd&) const;
	void AddEffect(Tensor_Effect*);

private:
	std::vector<Tensor_Effect*> tensor_effects;
	grd decay_constant;
};