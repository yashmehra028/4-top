#ifndef HADRONICTAUOBJECT_H
#define HADRONICTAUOBJECT_H

#include "ParticleObject.h"


#define HADRONICTAU_EXTRA_VARIABLES \
HADRONICTAU_VARIABLE(int, charge, 0)


class HadronicTauVariables{
public:
#define HADRONICTAU_VARIABLE(TYPE, NAME, DEFVAL) TYPE NAME;
  HADRONICTAU_EXTRA_VARIABLES;
#undef HADRONICTAU_VARIABLE

  HadronicTauVariables();
  HadronicTauVariables(HadronicTauVariables const& other);
  HadronicTauVariables& operator=(const HadronicTauVariables& other);

  void swap(HadronicTauVariables& other);
};

class HadronicTauObject : public ParticleObject{
public:
  HadronicTauVariables extras;

  HadronicTauObject();
  HadronicTauObject(int id_);
  HadronicTauObject(int id_, LorentzVector_t const& mom_);
  HadronicTauObject(const HadronicTauObject& other);
  HadronicTauObject& operator=(const HadronicTauObject& other);
  ~HadronicTauObject();

  void swap(HadronicTauObject& other);

};

#endif
