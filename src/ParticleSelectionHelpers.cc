#include "ParticleSelectionHelpers.h"
#include "MuonSelectionHelpers.h"
#include "ElectronSelectionHelpers.h"
#include "PhotonSelectionHelpers.h"
#include "HadronicTauSelectionHelpers.h"
#include "AK4JetSelectionHelpers.h"
#include "HelperFunctions.h"



namespace ParticleSelectionHelpers{
  bool useFakeableIdForPhysicsChecks = false;
}


#define SELECTION_TYPES \
SELECTION_TYPE(Loose) \
SELECTION_TYPE(Tight)
#define SELECTION_TYPE(TYPE) \
template<> bool ParticleSelectionHelpers::is##TYPE##Particle(MuonObject const* part){ \
  return part->testSelectionBit(MuonSelectionHelpers::kPreselection##TYPE); \
} \
template<> bool ParticleSelectionHelpers::is##TYPE##Particle(ElectronObject const* part){ \
  return part->testSelectionBit(ElectronSelectionHelpers::kPreselection##TYPE); \
} \
template<> bool ParticleSelectionHelpers::is##TYPE##Particle(PhotonObject const* part){ \
  return part->testSelectionBit(PhotonSelectionHelpers::kPreselection##TYPE); \
} \
template<> bool ParticleSelectionHelpers::is##TYPE##Particle(HadronicTauObject const* part){ \
  return part->testSelectionBit(HadronicTauSelectionHelpers::kPreselection##TYPE); \
}
SELECTION_TYPES;
#undef SELECTION_TYPE
#undef SELECTION_TYPES

#define SELECTION_TYPES \
SELECTION_TYPE(Fakeable)
#define SELECTION_TYPE(TYPE) \
template<> bool ParticleSelectionHelpers::is##TYPE##Particle(MuonObject const* part){ \
  return part->testSelectionBit(MuonSelectionHelpers::kPreselection##TYPE); \
} \
template<> bool ParticleSelectionHelpers::is##TYPE##Particle(ElectronObject const* part){ \
  return part->testSelectionBit(ElectronSelectionHelpers::kPreselection##TYPE); \
} \
template<> bool ParticleSelectionHelpers::is##TYPE##Particle(PhotonObject const* part){ return false; } \
template<> bool ParticleSelectionHelpers::is##TYPE##Particle(HadronicTauObject const* part){ return false; }
SELECTION_TYPES;
#undef SELECTION_TYPE
#undef SELECTION_TYPES

#define SELECTION_TYPES \
SELECTION_TYPE(Loose) \
SELECTION_TYPE(Fakeable) \
SELECTION_TYPE(Tight)
#define SELECTION_TYPE(TYPE) \
template<> bool ParticleSelectionHelpers::is##TYPE##Particle(ParticleObject const* part){ \
  MuonObject const* muon = dynamic_cast<MuonObject const*>(part); \
  ElectronObject const* electron = dynamic_cast<ElectronObject const*>(part); \
  PhotonObject const* photon = dynamic_cast<PhotonObject const*>(part); \
  HadronicTauObject const* htau = dynamic_cast<HadronicTauObject const*>(part); \
  if (muon) return is##TYPE##Particle(muon); \
  else if (electron) return is##TYPE##Particle(electron); \
  else if (photon) return is##TYPE##Particle(photon); \
  else if (htau) return is##TYPE##Particle(htau); \
  else return false; \
} \
template<> bool ParticleSelectionHelpers::is##TYPE##Particle(IvyParticle const* part){ return is##TYPE##Particle(dynamic_cast<ParticleObject const*>(part)); }
SELECTION_TYPES;
#undef SELECTION_TYPE
#undef SELECTION_TYPES


// Functions for jets
#define SELECTION_TYPES \
SELECTION_TYPE(Tight)

#define SELECTION_TYPE(TYPE) \
template<> bool ParticleSelectionHelpers::is##TYPE##Jet(AK4JetObject const* part){ \
  return part->testSelectionBit(AK4JetSelectionHelpers::kPreselection##TYPE); \
} \
template<> bool ParticleSelectionHelpers::is##TYPE##Jet(ParticleObject const* part){ \
  AK4JetObject const* ak4jet = dynamic_cast<AK4JetObject const*>(part); \
  if (ak4jet) return is##TYPE##Jet(ak4jet); \
  else return false; \
}

SELECTION_TYPES;

#undef SELECTION_TYPE
#undef SELECTION_TYPES


void ParticleSelectionHelpers::setUseFakeableIdForPhysicsChecks(bool flag){ useFakeableIdForPhysicsChecks = flag; }

template<> bool ParticleSelectionHelpers::isParticleForJetCleaning<MuonObject>(MuonObject const* part){
  return (isTightParticle(part) || (useFakeableIdForPhysicsChecks && isFakeableParticle(part)));
}
template<> bool ParticleSelectionHelpers::isParticleForJetCleaning<ElectronObject>(ElectronObject const* part){
  return (isTightParticle(part) || (useFakeableIdForPhysicsChecks && isFakeableParticle(part)));
}

template<> bool ParticleSelectionHelpers::isParticleForTriggerChecking<MuonObject>(MuonObject const* part){
  return (isTightParticle(part) || (useFakeableIdForPhysicsChecks && isFakeableParticle(part)));
}
template<> bool ParticleSelectionHelpers::isParticleForTriggerChecking<ElectronObject>(ElectronObject const* part){
  return (isTightParticle(part) || (useFakeableIdForPhysicsChecks && isFakeableParticle(part)));
}

template<> bool ParticleSelectionHelpers::isJetForHEMVeto<AK4JetObject>(AK4JetObject const* jet){
  // No PU jet id requirement, so do not use the isTightJet check.
  // No eta requirement either...
  return jet->testSelectionBit(AK4JetSelectionHelpers::kJetIdOnly) && jet->pt()>=AK4JetSelectionHelpers::ptThr_HEMVeto;
}
