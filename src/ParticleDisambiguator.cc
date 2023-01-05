#include "MuonHandler.h"
#include "ElectronHandler.h"
#include "PhotonHandler.h"
#include "HadronicTauHandler.h"
#include "JetMETHandler.h"
#include "ParticleDisambiguator.h"
#include "MuonSelectionHelpers.h"
#include "ElectronSelectionHelpers.h"
#include "PhotonSelectionHelpers.h"
#include "HadronicTauSelectionHelpers.h"
#include "ParticleSelectionHelpers.h"
#include "ParticleObjectHelpers.h"
#include "SamplesCore.h"


void ParticleDisambiguator::disambiguateParticles(
  MuonHandler* muonHandle,
  ElectronHandler* electronHandle,
  PhotonHandler* photonHandle,
  JetMETHandler* jetHandle,
  HadronicTauHandler* htauHandle
) const{
  std::vector<MuonObject*>* muons = (muonHandle ? &(muonHandle->productList) : nullptr);
  std::vector<ElectronObject*>* electrons = (electronHandle ? &(electronHandle->productList) : nullptr);
  std::vector<PhotonObject*>* photons = (photonHandle ? &(photonHandle->productList) : nullptr);
  std::vector<HadronicTauObject*>* htaus = (htauHandle ? &(htauHandle->productList) : nullptr);
  std::vector<AK4JetObject*>* ak4jets = (jetHandle ? &(jetHandle->ak4jets) : nullptr);
  std::vector<AK4JetObject*>* ak4jets_masked = (jetHandle ? &(jetHandle->ak4jets_masked) : nullptr);
  METObject* met = (jetHandle ? jetHandle->pfmet : nullptr);

  /*
  Special case for ak4 jet - muon relationships:
  Because MET is calibrated by omitting muons, one has to keep track of the suitable muon daughters of jets.
  Every global or standalone muon that is included as a PF candidate in an ak4 jet is MET-safe, so those are subtracted from calibrations.
  Further note: CMSSW MINIAOD-like routines do not respect the conservation of total momentum when systematics are considered.
  We address this issue in the protected disambiguateParticles routine.
  */
  if (ak4jets){
    for (auto*& jet:(*ak4jets)){
      auto const& jet_idx = jet->getUniqueIdentifier();
      IvyParticle::LorentzVector_t p4_mucands(0, 0, 0, 0);
      if (muons){
        for (auto const& part:(*muons)){
          auto const& part_idx = part->getUniqueIdentifier();
          if (
            (part->extras.isGlobal || part->extras.isStandalone)
            && (
              (part->extras.jetIdx>=0 && static_cast<unsigned int>(part->extras.jetIdx)==jet_idx)
              ||
              (jet->extras.muonIdx1>=0 && part_idx==static_cast<unsigned int>(jet->extras.muonIdx1))
              ||
              (jet->extras.muonIdx2>=0 && part_idx==static_cast<unsigned int>(jet->extras.muonIdx2))
              )
            ) p4_mucands += part->p4();
        }
      }
      {
        auto const p4_uncorrected = jet->uncorrected_p4();
        double muonSubtrFactor = std::abs(jet->extras.muonSubtrFactor);
        if (muonSubtrFactor<1e-5) muonSubtrFactor = 0;
        auto p4_mucands_ALT = p4_uncorrected * muonSubtrFactor;
        // res_ALT might not have the eta and phi exactly correct, but making a mistake in pT is less desirable.
        if (std::abs(p4_mucands_ALT.Pt() - p4_mucands.Pt())>std::min(p4_uncorrected.Pt()*0.01, 0.1)) p4_mucands = p4_mucands_ALT;
      }
      jet->reset_p4_mucands(p4_mucands);
    }
  }
  // Do the same for low-pT jets. In this case, they do not have any muonIdx variables, so all that is left to do is to approximate the muon momentum.
  if (ak4jets_masked){
    for (auto*& jet:(*ak4jets_masked)){
      double muonSubtrFactor = std::abs(jet->extras.muonSubtrFactor);
      if (muonSubtrFactor<1e-5) muonSubtrFactor = 0;
      jet->reset_p4_mucands((jet->uncorrected_p4() * muonSubtrFactor));
    }
  }

  disambiguateParticles(muons, electrons, photons, htaus, ak4jets, ak4jets_masked, met);
}

