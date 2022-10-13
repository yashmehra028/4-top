#ifndef LHEPARTICLEOBJECT_H
#define LHEPARTICLEOBJECT_H

#include "ParticleObject.h"


#define LHEPARTICLE_ID_MOMENTUM_VARIABLES \
LHEPARTICLE_VARIABLE(int, pdgId, 0) \
LHEPARTICLE_VARIABLE(int, status, 0) \
LHEPARTICLE_VARIABLE(float, pt, 0) \
LHEPARTICLE_VARIABLE(float, eta, 0) \
LHEPARTICLE_VARIABLE(float, phi, 0) \
LHEPARTICLE_VARIABLE(float, mass, 0)

#define LHEPARTICLE_EXTRA_VARIABLES \
LHEPARTICLE_VARIABLE(float, incomingpz, 0)

#define LHEPARTICLE_VARIABLES \
LHEPARTICLE_ID_MOMENTUM_VARIABLES \
LHEPARTICLE_EXTRA_VARIABLES


class LHEParticleVariables{
public:
#define LHEPARTICLE_VARIABLE(TYPE, NAME, DEFVAL) TYPE NAME;
  LHEPARTICLE_EXTRA_VARIABLES;
#undef LHEPARTICLE_VARIABLE

  LHEParticleVariables();
  LHEParticleVariables(LHEParticleVariables const& other);
  LHEParticleVariables& operator=(const LHEParticleVariables& other);

  void swap(LHEParticleVariables& other);

};

class LHEParticleObject : public ParticleObject{
protected:
  int st;

public:
  LHEParticleVariables extras;

  LHEParticleObject();
  LHEParticleObject(int id_, int st_);
  LHEParticleObject(int id_, int st_, LorentzVector_t const& mom_);
  LHEParticleObject(const LHEParticleObject& other);
  LHEParticleObject& operator=(const LHEParticleObject& other);
  ~LHEParticleObject();

  void swap(LHEParticleObject& other);

  int& status(){ return this->st; }
  int const& status() const{ return this->st; }

};

#endif
