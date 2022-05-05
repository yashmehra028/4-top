#include <NanoTools/NanoCORE/Tools/jetcorr/FactorizedJetCorrector.h> 
#include <NanoTools/NanoCORE/Nano.h> // to use namespace tas
#include <NanoTools/NanoCORE/IsolationTools.h> // to use coneCorrPt
#include <tree_tools.h> // for global variable definitions

bool passes_baseline_ft(int njets, int nbtags, float met, float ht, int id1, int id2, float lep1_pt, float lep2_pt) {
    if (lep1_pt < 25.) return 0;
    else if (lep2_pt < 20.) return 0;
    else if (njets < 2) return 0;
    else if (nbtags < 2) return 0;
    else if (ht < 300) return 0;
    else if (met < 50) return 0;
    else return 1;
}

//doCorr: 0-built-in, 1-corrected, 2-raw
std::pair <std::vector <Jet>, std::vector <Jet> > SSJetsCalculator(FactorizedJetCorrector* jetCorr, int doCorr, bool isFastsim, bool saveAllPt){
  std::vector <Jet> result_jets;
  std::vector <Jet> result_btags;
  std::vector <float> result_disc;
  std::vector <float> result_corrpt;

  for (unsigned int i = 0; i < tas::pfjets_p4().size(); i++){
    LorentzVector jet = tas::pfjets_p4().at(i);

    //Jet Corr
    jetCorr->setJetEta(jet.eta()); 
    jetCorr->setJetPt(jet.pt()*tas::pfjets_undoJEC().at(i)); 
    jetCorr->setJetA(tas::pfjets_area().at(i)); 
    jetCorr->setRho(tas::evt_fixgridfastjet_all_rho()); 
    float JEC = jetCorr->getCorrection(); 

    //Jet pT to use
    float pt = jet.pt(); 
    if (doCorr == 1) pt = jet.pt()*tas::pfjets_undoJEC().at(i)*JEC;
    if (doCorr == 2) pt = jet.pt()*tas::pfjets_undoJEC().at(i);
    
    //Kinematic jet cuts
    if (pt < bjetMinPt) continue;
    if (fabs(jet.eta()) > 2.4) continue;

    //Require jet ID
    if (!pass_SS_jetID(i,isFastsim)) continue;

    //Get discriminator
    auto jetobj = Jet(i, JEC);
    float disc = jetobj.disc();

    result_jets.push_back(jetobj);
    result_disc.push_back(disc);
    result_corrpt.push_back(pt);

  }

  // Clean all jets inclusively
  std::vector <bool> keep_jets = cleanJets(result_jets);
  int j = 0; 
  for (unsigned int i = 0; i < keep_jets.size(); i++){
    if (!keep_jets[i]) {
        result_jets.erase(result_jets.begin()+j); 
        result_disc.erase(result_disc.begin()+j); 
        result_corrpt.erase(result_corrpt.begin()+j); 
    }
    else j++; 
  }

  // Classify b-jets
  for (unsigned int i = 0; i < result_jets.size(); i++){
      float disc = result_disc.at(i);
      if (disc < gconf.btag_disc_wp) continue;
      result_btags.push_back(result_jets.at(i));
  }

  // Only retain high pt jets if not saving all pts
  std::vector<Jet> result_jets_cut;
  for (unsigned int i = 0; i < result_jets.size(); i++){
      if(!saveAllPt && (result_corrpt.at(i) < jetMinPt)) continue;
      result_jets_cut.push_back(result_jets.at(i));
  }

  std::pair <std::vector <Jet>, std::vector <Jet> > result = std::make_pair(result_jets_cut, result_btags);

  return result;
}