void ParticleDisambiguator::disambiguateParticles(
  std::vector<MuonObject*>*& muons,
  std::vector<ElectronObject*>*& electrons,
  std::vector<PhotonObject*>*& photons,
  std::vector<HadronicTauObject*>*& htaus,
  std::vector<AK4JetObject*>*& ak4jets,
  std::vector<AK4JetObject*>*& ak4jets_masked,
  METObject* met
) const{
  bool const use_muons_TopMVAany_Run2 = (
    MuonSelectionHelpers::selection_type == MuonSelectionHelpers::kTopMVA_Run2
    ||
    MuonSelectionHelpers::selection_type == MuonSelectionHelpers::kTopMVAv2_Run2
    );
  bool const use_electrons_TopMVAany_Run2 = (
    ElectronSelectionHelpers::selection_type == ElectronSelectionHelpers::kTopMVA_Run2
    ||
    ElectronSelectionHelpers::selection_type == ElectronSelectionHelpers::kTopMVAv2_Run2
    );

  if (muons){
    for (auto*& part:(*muons)){
      double min_dr = -1;
      AK4JetObject* ak4jet_chosen = nullptr;
      if (ak4jets){
        if (use_muons_TopMVAany_Run2){
          if (part->extras.jetIdx>=0){
            unsigned int const jetIdx_unsigned = static_cast<unsigned int>(part->extras.jetIdx);
            for (auto*& jet:(*ak4jets)){
              if (jetIdx_unsigned == jet->getUniqueIdentifier()){
                ak4jet_chosen = jet;
                break;
              }
            }
          }
        }
        else if (!ak4jet_chosen){
          for (auto*& jet:(*ak4jets)){
            double tmp_dr = jet->deltaR(part);
            if (use_muons_TopMVAany_Run2 && tmp_dr>=0.4) continue; // In the Top MVA ID implementation from Kirill Skovpen, jet matching is done only when dR<0.4.
            if (min_dr<0. || tmp_dr<min_dr){
              ak4jet_chosen = jet;
              min_dr = tmp_dr;
            }
          }
        }
      }
      if (ak4jet_chosen) part->addMother(ak4jet_chosen);

      // Set the selection bits
      MuonSelectionHelpers::setSelectionBits(*part);
    }
  }
  if (electrons){
    for (auto*& part:(*electrons)){
      double min_dr = -1;
      AK4JetObject* ak4jet_chosen = nullptr;
      if (ak4jets){
        if (use_electrons_TopMVAany_Run2){
          if (part->extras.jetIdx>=0){
            unsigned int const jetIdx_unsigned = static_cast<unsigned int>(part->extras.jetIdx);
            for (auto*& jet:(*ak4jets)){
              if (jetIdx_unsigned == jet->getUniqueIdentifier()){
                ak4jet_chosen = jet;
                break;
              }
            }
          }
        }
        else if (!ak4jet_chosen){
          for (auto*& jet:(*ak4jets)){
            double tmp_dr = jet->deltaR(part);
            if (min_dr<0. || tmp_dr<min_dr){
              ak4jet_chosen = jet;
              min_dr = tmp_dr;
            }
          }
        }
      }
      if (ak4jet_chosen) part->addMother(ak4jet_chosen);

      // Set the selection bits
      ElectronSelectionHelpers::setSelectionBits(*part);
    }
  }
  if (htaus){
    for (auto*& part:(*htaus)){
      double min_dr = -1;
      AK4JetObject* ak4jet_chosen = nullptr;
      if (ak4jets){
        for (auto*& jet:(*ak4jets)){
          double tmp_dr = jet->deltaR(part);
          if (min_dr<0. || tmp_dr<min_dr){
            ak4jet_chosen = jet;
            min_dr = tmp_dr;
          }
        }
      }
      if (ak4jet_chosen) part->addMother(ak4jet_chosen);

      // Set the selection bits
      HadronicTauSelectionHelpers::setSelectionBits(*part);
    }
  }

  // Disambiguate electrons from muons
  float thr_dR_e_mu = -1;
  if (use_muons_TopMVAany_Run2 && use_electrons_TopMVAany_Run2) thr_dR_e_mu = 0.05;
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
    bool const hasMaskedJets = (ak4jets_masked!=nullptr);
    std::vector<AK4JetObject*> ak4jets_new; ak4jets_new.reserve(ak4jets->size());
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
      if (htaus){
        for (auto const* part:(*htaus)){
          if (!ParticleSelectionHelpers::isParticleForJetCleaning(part)) continue;
          if (jet->deltaR(part)<std::max(0.4, static_cast<double>(HadronicTauSelectionHelpers::getIsolationDRmax(*part)))){
            doRemove=true;
            break;
          }
        }
      }

      if (!hasMaskedJets){ if (doRemove) jet->resetSelectionBits(); }
      else{
        if (doRemove) ak4jets_masked->push_back(jet);
        else ak4jets_new.push_back(jet);
      }
    }
    if (hasMaskedJets){
      std::swap(ak4jets_new, *ak4jets);
      ParticleObjectHelpers::sortByGreaterPt(*ak4jets_masked);
    }
  }
}
