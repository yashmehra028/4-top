# dataset,short_name,period,xsec,BR,extra_arguments

# TODO: How do I check the xsec are correct? all from this file.. https://github.com/cmstas/NanoTools/blob/master/NanoCORE/datasetinfo/scale1fbs.txt
# currently I just find the closest sample and assume it is the same.. 

# signal
tttt = \
[\
    # http://uaf-8.t2.ucsd.edu/~jguiang/dis/?query=%2FTTTT_TuneCP5_13TeV-amcatnlo-pythia8%2F*UL*v2%2FNANOAODSIM&type=basic&short=short
    ["/TTTT_TuneCP5_13TeV-amcatnlo-pythia8/RunIISummer20UL16NanoAODAPVv9-106X_mcRun2_asymptotic_preVFP_v11-v2/NANOAODSIM","tttt","2016_APV",8.091E-03,1,None],\
    ["/TTTT_TuneCP5_13TeV-amcatnlo-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v2/NANOAODSIM","tttt","2016_NonAPV",8.091E-03,1,None],\
    ["/TTTT_TuneCP5_13TeV-amcatnlo-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM","tttt","2017",8.091E-03,1,None],\
    ["/TTTT_TuneCP5_13TeV-amcatnlo-pythia8/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v2/NANOAODSIM","tttt",'2018',8.091E-03,1,None]\
]

# main backgrounds 

ttW = \
[]

ttZ = \
[\

]

ttH = \
[\
    # ttHH
    # http://uaf-8.t2.ucsd.edu/~jguiang/dis/?query=%2FTTHH_TuneCP5_13TeV-madgraph-pythia8%2F*UL*v2%2FNANOAODSIM&type=basic&short=short
    ["/TTHH_TuneCP5_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODAPVv9-106X_mcRun2_asymptotic_preVFP_v11-v2/NANOAODSIM", "ttHH", "2016_APV",3.7988E-06,1,None],\
    ["/TTHH_TuneCP5_13TeV-madgraph-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v2/NANOAODSIM", "ttHH", "2016_NonAPV",3.7988E-06,1,None],\
    ["/TTHH_TuneCP5_13TeV-madgraph-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM", "ttHH", "2017",3.7988E-06,1,None],\
    ["/TTHH_TuneCP5_13TeV-madgraph-pythia8/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v2/NANOAODSIM", "ttHH", "2018",3.7988E-06,1,None],\

    # ttH To non bb
    # http://uaf-8.t2.ucsd.edu/~jguiang/dis/?query=%2FttHToNonbb_M125_TuneCP5_13TeV-powheg-pythia8%2F*UL*v2%2FNANOAODSIM&type=basic&short=short
    ["/ttHToNonbb_M125_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL16NanoAODAPVv9-106X_mcRun2_asymptotic_preVFP_v11-v2/NANOAODSIM", "ttHToNonbb","2016_APV",2.9179E-05,1,None],\
    ["/ttHToNonbb_M125_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL16NanoAODv9-106X_mcRun2_asymptotic_v17-v2/NANOAODSIM", "ttHToNonbb","2016_APV",2.9179E-05,1,None],\
    ["/ttHToNonbb_M125_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM", "ttHToNonbb","2016_APV",2.9179E-05,1,None],\
    ["/ttHToNonbb_M125_TuneCP5_13TeV-powheg-pythia8/RunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v2/NANOAODSIM", "ttHToNonbb","2016_APV",2.9179E-05,1,None]

]

# TODO: what's up with ttVV?
#ttVV = \
#[]