void jets_calculator(){
//Determine and save jet and b-tag variables, raw
  std::pair <std::vector <Jet>, std::vector <Jet> > jet_results = SSJetsCalculator(jetCorr, 2, isFastsim);
  std::vector <LorentzVector> jets_raw;
  std::vector <LorentzVector> btags_raw;
  std::vector <float> jets_undoJEC_raw;
  for (unsigned int i = 0; i < jet_results.first.size(); i++)  jets_raw.push_back(jet_results.first.at(i).p4());
  for (unsigned int i = 0; i < jet_results.second.size(); i++) btags_raw.push_back(jet_results.second.at(i).p4());
  for (unsigned int i = 0; i < jet_results.first.size(); i++)  jets_undoJEC_raw.push_back(jet_results.first.at(i).undo_jec());
  njets_raw = jets_raw.size();
  nbtags_raw = btags_raw.size();
  ht_raw = 0;
  for (unsigned int i = 0; i < jets_raw.size(); i++) ht_raw += jets_raw.at(i).pt()*jets_undoJEC_raw.at(i);


  //Determine and save jet and b-tag variables, corrected
  jet_results = SSJetsCalculator(jetCorr, 1);
  for (unsigned int i = 0; i < jet_results.first.size(); i++) jets.push_back(jet_results.first.at(i).p4());
  for (unsigned int i = 0; i < jet_results.first.size(); i++) jets_flavor.push_back(jet_results.first.at(i).mcFlavor()); // NJA
  for (unsigned int i = 0; i < jet_results.first.size(); i++) jets_disc.push_back(jet_results.first.at(i).disc());
  std::vector<int> good_ijqs;
  std::vector<int> good_ijbs;
  for (unsigned int i = 0; i < jet_results.first.size(); i++) {
      auto jet = jet_results.first.at(i);
      if (!jet.isBtag()) good_ijqs.push_back(jet.idx());
  }
  for (unsigned int i = 0; i < jet_results.second.size(); i++) {
      auto jet = jet_results.second.at(i);
      good_ijbs.push_back(jet.idx());
  }

  for (unsigned int i = 0; i < jet_results.first.size(); i++) jets_JEC.push_back(jet_results.first.at(i).jec());
  for (unsigned int i = 0; i < jet_results.first.size(); i++) jets_undoJEC.push_back(jet_results.first.at(i).undo_jec());
  for (unsigned int i = 0; i < jet_results.second.size(); i++) {
      auto jet = jet_results.second.at(i).p4();
      auto disc = jet_results.second.at(i).disc();
      btags.push_back(jet);
      btags_flavor.push_back(jet_results.second.at(i).mcFlavor());
      btags_disc.push_back(disc);
      btags_JEC.push_back(jet_results.second.at(i).jec());
      btags_undoJEC.push_back(jet_results.second.at(i).undo_jec());
  }

  for (unsigned int i = 0; i < jet_results.first.size(); i++){
    jecUnc->setJetEta(jets[i].eta());
    jecUnc->setJetPt(jets[i].pt()*jets_undoJEC[i]*jets_JEC[i]);
    jets_unc.push_back(jecUnc->getUncertainty(true));
  }
  for (unsigned int i = 0; i < jet_results.second.size(); i++){
    jecUnc->setJetEta(btags[i].eta());
    jecUnc->setJetPt(btags[i].pt()*btags_undoJEC[i]*btags_JEC[i]);
    btags_unc.push_back(jecUnc->getUncertainty(true));
  }
  njets = jets.size();
  nbtags = btags.size();
  ht = 0;
  for (unsigned int i = 0; i < jets.size(); i++){
     ht += jets.at(i).pt()*jets_undoJEC.at(i)*jets_JEC.at(i);
  }

  pair <float, float> T1CHSMET = getT1CHSMET(jetCorr, NULL, 0);
  met = T1CHSMET.first;
  metPhi = T1CHSMET.second;
}

void leptons_calculator(){
  lep1_id = (tas::hyp_ll_p4().at(best_hyp).pt() > tas::hyp_lt_p4().at(best_hyp).pt()) ? tas::hyp_ll_id().at(best_hyp) : tas::hyp_lt_id().at(best_hyp);
  lep2_id = (tas::hyp_ll_p4().at(best_hyp).pt() <= tas::hyp_lt_p4().at(best_hyp).pt()) ? tas::hyp_ll_id().at(best_hyp) : tas::hyp_lt_id().at(best_hyp);
  lep1_idx = (tas::hyp_ll_p4().at(best_hyp).pt() > tas::hyp_lt_p4().at(best_hyp).pt()) ? tas::hyp_ll_index().at(best_hyp) : tas::hyp_lt_index().at(best_hyp);
  lep2_idx = (tas::hyp_ll_p4().at(best_hyp).pt() <= tas::hyp_lt_p4().at(best_hyp).pt()) ? tas::hyp_ll_index().at(best_hyp) : tas::hyp_lt_index().at(best_hyp);
  lep1_coneCorrPt = coneCorrPt(lep1_id, lep1_idx);
  lep2_coneCorrPt = coneCorrPt(lep2_id, lep2_idx);
}
