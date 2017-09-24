# This README.md describes directory structure of this submission. 
# The repo is for ECE511 - HW Assignment 2
# BRanch Predictor evaluation was carried out for various types of branch predictor and benchmarks are evaluated. 
# Results are also discussed.

├── configs                         #Mofifications done in O3_ARM_v7a.py file found in configs/common/cores/arm/ folder. 
│   └── O3_ARM_v7a.py
├── diagrams                        # Diagrams file used for creating report. 
│   ├── assignment2
│   └── readme.txt
├── docs                            # Report and analysis document. Further, documents used to understand different types of branch predictor. 
│   ├── analysis.xlsx
│   ├── assignment2 (1).pdf
│   ├── Bimodal predictor.pdf
│   ├── branchprediction_explanation.pdf
│   ├── perceptorn.pdf
│   ├── report.pdf
│   ├── SMT Branch Predictor.pdf
│   └── yags.pdf
├── README.md
├── results                         # All results of branch predictor scheme has been added here. With detailed naming convention used which are self explanatory. 
│   ├── 2bit
│   │   ├── 2bit_libquantum
│   │   │   ├── config.dot
│   │   │   ├── config.dot.pdf
│   │   │   ├── config.dot.svg
│   │   │   ├── config.ini
│   │   │   ├── config.json
│   │   │   └── stats.txt
│   │   └── 2bit_soplex
│   │       ├── config.dot
│   │       ├── config.dot.pdf
│   │       ├── config.dot.svg
│   │       ├── config.ini
│   │       ├── config.json
│   │       └── stats.txt
│   ├── bimode
│   │   ├── bimode_libquantum
│   │   │   ├── config.dot
│   │   │   ├── config.dot.pdf
│   │   │   ├── config.dot.svg
│   │   │   ├── config.ini
│   │   │   ├── config.json
│   │   │   └── stats.txt
│   │   └── bimode_soplex
│   │       ├── config.dot
│   │       ├── config.dot.pdf
│   │       ├── config.dot.svg
│   │       ├── config.ini
│   │       ├── config.json
│   │       └── stats.txt
│   ├── gshare
│   │   ├── 2048                                     #PHT Size
│   │   │   ├── gshare_libquantam_hashfunc2
│   │   │   │   ├── config.dot
│   │   │   │   ├── config.dot.pdf
│   │   │   │   ├── config.dot.svg
│   │   │   │   ├── config.ini
│   │   │   │   ├── config.json
│   │   │   │   └── stats.txt
│   │   │   ├── gshare_libquantum_hashfunc1
│   │   │   │   ├── config.dot
│   │   │   │   ├── config.dot.pdf
│   │   │   │   ├── config.dot.svg
│   │   │   │   ├── config.ini
│   │   │   │   ├── config.json
│   │   │   │   └── stats.txt
│   │   │   ├── gshare_soplex_hashfunc1
│   │   │   │   ├── config.dot
│   │   │   │   ├── config.dot.pdf
│   │   │   │   ├── config.dot.svg
│   │   │   │   ├── config.ini
│   │   │   │   ├── config.json
│   │   │   │   └── stats.txt
│   │   │   └── gshare_soplex_hashfunc2
│   │   │       ├── config.dot
│   │   │       ├── config.dot.pdf
│   │   │       ├── config.dot.svg
│   │   │       ├── config.ini
│   │   │       ├── config.json
│   │   │       └── stats.txt
│   │   └── 4096                                      #PHT Size
│   │       ├── gshare_libquantum_hashfun1
│   │       │   ├── config.dot
│   │       │   ├── config.dot.pdf
│   │       │   ├── config.dot.svg
│   │       │   ├── config.ini
│   │       │   ├── config.json
│   │       │   └── stats.txt
│   │       ├── gshare_libquantum_hashfun2
│   │       │   ├── config.dot
│   │       │   ├── config.dot.pdf
│   │       │   ├── config.dot.svg
│   │       │   ├── config.ini
│   │       │   ├── config.json
│   │       │   └── stats.txt
│   │       ├── gshare_soplex_hashfun1
│   │       │   ├── config.dot
│   │       │   ├── config.dot.pdf
│   │       │   ├── config.dot.svg
│   │       │   ├── config.ini
│   │       │   ├── config.json
│   │       │   └── stats.txt
│   │       └── gshare_soplex_hashfun2
│   │           ├── config.dot
│   │           ├── config.dot.pdf
│   │           ├── config.dot.svg
│   │           ├── config.ini
│   │           ├── config.json
│   │           └── stats.txt
│   ├── perceptron
│   │   ├── 12                                    #HistoryLength
│   │   │   ├── perceptron12_libquantam
│   │   │   │   ├── config.dot
│   │   │   │   ├── config.dot.pdf
│   │   │   │   ├── config.dot.svg
│   │   │   │   ├── config.ini
│   │   │   │   ├── config.json
│   │   │   │   └── stats.txt
│   │   │   └── perceptron12_soplex
│   │   │       ├── config.dot
│   │   │       ├── config.dot.pdf
│   │   │       ├── config.dot.svg
│   │   │       ├── config.ini
│   │   │       ├── config.json
│   │   │       └── stats.txt
│   │   ├── 24                                    #HistoryLength
│   │   │   ├── perceptron24_libquantam
│   │   │   │   ├── config.dot
│   │   │   │   ├── config.dot.pdf
│   │   │   │   ├── config.dot.svg
│   │   │   │   ├── config.ini
│   │   │   │   ├── config.json
│   │   │   │   └── stats.txt
│   │   │   └── perceptron24_soplex
│   │   │       ├── config.dot
│   │   │       ├── config.dot.pdf
│   │   │       ├── config.dot.svg
│   │   │       ├── config.ini
│   │   │       ├── config.json
│   │   │       └── stats.txt
│   │   ├── 48                                     #HistoryLength
│   │   │   ├── perceptron48_libquantam
│   │   │   │   ├── config.dot
│   │   │   │   ├── config.dot.pdf
│   │   │   │   ├── config.dot.svg
│   │   │   │   ├── config.ini
│   │   │   │   ├── config.json
│   │   │   │   └── stats.txt
│   │   │   └── perceptron48_soplex
│   │   │       ├── config.dot
│   │   │       ├── config.dot.pdf
│   │   │       ├── config.dot.svg
│   │   │       ├── config.ini
│   │   │       ├── config.json
│   │   │       └── stats.txt
│   │   └── theta                                  #Experiments keeping historyLength constant with different theta.
│   │       ├── perceptron48_theta128_libquantam
│   │       │   ├── config.dot
│   │       │   ├── config.dot.pdf
│   │       │   ├── config.dot.svg
│   │       │   ├── config.ini
│   │       │   ├── config.json
│   │       │   └── stats.txt
│   │       ├── perceptron48_theta128_soplex
│   │       │   ├── config.dot
│   │       │   ├── config.dot.pdf
│   │       │   ├── config.dot.svg
│   │       │   ├── config.ini
│   │       │   ├── config.json
│   │       │   └── stats.txt
│   │       ├── perceptron48_theta32_libquantam
│   │       │   ├── config.dot
│   │       │   ├── config.dot.pdf
│   │       │   ├── config.dot.svg
│   │       │   ├── config.ini
│   │       │   ├── config.json
│   │       │   └── stats.txt
│   │       └── perceptron48_theta32_soplex
│   │           ├── config.dot
│   │           ├── config.dot.pdf
│   │           ├── config.dot.svg
│   │           ├── config.ini
│   │           ├── config.json
│   │           └── stats.txt
│   ├── tournament
│   │   ├── tournament_libquantum
│   │   │   ├── config.dot
│   │   │   ├── config.dot.pdf
│   │   │   ├── config.dot.svg
│   │   │   ├── config.ini
│   │   │   ├── config.json
│   │   │   └── stats.txt
│   │   └── tournament_soplex
│   │       ├── config.dot
│   │       ├── config.dot.pdf
│   │       ├── config.dot.svg
│   │       ├── config.ini
│   │       ├── config.json
│   │       └── stats.txt
│   └── yags
│       ├── 2048                              #PHT Size #Cache Size. 
│       │   ├── yags_2048_1024_libquantum
│       │   │   ├── config.dot
│       │   │   ├── config.dot.pdf
│       │   │   ├── config.dot.svg
│       │   │   ├── config.ini
│       │   │   ├── config.json
│       │   │   └── stats.txt
│       │   ├── yags_2048_1024_soplex
│       │   │   ├── config.dot
│       │   │   ├── config.dot.pdf
│       │   │   ├── config.dot.svg
│       │   │   ├── config.ini
│       │   │   ├── config.json
│       │   │   └── stats.txt
│       │   ├── yags_2048_2048_libquantum
│       │   │   ├── config.dot
│       │   │   ├── config.dot.pdf
│       │   │   ├── config.dot.svg
│       │   │   ├── config.ini
│       │   │   ├── config.json
│       │   │   └── stats.txt
│       │   └── yags_2048_2048_soplex
│       │       ├── config.dot
│       │       ├── config.dot.pdf
│       │       ├── config.dot.svg
│       │       ├── config.ini
│       │       ├── config.json
│       │       └── stats.txt
│       └── 4096                              #PHT Size #Cache Size.
│           ├── yags_4096_1024_libquantum
│           │   ├── config.dot
│           │   ├── config.dot.pdf
│           │   ├── config.dot.svg
│           │   ├── config.ini
│           │   ├── config.json
│           │   └── stats.txt
│           ├── yags_4096_1024_soplex
│           │   ├── config.dot
│           │   ├── config.dot.pdf
│           │   ├── config.dot.svg
│           │   ├── config.ini
│           │   ├── config.json
│           │   └── stats.txt
│           ├── yags_4096_2048_libquantum
│           │   ├── config.dot
│           │   ├── config.dot.pdf
│           │   ├── config.dot.svg
│           │   ├── config.ini
│           │   ├── config.json
│           │   └── stats.txt
│           ├── yags_4096_2048_soplex
│           │   ├── config.dot
│           │   ├── config.dot.pdf
│           │   ├── config.dot.svg
│           │   ├── config.ini
│           │   ├── config.json
│           │   └── stats.txt
│           ├── yags_4096_512_libquantum
│           │   ├── config.dot
│           │   ├── config.dot.pdf
│           │   ├── config.dot.svg
│           │   ├── config.ini
│           │   ├── config.json
│           │   └── stats.txt
│           └── yags_4096_512_soplex
│               ├── config.dot
│               ├── config.dot.pdf
│               ├── config.dot.svg
│               ├── config.ini
│               ├── config.json
│               └── stats.txt
└── src                                         #Source code of different branch predictors. 
    ├── BranchPredictor.py
    ├── gshare.cc
    ├── gshare.cc_bk
    ├── gshare.hh
    ├── gshare.hh_bk
    ├── perceptron.cc
    ├── perceptron.hh
    ├── yags.cc
    └── yags.hh

53 directories, 225 files
