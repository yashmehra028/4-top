#include "MuonHandler.h"
#include "ElectronHandler.h"
#include "PhotonHandler.h"
#include "JetMETHandler.h"
#include "ParticleDisambiguator.h"
#include "MuonSelectionHelpers.h"
#include "ElectronSelectionHelpers.h"
#include "PhotonSelectionHelpers.h"
#include "ParticleSelectionHelpers.h"



void ParticleDisambiguator::disambiguateParticles(
  std::vector<MuonObject*>*& muons,
  std::vector<ElectronObject*>*& electrons,
  std::vector<PhotonObject*>*& photons,
  std::vector<AK4JetObject*>*& ak4jets
){
  // First disambiguate electrons from muons
  if (electrons){
    for (auto*& product:(*electrons)){
      bool isTightProduct = ParticleSelectionHelpers::isTightParticle(product);
      bool isFakeableProduct = ParticleSelectionHelpers::isFakeableParticle(product);
      bool doRemove=false;
      if (!doRemove && muons){
        for (auto const* part:(*muons)){
          bool isTightPart = ParticleSelectionHelpers::isTightParticle(part);
          bool isFakeablePart = ParticleSelectionHelpers::isFakeableParticle(part);
          bool isLoosePart = ParticleSelectionHelpers::isLooseParticle(part);
          if (
            !(
              isTightPart
              ||
              (isFakeablePart && !isTightProduct)
              ||
              (isLoosePart && !isFakeableProduct)
              )
            ) continue;
          //if (product->deltaR(part)<std::min(0.05f, std::min(MuonSelectionHelpers::getIsolationDRmax(*part), ElectronSelectionHelpers::getIsolationDRmax(*product)))){ doRemove=true; break; }
        }
      }
      if (doRemove) product->resetSelectionBits();
    }
  }
  // Disambiguate photons from muons and *new* electrons
  if (photons){
    for (auto*& product:(*photons)){
      bool isTightProduct = ParticleSelectionHelpers::isTightParticle(product);
      bool isFakeableProduct = ParticleSelectionHelpers::isFakeableParticle(product);
      bool doRemove=false;
      if (!doRemove && muons){
        for (auto const* part:(*muons)){
          bool isTightPart = ParticleSelectionHelpers::isTightParticle(part);
          bool isFakeablePart = ParticleSelectionHelpers::isFakeableParticle(part);
          bool isLoosePart = ParticleSelectionHelpers::isLooseParticle(part);
          if (
            !(
              isTightPart
              ||
              (isFakeablePart && !isTightProduct)
              ||
              (isLoosePart && !isFakeableProduct)
              )
            ) continue;
          //if (product->deltaR(part)<std::min(0.1f, std::min(MuonSelectionHelpers::getIsolationDRmax(*part), PhotonSelectionHelpers::getIsolationDRmax(*product)))){ doRemove=true; break; }
        }
      }
      if (!doRemove && electrons){
        for (auto const* part:(*electrons)){
          bool isTightPart = ParticleSelectionHelpers::isTightParticle(part);
          bool isFakeablePart = ParticleSelectionHelpers::isFakeableParticle(part);
          bool isLoosePart = ParticleSelectionHelpers::isLooseParticle(part);
          if (
            !(
              isTightPart
              ||
              (isFakeablePart && !isTightProduct)
              ||
              (isLoosePart && !isFakeableProduct)
              )
            ) continue;
          //if (product->deltaR(part)<std::min(0.1f, std::min(ElectronSelectionHelpers::getIsolationDRmax(*part), PhotonSelectionHelpers::getIsolationDRmax(*product)))){ doRemove=true; break; }
        }
      }
      if (doRemove) product->resetSelectionBits();
    }
  }

  // Now clean the jets
  // More complicated as we need to also assign mothers to muons and electrons
  if (ak4jets){
    if (muons){
      for (auto*& part:(*muons)){
        bool isTightPart = ParticleSelectionHelpers::isTightParticle(part);
        bool isFakeablePart = ParticleSelectionHelpers::isFakeableParticle(part);
        bool isLoosePart = ParticleSelectionHelpers::isLooseParticle(part);
        double min_dr = -1;
        AK4JetObject* ak4jet_chosen = nullptr;
        if (!isTightPart && !isFakeablePart && !isLoosePart) continue;
        for (auto*& jet:(*ak4jets)){
          if (!ParticleSelectionHelpers::isTightJet(jet)) continue;
          double tmp_dr = jet->deltaR(part);
          if (min_dr<0. || tmp_dr<min_dr){
            ak4jet_chosen = jet;
            min_dr = tmp_dr;
          }
        }
        if (!ak4jet_chosen) continue;
        part->addMother(ak4jet_chosen);
        if (!(part->ptratio()>=MuonSelectionHelpers::isoThr_medium_I2 || part->ptrel()>=MuonSelectionHelpers::isoThr_medium_I3)) part->setSelectionBit(MuonSelectionHelpers::kPreselectionTight, false);
      }
    }
    if (electrons){
      for (auto*& part:(*electrons)){
        bool isTightPart = ParticleSelectionHelpers::isTightParticle(part);
        bool isFakeablePart = ParticleSelectionHelpers::isFakeableParticle(part);
        bool isLoosePart = ParticleSelectionHelpers::isLooseParticle(part);
        double min_dr = -1;
        AK4JetObject* ak4jet_chosen = nullptr;
        if (!isTightPart && !isFakeablePart && !isLoosePart) continue;
        for (auto*& jet:(*ak4jets)){
          if (!ParticleSelectionHelpers::isTightJet(jet)) continue;
          double tmp_dr = jet->deltaR(part);
          if (min_dr<0. || tmp_dr<min_dr){
            ak4jet_chosen = jet;
            min_dr = tmp_dr;
          }
        }
        if (!ak4jet_chosen) continue;
        part->addMother(ak4jet_chosen);
        if (!(part->ptratio()>=ElectronSelectionHelpers::isoThr_tight_I2 || part->ptrel()>=ElectronSelectionHelpers::isoThr_tight_I3)) part->setSelectionBit(ElectronSelectionHelpers::kPreselectionTight, false);
      }
    }

    // Now that we *actually* selected leptons, let's go back and clean the jets... (sigh)
    for (auto*& jet:(*ak4jets)){
      bool doRemove = false;
      if (muons){
        for (auto const* part:(*muons)){
          if (!ParticleSelectionHelpers::isParticleForJetCleaning(part)) continue;
          if (jet->deltaR(part)<0.4){
            doRemove=true;
            break;
          }
        }
      }
      if (electrons){
        for (auto const* part:(*electrons)){
          if (!ParticleSelectionHelpers::isParticleForJetCleaning(part)) continue;
          if (jet->deltaR(part)<0.4){
            doRemove=true;
            break;
          }
        }
      }
      if (photons){
        for (auto const* part:(*photons)){
          if (!ParticleSelectionHelpers::isParticleForJetCleaning(part)) continue;
          if (jet->deltaR(part)<0.4){
            doRemove=true;
            break;
          }
        }
      }
      if (doRemove) jet->resetSelectionBits();
    }
  }
}

void ParticleDisambiguator::disambiguateParticles(
  MuonHandler* muonHandle,
  ElectronHandler* electronHandle,
  PhotonHandler* photonHandle,
  JetMETHandler* jetHandle
){
  std::vector<MuonObject*>* muons = (muonHandle ? &(muonHandle->productList) : nullptr);
  std::vector<ElectronObject*>* electrons = (electronHandle ? &(electronHandle->productList) : nullptr);
  std::vector<PhotonObject*>* photons = (photonHandle ? &(photonHandle->productList) : nullptr);
  std::vector<AK4JetObject*>* ak4jets = (jetHandle ? &(jetHandle->ak4jets) : nullptr);

  disambiguateParticles(muons, electrons, photons, ak4jets);
}
