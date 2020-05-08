/*! Common part of stat models for the hh->bbtautau analyses.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#include <iostream>
#include <tuple>
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/CardWriter.h"
#include "CombineHarvester/CombinePdfs/interface/MorphFunctions.h"
#include "HHStatAnalysis/Run2/interface/ttbb.h"
#include "HHStatAnalysis/Core/interface/exception.h"
#include "HHStatAnalysis/Core/interface/Tools.h"
#include "HHStatAnalysis/Core/interface/TextIO.h"
#include "HHStatAnalysis/Core/interface/RootExt.h"
#include "HHStatAnalysis/Run2/interface/CommonUncertainties.h"
#include "HHStatAnalysis/StatModels/interface/StatTools.h"

namespace hh_analysis {
namespace stat_models {
namespace Run2 {

const StatModel::v_str ttbb_base::ana_name = { "hh_ttbb" };
const StatModel::v_str ttbb_base::eras = { "13TeV" };

const std::string ttbb_base::bkg_TT = "TT";
const std::string ttbb_base::bkg_W = "W";
const std::string ttbb_base::bkg_QCD = "QCD";
const std::string ttbb_base::bkg_VH = "VH";
const std::string ttbb_base::bkg_ttH = "ttH";
const std::string ttbb_base::bkg_H = "H";
const std::string ttbb_base::bkg_EWK = "EWK";
const std::string ttbb_base::bkg_ST = "ST";
const std::string ttbb_base::bkg_VVV = "VVV";
const std::string ttbb_base::bkg_ttV = "ttV";
const std::string ttbb_base::bkg_ttVV = "ttVV";
const std::string ttbb_base::bkg_VV = "VV";
const std::string ttbb_base::bkg_DY_M_10_50 = "DY_M_10_50";

const std::string ttbb_base::bkg_DY_0b_0JPt = "DY_0b_0JPt";
const std::string ttbb_base::bkg_DY_0b_10JPt = "DY_0b_10JPt";
const std::string ttbb_base::bkg_DY_0b_30JPt = "DY_0b_30JPt";
const std::string ttbb_base::bkg_DY_0b_50JPt = "DY_0b_50JPt";
const std::string ttbb_base::bkg_DY_0b_100JPt = "DY_0b_100JPt";
const std::string ttbb_base::bkg_DY_0b_200JPt = "DY_0b_200JPt";
const std::string ttbb_base::bkg_DY_1b_0JPt = "DY_1b_0JPt";
const std::string ttbb_base::bkg_DY_1b_10JPt = "DY_1b_10JPt";
const std::string ttbb_base::bkg_DY_1b_30JPt = "DY_1b_30JPt";
const std::string ttbb_base::bkg_DY_1b_50JPt = "DY_1b_50JPt";
const std::string ttbb_base::bkg_DY_1b_100JPt = "DY_1b_100JPt";
const std::string ttbb_base::bkg_DY_1b_200JPt = "DY_1b_200JPt";
const std::string ttbb_base::bkg_DY_2b_0JPt = "DY_2b_0JPt";
const std::string ttbb_base::bkg_DY_2b_10JPt = "DY_2b_10JPt";
const std::string ttbb_base::bkg_DY_2b_30JPt = "DY_2b_30JPt";
const std::string ttbb_base::bkg_DY_2b_50JPt = "DY_2b_50JPt";
const std::string ttbb_base::bkg_DY_2b_100JPt = "DY_2b_100JPt";
const std::string ttbb_base::bkg_DY_2b_200JPt = "DY_2b_200JPt";

const StatModel::v_str ttbb_base::bkg_DY_0b = {
    bkg_DY_0b_0JPt, bkg_DY_0b_10JPt, bkg_DY_0b_30JPt, bkg_DY_0b_50JPt, bkg_DY_0b_100JPt, bkg_DY_0b_200JPt,
};

const StatModel::v_str ttbb_base::bkg_DY_1b = {
    bkg_DY_1b_0JPt, bkg_DY_1b_10JPt, bkg_DY_1b_30JPt, bkg_DY_1b_50JPt, bkg_DY_1b_100JPt, bkg_DY_1b_200JPt,
};

const StatModel::v_str ttbb_base::bkg_DY_2b = {
    bkg_DY_2b_0JPt, bkg_DY_2b_10JPt, bkg_DY_2b_30JPt, bkg_DY_2b_50JPt, bkg_DY_2b_100JPt, bkg_DY_2b_200JPt,
};

const StatModel::v_str ttbb_base::bkg_DY = analysis::tools::join(bkg_DY_0b, bkg_DY_1b, bkg_DY_2b);

const StatModel::v_str ttbb_base::bkg_pure_MC = analysis::tools::join(bkg_W, bkg_VH, bkg_ttH, bkg_H, bkg_EWK,
                                                                      bkg_ST, bkg_VVV, bkg_ttV, bkg_ttVV,
                                                                      bkg_VV, bkg_DY_M_10_50);
const StatModel::v_str ttbb_base::bkg_MC = analysis::tools::join(bkg_pure_MC, bkg_DY, bkg_TT);
const StatModel::v_str ttbb_base::bkg_all = analysis::tools::join(bkg_MC, bkg_QCD);
// const StatModel::v_str ttbb_base::shape_suffixes = { "", "_CMS_scale_t_13TeVUp", "_CMS_scale_t_13TeVDown",
//                                                      "_CMS_scale_j_13TeVUp", "_CMS_scale_j_13TeVDown"};
const StatModel::v_str ttbb_base::shape_suffixes = { "" };


ttbb_base::ttbb_base(const StatModelDescriptor& _desc, const std::string& input_file_name) :
    StatModel(_desc, input_file_name), signal_processes({ desc.signal_process }),
    all_mc_processes(ch::JoinStr({ signal_processes, bkg_MC })),
    all_processes(ch::JoinStr({ signal_processes, bkg_all }))
{
}

void ttbb_base::AddProcesses(ch::CombineHarvester& cb)
{
    for(const auto& channel : desc.channels) {
        const auto& ch_all_categories = GetChannelCategories(channel);
        for(size_t n = 0; n < desc.categories.size(); ++n) {
            const auto& category = desc.categories.at(n);
            const std::string bin_name = ShapeNameRule::BinName(channel, category);
            ch::Categories ch_categories;
            ch_categories.push_back({n, bin_name});
            cb.AddObservations(wildcard, ana_name, eras, {channel}, ch_categories);
            cb.AddProcesses(desc.signal_points, ana_name, eras, {channel}, signal_processes,
                                   ch_categories, true);
            for(const auto& bkg : bkg_all) {
                bool all_ok = true;
                for(const auto& suffix : shape_suffixes) {
                    if(suffix.size() && bkg == bkg_QCD) continue;
                    const auto yield = GetBackgroundYield(bkg + suffix, channel, category);
                    if(!yield || yield->value < 1.1e-9) {
                        all_ok = false;
                        break;
                    }
                }
                if(all_ok)
                    cb.AddProcesses(wildcard, ana_name, eras, {channel}, {bkg}, ch_categories, false);
            }
        }
    }
}

void ttbb_base::AddSystematics(ch::CombineHarvester& cb)
{
    using ch::syst::SystMap;
    using CU = CommonUncertainties;
    static constexpr double unc_thr = 0.005;

    CU::lumi().ApplyGlobal(cb, signal_processes, bkg_pure_MC, bkg_TT);
    CU::QCDscale_W().ApplyGlobal(cb, bkg_W);
    CU::QCDscale_WW().ApplyGlobal(cb, bkg_VV);
    CU::QCDscale_WZ().ApplyGlobal(cb, bkg_VV);
    CU::QCDscale_ZZ().ApplyGlobal(cb, bkg_VV);
    CU::QCDscale_EWK().ApplyGlobal(cb, bkg_EWK);
    CU::QCDscale_ttbar().ApplyGlobal(cb, bkg_TT);
    CU::QCDscale_tW().ApplyGlobal(cb, bkg_ST);
    CU::QCDscale_ZH().ApplyGlobal(cb, bkg_VH);

    // CU::scale_j().Apply(cb, all_mc_processes);
//    CU::scale_b().ApplyGlobal(cb, signal_processes, bkg_pure_MC, bkg_TT);

    static constexpr double eff_b_unc = 0.02;
    CU::eff_b().Apply(cb, eff_b_unc, bkg_DY_1b, bkg_VV, bkg_EWK, bkg_VH, bkg_ST);
    CU::eff_b().Apply(cb, eff_b_unc * std::sqrt(2.), signal_processes, bkg_DY_2b, bkg_TT);

    CU::eff_e().Channel("eTau").Apply(cb, CU::eff_e().up_value, all_mc_processes);
    CU::eff_m().Channel("muTau").Apply(cb, CU::eff_m().up_value, all_mc_processes);
    CU::eff_t().Channels({"eTau", "muTau"}).Apply(cb, CU::eff_t().up_value, all_mc_processes);
    CU::eff_t().Channel("tauTau").Apply(cb, CU::eff_t().up_value * std::sqrt(2.), all_mc_processes);
    // CU::scale_t().Apply(cb, all_mc_processes);

    // CU::topPt().Apply(cb, bkg_TT);


    auto dy_file = root_ext::OpenRootFile("HHStatAnalysis/Run2/data/DY_Scale_factor_LO_NbjetBins_ptBins_pt_H_tt_mh_2018_full_run2v.root");
    auto dy_matrix = root_ext::ReadObject<TH2D>(*dy_file, "NbjetBins_ptBins/covariance_matrix");

    const size_t DYUncDim = dy_matrix->GetNbinsX();
    TMatrixD dy_unc_cov(DYUncDim, DYUncDim);


    for(size_t i = 1; i <= DYUncDim; ++i) {
      for(size_t j = i; j <= DYUncDim; ++j) {
        dy_unc_cov[i-1][j-1] = dy_unc_cov[j-1][i-1] = dy_matrix->GetBinContent(i,j);
      }
    }

    TVectorD dy_sf(DYUncDim);
    auto sf_histo = root_ext:: ReadObject<TH1D>(*dy_file, "NbjetBins_ptBins/scale_factors");

    for(size_t i = 1; i <= DYUncDim; ++i) {
      dy_sf[i-1] = sf_histo->GetBinContent(i);
    }

    auto dy_w_inv = stat_tools::ComputeWhiteningMatrix(dy_unc_cov).Invert();
    std::cout << "ttbb: inverse whitening matrix for DY sf covariance matrix" << std::endl;
    dy_w_inv.Print();

    for(size_t n = 0; n < DYUncDim; ++n) {
        std::ostringstream ss_unc_name;
        ss_unc_name << "DY_norm_unc_" << n;
        const Uncertainty DY_norm_unc(ss_unc_name.str(), CorrelationRange::Analysis, UncDistributionType::lnN);
        for(size_t k = 0; k < bkg_DY.size(); ++k) {
            const double unc_value = dy_w_inv[k][n] / dy_sf[k];
//            if(std::abs(unc_value) >= unc_thr)
            DY_norm_unc.Apply(cb, unc_value, bkg_DY.at(k));
        }
    }

    static const std::map<std::string, std::tuple<double, double, double, double, double, double>> qcd_os_ss_sf = {
        { "eTau", std::make_tuple(1.24, 0.05, 1.87, 0.13, 2.663, 0.167) },
        { "muTau", std::make_tuple(1.363, 0.055, 2.108, 0.149, 4.252, 0.403) },
        { "tauTau", std::make_tuple(1.6, 0.1, 1.521, 0.172, 2.729, 0.260) }
    };


    const Uncertainty qcd_norm("qcd_norm", CorrelationRange::Category, UncDistributionType::lnN);
    for(const auto& channel : desc.channels) {
        for(const auto& category : desc.categories) {
            const auto qcd_yield = GetBackgroundYield(bkg_QCD, channel, category);
            if(!qcd_yield)
                throw exception("QCD not found");
            const double ss_qcd_yield = qcd_yield->value / std::get<0>(qcd_os_ss_sf.at(channel));
            const double rel_error = 1 / std::sqrt(ss_qcd_yield);
            if(rel_error >= unc_thr)
                qcd_norm.Channel(channel).Category(category).Apply(cb, rel_error, bkg_QCD);
        }
    }

    const Uncertainty qcd_sf_unc("qcd_sf_unc", CorrelationRange::Channel, UncDistributionType::lnN);
    for(const auto& sf_entry : qcd_os_ss_sf) {
        const double rel_stat_unc = std::get<1>(sf_entry.second) / std::get<0>(sf_entry.second);
        double rel_ext_unc = 0.3;
//        if(std::abs(std::get<2>(sf_entry.second) - std::get<0>(sf_entry.second)) >
//                std::get<1>(sf_entry.second) + std::get<3>(sf_entry.second))
//            rel_ext_unc = std::get<2>(sf_entry.second) / std::get<0>(sf_entry.second) - 1;
        const double cmb_unc = std::sqrt(std::pow(rel_stat_unc, 2) + std::pow(rel_ext_unc, 2));
        const double cmb_unc_up = rel_ext_unc > 0 ? cmb_unc : rel_stat_unc;
        const double cmb_unc_down = rel_ext_unc > 0 ? -cmb_unc : -rel_stat_unc;
        qcd_sf_unc.Channel(sf_entry.first).Apply(cb, std::make_pair(cmb_unc_up, cmb_unc_down), bkg_QCD);
        const auto prev_precision = std::cout.precision();
        std::cout << std::setprecision(4) << "ttbb/" << sf_entry.first << ": QCD OS/SS scale factor uncertainties:\n"
                  << "\tstat unc: +/- " << rel_stat_unc * 100 << "%\n"
                  << "\textrapolation unc: " << rel_ext_unc * 100 << "%\n"
                  << "\ttotal unc: +" << cmb_unc_up * 100 << "% / " << cmb_unc_down * 100 << "%."
                  << std::setprecision(prev_precision) << std::endl;
    }
}

} // namespace Run2
} // namespace stat_models
} // namespace hh_analysis
