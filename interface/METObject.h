#ifndef METOBJECT_H
#define METOBJECT_H

#include "ParticleObject.h"


#define MET_EXTRA_VARIABLES \
MET_VARIABLE(float, pt) \
MET_VARIABLE(float, phi) \
MET_VARIABLE(float, covXX) \
MET_VARIABLE(float, covXY) \
MET_VARIABLE(float, covYY) \
MET_VARIABLE(float, MetUnclustEnUpDeltaX) \
MET_VARIABLE(float, MetUnclustEnUpDeltaY)


class METVariables{
public:
#define MET_VARIABLE(TYPE, NAME) TYPE NAME;
  MET_EXTRA_VARIABLES;
#undef MET_VARIABLE

  METVariables();
  METVariables(METVariables const& other);
  METVariables& operator=(const METVariables& other);

  void swap(METVariables& other);

};

class METObject{
public:
  METVariables extras;

protected:
  ParticleObject::LorentzVector_t currentXYshift;

public:
  METObject();
  METObject(const METObject& other);
  METObject& operator=(const METObject& other);
  ~METObject();

  void swap(METObject& other);

  void setXYShift(ParticleObject::LorentzVector_t const& shift){ currentXYshift=shift; }
  void setXYShift(float const& shift_x, float const& shift_y){ this->setXYShift(ParticleObject::LorentzVector_t(shift_x, shift_y, 0., 0.)); }

  void getPtPhi(float& pt, float& phi) const;
  ParticleObject::LorentzVector_t::Scalar met() const;
  ParticleObject::LorentzVector_t::Scalar phi() const;
  ParticleObject::LorentzVector_t::Scalar pt() const{ return met(); }
  ParticleObject::LorentzVector_t::Scalar px() const;
  ParticleObject::LorentzVector_t::Scalar py() const;
  ParticleObject::LorentzVector_t p4() const;

  static ParticleObject::LorentzVector_t constructP4FromPtPhi(ParticleObject::LorentzVector_t::Scalar pt, ParticleObject::LorentzVector_t::Scalar phi);
  static ParticleObject::LorentzVector_t constructP4FromPxPy(ParticleObject::LorentzVector_t::Scalar px, ParticleObject::LorentzVector_t::Scalar py);

};

#endif
