#!/usr/bin/env python

import os
from cmseostools import findDatasets
from FloatToString import FloatToStringMixedStyle


old_datasets = [
   ["/DYJetsToLL_M-10to50_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v2/MINIAODSIM", "DY_2l_M_10to50", 15800.],
   ["/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "DY_2l_M_50", 6225.4],

   ["/TTTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "TT_2l2nu", 831.76, 0.105],
   ["/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "TT_lnu_nlo", 831.76, 0.438],
   ["/TTJets_TuneCP5_13TeV-amcatnloFXFX-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TTJets"],
   ["/TTJets_SingleLeptFromT_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "TT_lnu_top_lo"],
   ["/TTJets_SingleLeptFromTbar_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "TT_lnu_antitop_lo"],
   ["/TT_DiLept_TuneCP5_13TeV-amcatnlo-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TT0J_2l2nu_nlo"],
   ["/TTPlus1Jet_DiLept_TuneCP5_13TeV-amcatnloFXFX-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TT1J_2l2nu_nlo"],

   ["/TTZToLL_M-1to10_TuneCP5_13TeV-amcatnlo-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "TTZ_2l2nu_M_1to10"],
   ["/TTZToLLNuNu_M-10_TuneCP5_13TeV-amcatnlo-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TTZ_2l2nu_M_10", 0.2432],
   ["/TTWJetsToLNu_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TTW_lnu", 0.2181],

   ["/TTGamma_SingleLeptFromT_TuneCP5_13TeV_madgraph_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TTG_lnu_top"],
   ["/TTGamma_SingleLeptFromTbar_TuneCP5_13TeV_madgraph_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TTG_lnu_antitop"],
   ["/TTGamma_Dilept_TuneCP5_13TeV_madgraph_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TTG_2l2nu"],
   ["/TTGJets_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "TTGJets", 3.746],

   ["/ttHToNonbb_M125_TuneCP5_13TeV-powheg-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v2/MINIAODSIM", "TTH_HToNonBB"],

   ["/TTTJ_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TTTJ"],
   ["/TTHH_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TTHH"],
   ["/TTWZ_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TTWZ"],
   ["/TTZZ_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TTZZ"],
   ["/TTTW_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TTTW"],
   ["/TTWH_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TTWH"],
   ["/TTZH_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TTZH"],
   ["/TTWW_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TTWW"],

   ["/ST_tW_top_5f_NoFullyHadronicDecays_TuneCP5_13TeV-powheg-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v3/MINIAODSIM", "ST_tW_top_5f_NoFullyHadronicDecays", 34.9],
   ["/ST_tW_antitop_5f_NoFullyHadronicDecays_TuneCP5_13TeV-powheg-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v3/MINIAODSIM","ST_tW_antitop_5f_NoFullyHadronicDecays", 34.9],
   ["/tZq_ll_4f_ckm_NLO_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TZ_2l_4f", 0.0758],
   ["/TGJets_leptonDecays_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "TGJets", 2.872],
   ["/ST_tWll_5f_LO_TuneCP5_PSweights_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v1/MINIAODSIM", "ST_tW2l_5f"],

   ["/WJetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v2/MINIAODSIM", "WJets_lnu_lo"],
   ["/WJetsToLNu_0J_TuneCP5_13TeV-amcatnloFXFX-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "WJets_lnu_0j", 49141.],
   ["/WJetsToLNu_1J_TuneCP5_13TeV-amcatnloFXFX-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "WJets_lnu_1j", 8045.1],
   ["/WJetsToLNu_2J_TuneCP5_13TeV-amcatnloFXFX-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "WJets_lnu_2j", 3159.9],
   ["/WJetsToLNu_HT-70To100_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "WJets_lnu_HT_70-100", 1304.],
   ["/WJetsToLNu_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "WJets_lnu_HT_100-200", 1677.06],
   ["/WJetsToLNu_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "WJets_lnu_HT_200-400", 494.406],
   ["/WJetsToLNu_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "WJets_lnu_HT_400-600", 70.1921],
   ["/WJetsToLNu_HT-600To800_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "WJets_lnu_HT_600-800", 17.4361],
   ["/WJetsToLNu_HT-800To1200_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "WJets_lnu_HT_800-1200", 7.80813],
   ["/WJetsToLNu_HT-1200To2500_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "WJets_lnu_HT_1200-2500", 1.73151],
   ["/WJetsToLNu_HT-2500ToInf_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "WJets_lnu_HT_2500-inf", 0.04063],

   ["/WW_DoubleScattering_13TeV-pythia8_TuneCP5/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v1/MINIAODSIM", "qqWW_2l2nu_DPS"],
   ["/WWTo2L2Nu_NNPDF31_TuneCP5_13TeV-powheg-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "qqWW_2l2nu", 11.08],
   ["/WpWpJJ_EWK-QCD_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "qqWpWp_2l2nu"],
   ["/WZTo3LNu_TuneCP5_13TeV-powheg-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "qqWZ_3lnu_POWHEG", 4.658],
   ["/WZTo3LNu_mllmin01_NNPDF31_TuneCP5_13TeV_powheg_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "qqWZ_3lnu_POWHEG_mll_0p1-inf", 62.168],
   ["/WZTo3LNu_TuneCP5_13TeV-amcatnloFXFX-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "qqWZ_3lnu_MG", 5.087],
   ["/ZZTo4L_TuneCP5_13TeV_powheg_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext2-v2/MINIAODSIM", "qqZZ_4l", 1.325],
   ["/ZGToLLG_01J_5f_TuneCP5_13TeV-amcatnloFXFX-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "qqZG_ll_01j"],
   ["/ZLLGJets_MonoPhoton_PtG-40to130_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v2/MINIAODSIM", "qqZG_ll_pTG_40-130_lo", 5.485],
   ["/ZLLGJets_MonoPhoton_PtG-130_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v2/MINIAODSIM", "qqZG_ll_pTG_130-inf_lo", 0.1472],
   ["/ZGTo2LG_PtG-130_TuneCP5_13TeV-amcatnloFXFX-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v2/MINIAODSIM", "qqZG_ll_pTG_130-inf_nlo", 0.1595],
   ["/WGToLNuG_01J_5f_TuneCP5_13TeV-amcatnloFXFX-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v1/MINIAODSIM", "qqWG_lnu_nlo_01j", 191.4],
   ["/WGToLNuG_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "qqWG_lnu_lo", 444.6],

   ["/WWW_4F_TuneCP5_13TeV-amcatnlo-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "WWW_4f", 0.2154],
   ["/WWG_TuneCP5_13TeV-amcatnlo-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "WWG"],
   ["/WWZ_TuneCP5_13TeV-amcatnlo-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "WWZ", 0.1675],
   ["/WZG_TuneCP5_13TeV-amcatnlo-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "WZG", 0.04123],
   ["/WZZ_TuneCP5_13TeV-amcatnlo-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "WZZ", 0.0571],
   ["/ZZZ_TuneCP5_13TeV-amcatnlo-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM", "ZZZ", 0.01473],

   ["/VHToNonbb_M125_13TeV_amcatnloFXFX_madspin_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v2/MINIAODSIM", "VH_HToNonbb"],
   ["/GluGluHToZZTo4L_M125_13TeV_powheg2_JHUGenV7011_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v2/MINIAODSIM", "GGH_ZZTo4L_POWHEG", 28.87, 0.0002745],

   ["/GluGluToContinToZZTo2e2mu_13TeV_MCFM701_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "ggZZTo2e2mu_Bkg_MCFM", 3.19142e-03],
   ["/GluGluToContinToZZTo2e2tau_13TeV_MCFM701_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "ggZZTo2e2tau_Bkg_MCFM", 3.19142e-03],
   ["/GluGluToContinToZZTo2mu2tau_13TeV_MCFM701_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "ggZZTo2mu2tau_Bkg_MCFM", 3.19142e-03],
   ["/GluGluToContinToZZTo4e_13TeV_MCFM701_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "ggZZTo4e_Bkg_MCFM", 1.58549e-03],
   ["/GluGluToContinToZZTo4mu_13TeV_MCFM701_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "ggZZTo4mu_Bkg_MCFM", 1.58549e-03],
   ["/GluGluToContinToZZTo4tau_13TeV_MCFM701_pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "ggZZTo4tau_Bkg_MCFM", 1.58549e-03],

   ["/TTTT_TuneCP5_13TeV-amcatnlo-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15_ext1-v2/MINIAODSIM","TTTT"],

   ["/TTTT_hhat_0p0_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "TTTT_HOblique_hhat_0p0"],
   ["/TTTT_hhat_0p08_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "TTTT_HOblique_hhat_0p08"],
   ["/TTTT_hhat_0p12_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "TTTT_HOblique_hhat_0p12"],
   ["/TTTT_hhat_0p16_TuneCP5_13TeV-madgraph-pythia8/RunIIAutumn18MiniAOD-102X_upgrade2018_realistic_v15-v1/MINIAODSIM", "TTTT_HOblique_hhat_0p16"],
]


if __name__ == "__main__":
   fxsec = open("scale1fbs.txt","r")
   dset_xsec_map = dict()
   for line in fxsec:
      line.lstrip()
      if line.startswith("#"): continue
      if "MINIAODSIM" not in line: continue
      lents = line.split()
      dset = lents[0]
      dset_xsec = lents[4]
      dset_xsec_map[dset] = dset_xsec # If there are multiple entries, map should automatically take the latest value
   tmp_datasets = []
   for sample_sname_pair in old_datasets:
      dset = sample_sname_pair[0]
      sname = sample_sname_pair[1]
      xsec = None
      BR = 1
      comment = ""
      if len(sample_sname_pair)>2:
         xsec = sample_sname_pair[2]
         if len(sample_sname_pair)>3:
            BR = sample_sname_pair[3]
      if xsec is not None:
         comment = "xsec = {} BR = {} from off-shell fwk".format(xsec, BR)
      elif dset in dset_xsec_map:
         xsec = float(dset_xsec_map[dset])
         comment = "xsec = {} BR = {} from scale1fbs.txt".format(xsec, BR)
      else:
         raise RuntimeWarning("Data set {} is not found in the xsec file.".format(dset))
         continue
      tmp_datasets.append([dset, sname, xsec, BR, comment])
   old_datasets = tmp_datasets
   fxsec.close()

   new_datasets = [ ["dataset", "short_name", "period", "xsec", "BR", "extra_arguments"] ]
   for sample_sname_pair in old_datasets:
      old_dset = sample_sname_pair[0]
      sname = sample_sname_pair[1]
      xsec = FloatToStringMixedStyle(sample_sname_pair[2])
      BR = FloatToStringMixedStyle(sample_sname_pair[3])
      comment = sample_sname_pair[4]

      print("Extracting the new data sets for {} (comment: '{}')".format(sname, comment))

      new_datasets.append([ "# "+sname, "data set for "+old_dset.split('/')[1], comment, "", "", "" ])

      dsets = findDatasets('/'+old_dset.split('/')[1]+"*/RunIISummer20UL16NanoAODAPVv9-106X_mcRun2*/NANOAODSIM")
      idset = -1
      for dset in dsets:
         if dset.strip() != "":
            idset += 1
         else:
            continue
         sname_eff = sname
         if idset==1:
            sname_eff = "{}_ext".format(sname)
         elif idset>1:
            sname_eff = "{}_ext{}".format(sname, idset)
         new_datasets.append([ dset, sname_eff, "2016_APV", xsec, BR, "" ])

      dsets = findDatasets('/'+old_dset.split('/')[1]+"*/RunIISummer20UL16NanoAODv9-106X_mcRun2*/NANOAODSIM")
      idset = -1
      for dset in dsets:
         if dset.strip() != "":
            idset += 1
         else:
            continue
         sname_eff = sname
         if idset==1:
            sname_eff = "{}_ext".format(sname)
         elif idset>1:
            sname_eff = "{}_ext{}".format(sname, idset)
         new_datasets.append([ dset, sname_eff, "2016_NonAPV", xsec, BR, "" ])

      dsets = findDatasets('/'+old_dset.split('/')[1]+"*/RunIISummer20UL17NanoAODv9-106X_mc2017*/NANOAODSIM")
      idset = -1
      for dset in dsets:
         if dset.strip() != "":
            idset += 1
         else:
            continue
         sname_eff = sname
         if idset==1:
            sname_eff = "{}_ext".format(sname)
         elif idset>1:
            sname_eff = "{}_ext{}".format(sname, idset)
         new_datasets.append([ dset, sname_eff, "2017", xsec, BR, "" ])

      dsets = findDatasets('/'+old_dset.split('/')[1]+"*/RunIISummer20UL18NanoAODv9-106X_upgrade2018*/NANOAODSIM")
      idset = -1
      for dset in dsets:
         if dset.strip() != "":
            idset += 1
         else:
            continue
         sname_eff = sname
         if idset==1:
            sname_eff = "{}_ext".format(sname)
         elif idset>1:
            sname_eff = "{}_ext{}".format(sname, idset)
         new_datasets.append([ dset, sname_eff, "2018", xsec, BR, "" ])

   fout = open("samples_MC.csv","w")
   for dd in new_datasets:
      fout.write(",".join(dd)+'\n')
   fout.close()



