/*! Definition of common uncertaintainties.
This file is part of https://github.com/cms-hh/StatAnalysis. */

#pragma once

#include "HHStatAnalysis/StatModels/interface/Uncertainty.h"

#define UNC(name, cor_range, distr_type, ...) \
    static const GlobalUncertainty& name() { \
        static const GlobalUncertainty u(#name, CorrelationRange::cor_range, \
                                         UncDistributionType::distr_type, ##__VA_ARGS__); \
        return u; \
    }

namespace hh_analysis {
namespace stat_models {
namespace Run2_2016 {

struct CommonUncertainties {

    // LHC uncertainties
    UNC(lumi, LHC, lnN, 0.026) // LUM-17-001
    UNC(QCDscale_W, LHC, lnN, 0.008, -0.004) // total unc. https://cms-gen-dev.cern.ch/xsdb/search1/process_name/WJetsToLNu
    UNC(QCDscale_WW, LHC, lnN, 0.04)
    UNC(QCDscale_WZ, LHC, lnN, 0.04)
    UNC(QCDscale_ZZ, LHC, lnN, 0.04)
    UNC(QCDscale_EWK, LHC, lnN, 0.04)
    UNC(QCDscale_ttbar, LHC, lnN, +0.048, -0.055) // scale & PDF+alpha_s https://twiki.cern.ch/twiki/bin/view/LHCPhysics/TtbarNNLO
    UNC(QCDscale_tW, LHC, lnN, 0.054) // scale & PDF https://twiki.cern.ch/twiki/bin/viewauth/CMS/SingleTopSigma
    UNC(QCDscale_ZH, LHC, lnN, 0.041, -0.035) // scale & PDF+alpha_s @ mH=125GeV https://twiki.cern.ch/twiki/bin/view/LHCPhysics/CERNYellowReportPageAt13TeV#ZH_Process
    UNC(QCDscale_ggHH, LHC, lnN, +0.043, -0.060) // scale @ mH=125GeV https://twiki.cern.ch/twiki/bin/view/LHCPhysics/LHCHXSWGHH
    UNC(pdf_ggHH, LHC, lnN, 0.059) // Th & PDF & alpha_s @ mH=125GeV https://twiki.cern.ch/twiki/bin/view/LHCPhysics/LHCHXSWGHH
    UNC(BR_SM_H_bb, LHC, lnN, 0.012, -0.013) // Th & m_q & alpha_s @ mH=125GeV https://twiki.cern.ch/twiki/bin/view/LHCPhysics/CERNYellowReportPageBR
    UNC(BR_SM_H_tautau, LHC, lnN, 0.016) // Th & m_q & alpha_s @ mH=125GeV https://twiki.cern.ch/twiki/bin/view/LHCPhysics/CERNYellowReportPageBR

    // CMS uncertainties
    UNC(scale_j, Experiment, shape)
    UNC(res_j, Experiment, shape)
//    UNC(scale_b, Experiment, lnN, 0.02)
    UNC(eff_b, Experiment, lnN)
    UNC(eff_e, Experiment, lnN, 0.01)
    UNC(eff_m, Experiment, lnN, 0.018)
    UNC(eff_t, Experiment, lnN, 0.05)
    UNC(scale_t, Experiment, shape)
    UNC(topPt, Experiment, shape)
};

} // namespace Run2_2016
} // namespace stat_models
} // namespace hh_analysis

#undef UNC
