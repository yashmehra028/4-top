#ifndef GENJETOBJECT_H
#define GENJETOBJECT_H

#include "ParticleObject.h"


#define GENJET_EXTRA_VARIABLES \
GENJET_VARIABLE(int, hadronFlavour, 0) \
GENJET_VARIABLE(int, partonFlavour, 0)

#define GENJET_MOMENTUM_VARIABLES \
GENJET_VARIABLE(float, pt, 0) \
GENJET_VARIABLE(float, eta, 0) \
GENJET_VARIABLE(float, phi, 0) \
GENJET_VARIABLE(float, mass, 0)

#define GENJET_NANOAOD_VARIABLES \
GENJET_MOMENTUM_VARIABLES \
GENJET_EXTRA_VARIABLES


class GenJetVariables{
public:
#define GENJET_VARIABLE(TYPE, NAME, DEFVAL) TYPE NAME;
  GENJET_EXTRA_VARIABLES;
#undef GENJET_VARIABLE

  GenJetVariables();
  GenJetVariables(GenJetVariables const& other);
  GenJetVariables& operator=(const GenJetVariables& other);

  void swap(GenJetVariables& other);
};


class GenJetObject : public ParticleObject{
public:
  GenJetVariables extras;

  GenJetObject();
  GenJetObject(LorentzVector_t const& mom_);
  GenJetObject(const GenJetObject& other);
  GenJetObject& operator=(const GenJetObject& other);
  ~GenJetObject();

  void swap(GenJetObject& other);
};

#endif
