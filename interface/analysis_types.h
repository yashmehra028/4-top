#ifndef ANALYSIS_TYPES_H
#define ANALYSIS_TYPES_H


#include "Math/LorentzVector.h"


#define SIMPLE_DATA_DIRECTIVES \
SIMPLE_DATA_DIRECTIVE(int, year, -1) \
SIMPLE_DATA_DIRECTIVE(int, run, -1) \
SIMPLE_DATA_DIRECTIVE(int, lumiBlock, -1) \
SIMPLE_DATA_DIRECTIVE(int, event, -1) \
SIMPLE_DATA_DIRECTIVE(int, is_fastsim, 0) \
SIMPLE_DATA_DIRECTIVE(int, rawmet, -1) \
SIMPLE_DATA_DIRECTIVE(int, rawmetPhi, -1) \
SIMPLE_DATA_DIRECTIVE(int, calomet, -1) \
SIMPLE_DATA_DIRECTIVE(int, calometPhi, -1) \
SIMPLE_DATA_DIRECTIVE(int, met, -1) \
SIMPLE_DATA_DIRECTIVE(int, metPhi, -1) \
SIMPLE_DATA_DIRECTIVE(int, lumi, -1) \
SIMPLE_DATA_DIRECTIVE(int, filt_csc, 0) \
SIMPLE_DATA_DIRECTIVE(int, filt_hbhe, 0) \
SIMPLE_DATA_DIRECTIVE(int, filt_hcallaser, 0) \
SIMPLE_DATA_DIRECTIVE(int, filt_ecaltp, 0) \
SIMPLE_DATA_DIRECTIVE(int, filt_trkfail, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep1_isPrompt, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep1_isDirectPrompt, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep1_isStat3, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_isPrompt, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_isDirectPrompt, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_isStat3, 0) \
SIMPLE_DATA_DIRECTIVE(int, is_real_data, 0) \
SIMPLE_DATA_DIRECTIVE(int, scale1fb, 1) \
SIMPLE_DATA_DIRECTIVE(int, genps_weight, 1) \
SIMPLE_DATA_DIRECTIVE(int, neventstotal, -1) \
SIMPLE_DATA_DIRECTIVE(int, kfactor, -1) \
SIMPLE_DATA_DIRECTIVE(int, gen_met, -1) \
SIMPLE_DATA_DIRECTIVE(int, gen_met_phi, -1) \
SIMPLE_DATA_DIRECTIVE(int, skim, 0) \
SIMPLE_DATA_DIRECTIVE(int, skim_nomet, 0) \
SIMPLE_DATA_DIRECTIVE(int, nleps, 0) \
SIMPLE_DATA_DIRECTIVE(int, sr, 0) \
SIMPLE_DATA_DIRECTIVE(int, br, 0) \
SIMPLE_DATA_DIRECTIVE(int, ss_nleps, -1) \
SIMPLE_DATA_DIRECTIVE(int, ss_br, 0) \
SIMPLE_DATA_DIRECTIVE(int, ss_sr, -1) \
SIMPLE_DATA_DIRECTIVE(int, ss_region, -1) \
SIMPLE_DATA_DIRECTIVE(int, njets, -1) \
SIMPLE_DATA_DIRECTIVE(int, njetsAG, -1) \
SIMPLE_DATA_DIRECTIVE(int, nbtagsAG, -1) \
SIMPLE_DATA_DIRECTIVE(int, njets_raw, -1) \
SIMPLE_DATA_DIRECTIVE(int, hyp_class, -1) \
SIMPLE_DATA_DIRECTIVE(int, ht, -1) \
SIMPLE_DATA_DIRECTIVE(int, ht_raw, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep1_motherID, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_motherID, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep1_mc_id, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep2_mc_id, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep1_mc_motherid, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep2_mc_motherid, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep1_id, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep2_id, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep1_coneCorrPt, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep2_coneCorrPt, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep3_coneCorrPt, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep4_coneCorrPt, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep1_idx, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep2_idx, -1) \
SIMPLE_DATA_DIRECTIVE(int, nbtags, -1) \
SIMPLE_DATA_DIRECTIVE(int, nbtags_raw, -1) \
SIMPLE_DATA_DIRECTIVE(int, sf_dilepTrig_hpt, -1) \
SIMPLE_DATA_DIRECTIVE(int, sf_dilepTrig_lpt, -1) \
SIMPLE_DATA_DIRECTIVE(int, sf_dilepTrig_vlpt, -1) \
SIMPLE_DATA_DIRECTIVE(int, hyp_type, -1) \
SIMPLE_DATA_DIRECTIVE(int, sf_dilep_eff, -1) \
SIMPLE_DATA_DIRECTIVE(int, mt, -1) \
SIMPLE_DATA_DIRECTIVE(int, mt_l2, -1) \
SIMPLE_DATA_DIRECTIVE(int, mt2, -1) \
SIMPLE_DATA_DIRECTIVE(int, xsec, -1) \
SIMPLE_DATA_DIRECTIVE(int, xsec_ps, -1) \
SIMPLE_DATA_DIRECTIVE(int, xsec_error, -1) \
SIMPLE_DATA_DIRECTIVE(int, mtmin, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep3_id, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep3_idx, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep4_id, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep4_idx, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep3_quality, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep3_mcid, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep3_mc_motherid, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep3_mc3idx, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep3_motherID, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep4_mcid, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep4_mcidx, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep1_iso, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep2_iso, -1) \
SIMPLE_DATA_DIRECTIVE(int, ncharginos, -1) \
SIMPLE_DATA_DIRECTIVE(int, nhiggs, -1) \
SIMPLE_DATA_DIRECTIVE(int, higgs_mass, -1) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_passes_id, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_passes_id, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_passes_id, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_passes_id, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_tight, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_veto, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_fo, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_tight, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_veto, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_fo, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_tight, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_veto, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_fo, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_tight, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_veto, false) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_fo, false) \
SIMPLE_DATA_DIRECTIVE(int, lep1_dxyPV, -999998) \
SIMPLE_DATA_DIRECTIVE(int, lep2_dxyPV, -999998) \
SIMPLE_DATA_DIRECTIVE(int, lep1_dZ, -999998) \
SIMPLE_DATA_DIRECTIVE(int, lep2_dZ, -999998) \
SIMPLE_DATA_DIRECTIVE(int, lep1_d0_err, -999998) \
SIMPLE_DATA_DIRECTIVE(int, lep2_d0_err, -999998) \
SIMPLE_DATA_DIRECTIVE(int, lep1_ip3d, -999998) \
SIMPLE_DATA_DIRECTIVE(int, lep2_ip3d, -999998) \
SIMPLE_DATA_DIRECTIVE(int, lep1_MVA, -999998) \
SIMPLE_DATA_DIRECTIVE(int, lep2_MVA, -999998) \
SIMPLE_DATA_DIRECTIVE(int, lep1_ip3d_err, -999998) \
SIMPLE_DATA_DIRECTIVE(int, lep2_ip3d_err, -999998) \
SIMPLE_DATA_DIRECTIVE(int, lep1_el_conv_vtx_flag, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep1_nPixelMiss, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep1_el_exp_innerlayers, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep1_tightCharge, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep1_mu_ptErr, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep1_el_threeChargeAgree, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_el_conv_vtx_flag, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_nPixelMiss, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_el_exp_innerlayers, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_tightCharge, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_mu_ptErr, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_el_threeChargeAgree, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep1_el_etaSC, -999) \
SIMPLE_DATA_DIRECTIVE(int, lep2_el_etaSC, -999) \
SIMPLE_DATA_DIRECTIVE(int, lep1_isTrigSafeNoIsov1, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep1_isTrigSafev1, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_isTrigSafeNoIsov1, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_isTrigSafev1, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep1_tightMuonPOG, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep1_mediumMuonPOG, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep1_looseMuonPOG, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_tightMuonPOG, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_mediumMuonPOG, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep2_looseMuonPOG, 0) \
SIMPLE_DATA_DIRECTIVE(int, lep3_mediumMuonPOG, 0) \
SIMPLE_DATA_DIRECTIVE(int, nVetoElectrons7, 0) \
SIMPLE_DATA_DIRECTIVE(int, nVetoElectrons10, 0) \
SIMPLE_DATA_DIRECTIVE(int, nVetoElectrons25, 0) \
SIMPLE_DATA_DIRECTIVE(int, nVetoMuons5, 0) \
SIMPLE_DATA_DIRECTIVE(int, nVetoMuons10, 0) \
SIMPLE_DATA_DIRECTIVE(int, nVetoMuons25, 0) \
SIMPLE_DATA_DIRECTIVE(string, filename, "") \
SIMPLE_DATA_DIRECTIVE(int, lep1_ptrel_ma, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep1_ptratio_ma, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep2_ptrel_ma, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep2_ptratio_ma, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep1_ptrel_v1, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep2_ptrel_v1, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep1_ptratio, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep2_ptratio, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep1_miniIso, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep2_miniIso, -1) \
SIMPLE_DATA_DIRECTIVE(LorentzVector, lep1_p4, LorentzVector(0,0,0,0)) \
SIMPLE_DATA_DIRECTIVE(LorentzVector, lep2_p4, LorentzVector(0,0,0,0)) \
SIMPLE_DATA_DIRECTIVE(LorentzVector, lep3_p4, LorentzVector(0,0,0,0)) \
SIMPLE_DATA_DIRECTIVE(LorentzVector, lep4_p4, LorentzVector(0,0,0,0)) \
SIMPLE_DATA_DIRECTIVE(LorentzVector, lep1_p4_gen, LorentzVector(0,0,0,0)) \
SIMPLE_DATA_DIRECTIVE(LorentzVector, lep2_p4_gen, LorentzVector(0,0,0,0)) \
SIMPLE_DATA_DIRECTIVE(LorentzVector, lep1_closeJet, LorentzVector(0,0,0,0)) \
SIMPLE_DATA_DIRECTIVE(LorentzVector, lep2_closeJet, LorentzVector(0,0,0,0)) \
SIMPLE_DATA_DIRECTIVE(LorentzVector, dilep_p4, LorentzVector(0,0,0,0)) \
SIMPLE_DATA_DIRECTIVE(float, dilep_mass, -1.) \
SIMPLE_DATA_DIRECTIVE(float, mass13, -1.) \
SIMPLE_DATA_DIRECTIVE(float, mass23, -1.) \
SIMPLE_DATA_DIRECTIVE(float, mass123, -1.) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_isGoodLeg, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_isGoodLeg, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_isFakeLeg, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_isFakeLeg, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_multiIso, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_multiIso, 0) \
SIMPLE_DATA_DIRECTIVE(float, lep1_sip, -1.) \
SIMPLE_DATA_DIRECTIVE(float, lep2_sip, -1.) \
SIMPLE_DATA_DIRECTIVE(bool, passed_id_inSituFR_lep1, 0) \
SIMPLE_DATA_DIRECTIVE(bool, passed_id_inSituFR_lep2, 0) \
SIMPLE_DATA_DIRECTIVE(int, nGoodVertices, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_trigMatch_noIsoReq, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_trigMatch_isoReq, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_trigMatch_noIsoReq, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_trigMatch_isoReq, 0) \
SIMPLE_DATA_DIRECTIVE(bool, fired_trigger, 0) \
SIMPLE_DATA_DIRECTIVE(bool, fired_trigger_ss, 0) \
SIMPLE_DATA_DIRECTIVE(unsigned int, triggers, 0) \
SIMPLE_DATA_DIRECTIVE(bool, passes_met_filters, 0) \
SIMPLE_DATA_DIRECTIVE(bool, passes_chargedcand_filter, 0) \
SIMPLE_DATA_DIRECTIVE(float, evt_egclean_pfmet, -1.0) \
SIMPLE_DATA_DIRECTIVE(float, evt_muegclean_pfmet, -1.0) \
SIMPLE_DATA_DIRECTIVE(float, evt_muegcleanfix_pfmet, -1.0) \
SIMPLE_DATA_DIRECTIVE(float, evt_uncorr_pfmet, -1.0) \
SIMPLE_DATA_DIRECTIVE(bool, filt_noBadMuons, 0) \
SIMPLE_DATA_DIRECTIVE(bool, filt_duplicateMuons, 0) \
SIMPLE_DATA_DIRECTIVE(bool, filt_badMuons, 0) \
SIMPLE_DATA_DIRECTIVE(bool, failsRA2Filter, 0) \
SIMPLE_DATA_DIRECTIVE(bool, madeExtraZ, 0) \
SIMPLE_DATA_DIRECTIVE(bool, madeExtraG, 0) \
SIMPLE_DATA_DIRECTIVE(int, nisrMatch, -1) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf1, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf1_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf1_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf2, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf2_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf2_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf3, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf3_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf3_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf4, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf4_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf4_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_light_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_light_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf1_light_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf1_light_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf2_light_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf2_light_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf3_light_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf3_light_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf4_light_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf4_light_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_heavy_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_heavy_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf1_heavy_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf1_heavy_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf2_heavy_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf2_heavy_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf3_heavy_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf3_heavy_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf4_heavy_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf4_heavy_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_central, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_down_cferr1, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_down_cferr2, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_down_hf, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_down_hfstats1, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_down_hfstats2, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_down_jes, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_down_lf, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_down_lfstats1, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_down_lfstats2, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_up_cferr1, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_up_cferr2, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_up_hf, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_up_hfstats1, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_up_hfstats2, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_up_jes, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_up_lf, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_up_lfstats1, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_btagsf_iter_up_lfstats2, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_isr, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_isr_dy, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_isr_tt, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_isr_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_isr_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_scale_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_scale_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_pdf_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_pdf_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_alphas_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_alphas_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_isrvar_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_isrvar_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_fsrvar_UP, -9999.) \
SIMPLE_DATA_DIRECTIVE(float, weight_fsrvar_DN, -9999.) \
SIMPLE_DATA_DIRECTIVE(int, njets_unc_up, 0) \
SIMPLE_DATA_DIRECTIVE(int, njets_unc_dn, 0) \
SIMPLE_DATA_DIRECTIVE(int, ht_unc_up, 0) \
SIMPLE_DATA_DIRECTIVE(int, ht_unc_dn, 0) \
SIMPLE_DATA_DIRECTIVE(int, nbtags_unc_up, 0) \
SIMPLE_DATA_DIRECTIVE(int, nbtags_unc_dn, 0) \
SIMPLE_DATA_DIRECTIVE(int, met_unc_up, 0) \
SIMPLE_DATA_DIRECTIVE(int, met_unc_dn, 0) \
SIMPLE_DATA_DIRECTIVE(int, metPhi_unc_up, 0) \
SIMPLE_DATA_DIRECTIVE(int, metPhi_unc_dn, 0) \
SIMPLE_DATA_DIRECTIVE(int, njets_JER_up, 0) \
SIMPLE_DATA_DIRECTIVE(int, njets_JER_dn, 0) \
SIMPLE_DATA_DIRECTIVE(float, ht_JER_up, 0) \
SIMPLE_DATA_DIRECTIVE(float, ht_JER_dn, 0) \
SIMPLE_DATA_DIRECTIVE(float, nbtags_JER_up, 0) \
SIMPLE_DATA_DIRECTIVE(float, nbtags_JER_dn, 0) \
SIMPLE_DATA_DIRECTIVE(float, met_JER_up, 0) \
SIMPLE_DATA_DIRECTIVE(float, met_JER_dn, 0) \
SIMPLE_DATA_DIRECTIVE(float, metPhi_JER_up, 0) \
SIMPLE_DATA_DIRECTIVE(float, metPhi_JER_dn, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_genps_isHardProcess, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_genps_fromHardProcessFinalState, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_genps_fromHardProcessDecayed, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_genps_isDirectHardProcessTauDecayProductFinalState, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_genps_fromHardProcessBeforeFSR, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_genps_isLastCopy, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep1_genps_isLastCopyBeforeFSR, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_genps_isHardProcess, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_genps_fromHardProcessFinalState, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_genps_fromHardProcessDecayed, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_genps_isDirectHardProcessTauDecayProductFinalState, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_genps_fromHardProcessBeforeFSR, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_genps_isLastCopy, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep2_genps_isLastCopyBeforeFSR, 0) \
SIMPLE_DATA_DIRECTIVE(float, glglpt, -1.) \
SIMPLE_DATA_DIRECTIVE(float, isr_unc, -1.) \
SIMPLE_DATA_DIRECTIVE(int, lep1_mc3idx, -1) \
SIMPLE_DATA_DIRECTIVE(int, lep2_mc3idx, -1) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_el_etaSC, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_el_conv_vtx_flag, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_el_exp_innerlayers, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_el_threeChargeAgree, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_el_dxyPV, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_el_ip3d, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_el_dzPV, 0) \
SIMPLE_DATA_DIRECTIVE(float, lep3_el_MVA_value, 0.) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_el_MVA, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_passes_RA5, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_mu_dxyPV, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_mu_ip3d, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_mu_dzPV, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_mu_ptErr, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_el_etaSC, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_el_conv_vtx_flag, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_el_exp_innerlayers, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_el_threeChargeAgree, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_el_dxyPV, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_el_ip3d, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_el_dzPV, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_el_MVA_value, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_el_MVA, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_mu_dxyPV, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_mu_ip3d, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_mu_dzPV, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_mu_ptErr, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_isTrigSafeNoIsov1, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep3_isTrigSafev1, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_isTrigSafeNoIsov1, 0) \
SIMPLE_DATA_DIRECTIVE(bool, lep4_isTrigSafev1, 0) \
SIMPLE_DATA_DIRECTIVE(int, extragenb, 0) \
SIMPLE_DATA_DIRECTIVE(bool, hasgammatoll, false) \
SIMPLE_DATA_DIRECTIVE(bool, gammatollmomemu, false) \
SIMPLE_DATA_DIRECTIVE(LorentzVector, gammatoll1, LorentzVector(0,0,0,0)) \
SIMPLE_DATA_DIRECTIVE(LorentzVector, gammatoll2, LorentzVector(0,0,0,0)) \
SIMPLE_DATA_DIRECTIVE(float, gammatolldr, -1.) \
SIMPLE_DATA_DIRECTIVE(int, extragenc, 0) \
SIMPLE_DATA_DIRECTIVE(int, ngenjets, 0) \
SIMPLE_DATA_DIRECTIVE(int, genht, 0) \
SIMPLE_DATA_DIRECTIVE(int, ngenjets30, 0) \
SIMPLE_DATA_DIRECTIVE(int, passfilter, 0) \
SIMPLE_DATA_DIRECTIVE(int, genht30, 0) \
SIMPLE_DATA_DIRECTIVE(int, ndrlt0p4, 0) \
SIMPLE_DATA_DIRECTIVE(int, gengood, 0) \
SIMPLE_DATA_DIRECTIVE(int, nleptonic, 0) \
SIMPLE_DATA_DIRECTIVE(int, ntau, 0) \
SIMPLE_DATA_DIRECTIVE(int, nleptonicW, 0) \
SIMPLE_DATA_DIRECTIVE(int, mfourtop, 0) \
SIMPLE_DATA_DIRECTIVE(int, nhadronicW, 0) \
SIMPLE_DATA_DIRECTIVE(int, nW, 0) \
SIMPLE_DATA_DIRECTIVE(float, leptonicWSF, 0.) \
SIMPLE_DATA_DIRECTIVE(float, hadronicWSF, 0.) \
SIMPLE_DATA_DIRECTIVE(float, decayWSF, 0.) \
SIMPLE_DATA_DIRECTIVE(int, njincone1, 0) \
SIMPLE_DATA_DIRECTIVE(int, njincone2, 0) \
SIMPLE_DATA_DIRECTIVE(float, prefire_sf, 1.) \
SIMPLE_DATA_DIRECTIVE(float, prefire_sfup, 1.) \
SIMPLE_DATA_DIRECTIVE(float, prefire_sfdown, 1.) \
SIMPLE_DATA_DIRECTIVE(float, prefire2016_sf, 1.) \
SIMPLE_DATA_DIRECTIVE(float, prefire2016_sfup, 1.) \
SIMPLE_DATA_DIRECTIVE(float, prefire2016_sfdown, 1.) \
SIMPLE_DATA_DIRECTIVE(float, prefire2017_sf, 1.) \
SIMPLE_DATA_DIRECTIVE(float, prefire2017_sfup, 1.) \
SIMPLE_DATA_DIRECTIVE(float, prefire2017_sfdown, 1.) \
SIMPLE_DATA_DIRECTIVE(float, prefire2017ele_sf, 1.) \
SIMPLE_DATA_DIRECTIVE(float, prefire2017ele_sfup, 1.) \
SIMPLE_DATA_DIRECTIVE(float, prefire2017ele_sfdown, 1.) \
SIMPLE_DATA_DIRECTIVE(int, bdt_nforwardjets20, 0) \
SIMPLE_DATA_DIRECTIVE(float, bdt_avgcdisc, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_nbtags, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_njets, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_met, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_ptl2, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_nlb40, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_ntb40, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_nleps, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_htb, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_ml1j1, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_dphil1l2, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_maxmjoverpt, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_detal1l2, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_q1, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_ptj1, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_ptj6, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_ptj7, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_ptj8, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_ptl1, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_ptl3, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jec_up_nbtags, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jec_dn_nbtags, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jer_up_nbtags, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jer_dn_nbtags, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jec_up_njets, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jec_dn_njets, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jer_up_njets, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jer_dn_njets, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jec_up_met, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jec_dn_met, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jer_up_met, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jer_dn_met, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jec_up_htb, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jec_dn_htb, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jer_up_htb, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jer_dn_htb, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jec_up_nlb40, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jec_dn_nlb40, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jer_up_nlb40, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jer_dn_nlb40, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jec_up_ntb40, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jec_dn_ntb40, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jer_up_ntb40, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_jer_dn_ntb40, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_disc, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_disc_jec_up, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_disc_jer_up, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_disc_jec_dn, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_disc_jer_dn, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_run2_disc, 0.) \
SIMPLE_DATA_DIRECTIVE(int, bdt_run2_SRDISC, -1) \
SIMPLE_DATA_DIRECTIVE(float, bdt_run2_disc_jec_up, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_run2_disc_jer_up, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_run2_disc_jec_dn, 0.) \
SIMPLE_DATA_DIRECTIVE(float, bdt_run2_disc_jer_dn, 0.) \
SIMPLE_DATA_DIRECTIVE(float, yearlumi, 0.) \
SIMPLE_DATA_DIRECTIVE(float, weight_lepsf1, 0.) \
SIMPLE_DATA_DIRECTIVE(float, weight_lepsf2, 0.) \
SIMPLE_DATA_DIRECTIVE(float, weight_lepsf3, 0.) \
SIMPLE_DATA_DIRECTIVE(float, weight_lepsf, 0.) \
SIMPLE_DATA_DIRECTIVE(float, weight_pu, 0.) \
SIMPLE_DATA_DIRECTIVE(float, weight_triggersf, 0.) \
SIMPLE_DATA_DIRECTIVE(float, weight_isrsf, 0.) \
SIMPLE_DATA_DIRECTIVE(float, weight, 0.) \
SIMPLE_DATA_DIRECTIVE(float, lep1_pt, -1.) \
SIMPLE_DATA_DIRECTIVE(float, lep2_pt, -1.) \
SIMPLE_DATA_DIRECTIVE(float, lep3_pt, -1.) \
SIMPLE_DATA_DIRECTIVE(float, lep1_eta, -1.) \
SIMPLE_DATA_DIRECTIVE(float, lep2_eta, -1.) \
SIMPLE_DATA_DIRECTIVE(float, lep3_eta, -1.) \
SIMPLE_DATA_DIRECTIVE(float, lep1_phi, -1.) \
SIMPLE_DATA_DIRECTIVE(float, lep2_phi, -1.) \
SIMPLE_DATA_DIRECTIVE(float, lep3_phi, -1.) 


#define VECTOR_DATA_DIRECTIVES 


#endif