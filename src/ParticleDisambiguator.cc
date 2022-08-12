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
  if (muons){
    for (auto*& part:(*muons)){
      double min_dr = -1;
      AK4JetObject* ak4jet_chosen = nullptr;
      if (ak4jets){
        for (auto*& jet:(*ak4jets)){
          double tmp_dr = jet->deltaR(part);
          if (
            (
              MuonSelectionHelpers::selection_type == MuonSelectionHelpers::kTopMVA_Run2
              ||
              MuonSelectionHelpers::selection_type == MuonSelectionHelpers::kTopMVAv2_Run2
              )
            &&
            tmp_dr>=0.4
            ) continue; // In the Top MVA ID implementation from Kirill Skovpen, jet matching is done only when dR<0.4.
          if (min_dr<0. || tmp_dr<min_dr){
            ak4jet_chosen = jet;
            min_dr = tmp_dr;
          }
        }
      }
      if (ak4jet_chosen) part->addMother(ak4jet_chosen);

      MuonSelectionHelpers::setSelectionBits(*part);
    }
  }
  if (electrons){
    for (auto*& part:(*electrons)){
      double min_dr = -1;
      AK4JetObject* ak4jet_chosen = nullptr;
      if (ak4jets){
        for (auto*& jet:(*ak4jets)){
          double tmp_dr = jet->deltaR(part);
          if (
            (
              ElectronSelectionHelpers::selection_type == ElectronSelectionHelpers::kTopMVA_Run2
              ||
              ElectronSelectionHelpers::selection_type == ElectronSelectionHelpers::kTopMVAv2_Run2
              )
            &&
            tmp_dr>=0.4
            ) continue; // In the Top MVA ID implementation from Kirill Skovpen, jet matching is done only when dR<0.4.
          if (min_dr<0. || tmp_dr<min_dr){
            ak4jet_chosen = jet;
            min_dr = tmp_dr;
          }
        }
      }
      if (ak4jet_chosen) part->addMother(ak4jet_chosen);

      ElectronSelectionHelpers::setSelectionBits(*part);
    }
  }

  // Disambiguate electrons from muons
  float thr_dR_e_mu = -1;
  if (
    (
      MuonSelectionHelpers::selection_type == MuonSelectionHelpers::kTopMVA_Run2
      ||
      MuonSelectionHelpers::selection_type == MuonSelectionHelpers::kTopMVAv2_Run2
      )
    &&
    (
      ElectronSelectionHelpers::selection_type == ElectronSelectionHelpers::kTopMVA_Run2
      ||
      ElectronSelectionHelpers::selection_type == ElectronSelectionHelpers::kTopMVAv2_Run2
      )
    ) thr_dR_e_mu = 0.05;
  if (electrons && thr_dR_e_mu>=0.f){
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
          if (product->deltaR(part)<thr_dR_e_mu){ doRemove=true; break; }
        }
      }
      if (doRemove) product->resetSelectionBits();
    }
  }
  // Disambiguate photons from muons and *new* electrons
  // Could later set these to 0.1, disabled for now.
  constexpr float thr_dR_gamma_mu = -1;
  constexpr float thr_dR_gamma_e = -1;
  if (photons){
    for (auto*& product:(*photons)){
      bool isTightProduct = ParticleSelectionHelpers::isTightParticle(product);
      bool isFakeableProduct = ParticleSelectionHelpers::isFakeableParticle(product);
      bool doRemove=false;
      if (!doRemove && muons && thr_dR_gamma_mu>=0.f){
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
          if (product->deltaR(part)<thr_dR_gamma_mu){ doRemove=true; break; }
        }
      }
      if (!doRemove && electrons && thr_dR_gamma_e>=0.f){
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
          if (product->deltaR(part)<thr_dR_gamma_e){ doRemove=true; break; }
        }
      }
      if (doRemove) product->resetSelectionBits();
    }
  }

  // Now clean the jets at the final step
  if (ak4jets){
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
