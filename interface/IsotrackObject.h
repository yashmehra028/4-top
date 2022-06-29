#ifndef ISOTRACKOBJECT_H
#define ISOTRACKOBJECT_H

#include "ParticleObject.h"


#define ISOTRACK_EXTRA_VARIABLES \
ISOTRACK_VARIABLE(bool, isPFcand) \
ISOTRACK_VARIABLE(bool, isFromLostTrack) \
ISOTRACK_VARIABLE(bool, isHighPurityTrack) \
ISOTRACK_VARIABLE(int, fromPV) \
ISOTRACK_VARIABLE(float, pfRelIso03_chg) \
ISOTRACK_VARIABLE(float, pfRelIso03_all) \
ISOTRACK_VARIABLE(float, miniPFRelIso_chg) \
ISOTRACK_VARIABLE(float, miniPFRelIso_all) \
ISOTRACK_VARIABLE(float, dxy) \
ISOTRACK_VARIABLE(float, dz)


class IsotrackVariables{
public:
#define ISOTRACK_VARIABLE(TYPE, NAME) TYPE NAME;
  ISOTRACK_EXTRA_VARIABLES;
#undef ISOTRACK_VARIABLE

  IsotrackVariables();
  IsotrackVariables(IsotrackVariables const& other);
  IsotrackVariables& operator=(const IsotrackVariables& other);

  void swap(IsotrackVariables& other);

};

class IsotrackObject : public ParticleObject{
public:
  IsotrackVariables extras;

  IsotrackObject();
  IsotrackObject(int id);
  IsotrackObject(int id, LorentzVector_t const& mom_);
  IsotrackObject(const IsotrackObject& other);
  IsotrackObject& operator=(const IsotrackObject& other);
  ~IsotrackObject();

  void swap(IsotrackObject& other);

};

#endif
