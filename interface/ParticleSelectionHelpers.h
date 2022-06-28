#ifndef PARTICLESELECTIONHELPERS_H
#define PARTICLESELECTIONHELPERS_H

#include "MuonObject.h"
#include "ElectronObject.h"
#include "PhotonObject.h"
#include "AK4JetObject.h"


namespace ParticleSelectionHelpers{
#define SELECTION_TYPES \
SELECTION_TYPE(Loose) \
SELECTION_TYPE(Tight) \
SELECTION_TYPE(Fakeable)

#define SELECTION_TYPE(TYPE) \
  template<typename T> bool is##TYPE##Particle(T const* part); \
  template<> bool is##TYPE##Particle(ParticleObject const* part); \
  template<> bool is##TYPE##Particle(MuonObject const* part); \
  template<> bool is##TYPE##Particle(ElectronObject const* part); \
  template<> bool is##TYPE##Particle(PhotonObject const* part); \
  template<> bool is##TYPE##Particle(IvyParticle const* part);

  SELECTION_TYPES;

#undef SELECTION_TYPE
#undef SELECTION_TYPES

  template<typename T> bool isParticleForJetCleaning(T const* part);
  template<typename T> bool isParticleForTriggerChecking(T const* part);

  template<typename T> bool isJetForTriggerChecking(T const* jet);
  template<typename T> bool isJetForHEMVeto(T const*);
  template<> bool isJetForHEMVeto<AK4JetObject>(AK4JetObject const* jet);

  // Selection on jets
#define SELECTION_TYPES \
SELECTION_TYPE(Tight)

#define SELECTION_TYPE(TYPE) \
  template<typename T> bool is##TYPE##Jet(T const* part); \
  template<> bool is##TYPE##Jet(AK4JetObject const* part);

  SELECTION_TYPES;

#undef SELECTION_TYPE
#undef SELECTION_TYPES

}

template<typename T> bool ParticleSelectionHelpers::isParticleForJetCleaning(T const* part){ return isLooseParticle(part); }
template bool ParticleSelectionHelpers::isParticleForJetCleaning<MuonObject>(MuonObject const*);
template bool ParticleSelectionHelpers::isParticleForJetCleaning<ElectronObject>(ElectronObject const*);
template bool ParticleSelectionHelpers::isParticleForJetCleaning<PhotonObject>(PhotonObject const*);

template<typename T> bool ParticleSelectionHelpers::isParticleForTriggerChecking(T const* part){ return isLooseParticle(part); }
template bool ParticleSelectionHelpers::isParticleForTriggerChecking<MuonObject>(MuonObject const*);
template bool ParticleSelectionHelpers::isParticleForTriggerChecking<ElectronObject>(ElectronObject const*);
template bool ParticleSelectionHelpers::isParticleForTriggerChecking<PhotonObject>(PhotonObject const*);

template<typename T> bool ParticleSelectionHelpers::isJetForTriggerChecking(T const* jet){ return isTightJet(jet); }
template bool ParticleSelectionHelpers::isJetForTriggerChecking<AK4JetObject>(AK4JetObject const*);

#endif
