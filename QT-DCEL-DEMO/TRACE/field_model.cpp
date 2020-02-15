#include "field_model.h"

Tensor_Effect::Tensor_Effect(const Pgrd& _p)
: position(_p) {}

Pgrd Tensor_Effect::AtPoint(const Pgrd & target, const grd& decay_constant) const {
	const Pgrd offset = target - position;
	const grd decay = exp((-decay_constant * offset.SizeSquared()).n);
	return generate(offset) *= decay;
}

Radial_Effect::Radial_Effect(const Pgrd& _p, const grd& _s) 
: Tensor_Effect(_p)
, strength(_s) {}

Pgrd Radial_Effect::generate(const Pgrd& offset) const {
	Pgrd result(offset.Y * -1, offset.X);
	result.Normalize();
	result *= strength;
	return result;
}

Grid_Effect::Grid_Effect(const Pgrd& _p, const Pgrd& _s)
: Tensor_Effect(_p)
, strength(_s) {}

Pgrd Grid_Effect::generate(const Pgrd&) const {
	return strength;
}

Tensor_Field::Tensor_Field(const grd& decay)
: decay_constant(decay) {}

Pgrd Tensor_Field::SumPoint(const Pgrd& target) const {
	Pgrd basis;
	for (const auto& effect : tensor_effects)
		basis += effect->AtPoint(target, decay_constant);
	return basis;
}

void Tensor_Field::AddEffect(Tensor_Effect* target) {
	tensor_effects.emplace_back(target);
}