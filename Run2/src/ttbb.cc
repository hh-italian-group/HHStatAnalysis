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
const std::string ttbb_base::era = "13TeV";

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

const StatModel::v_str ttbb_base::bkg_pure_MC = analysis::tools::join(bkg_TT, bkg_W, bkg_VH, bkg_ttH, bkg_H, bkg_EWK,
                                                                      bkg_ST, bkg_VVV, bkg_ttV, bkg_ttVV,
                                                                      bkg_VV, bkg_DY_M_10_50);
const StatModel::v_str ttbb_base::shape_suffixes = { "", "scale_t", "scale_j" };

std::string ttbb_base::GetCHChannel(unsigned year, const std::string& channel)
{
    return boost::str(boost::format("%1%_%2%") % year % channel);
}

ttbb_base::v_str ttbb_base::GetCHChannels(const std::vector<unsigned>& years, const v_str& channels)
{
    v_str ch_channels;
    for(auto year : years) {
        for(const auto& channel : channels)
            ch_channels.push_back(GetCHChannel(year, channel));
    }
    return ch_channels;
}

bool ttbb_base::IsValidCategory(unsigned year, const std::string& channel, const std::string& category)
{
    return category != "2j1b+_VBFT" || (year == 2018 && channel == "tauTau");
}

bool ttbb_base::IsValidCategory(const std::vector<unsigned>& years, const v_str& channels, const std::string& category)
{
    for(auto year : years) {
        for(const auto& channel : channels) {
            if(IsValidCategory(year, channel, category))
                return true;
        }
    }
    return false;
}

ttbb_base::ttbb_base(const StatModelDescriptor& _desc, const std::string& input_file_name) :
    StatModel(_desc, input_file_name), signal_processes(desc.signal_processes)
{
    std::map<int, std::vector<int>> j_pts;
    j_pts[2017] = {0, 10, 30, 50, 100, 200};
    j_pts[2016] = {0, 10, 50, 80, 110, 190};

    std::set<std::string> all_DY_names;
    std::map<size_t, std::set<std::string>> nb_DY_names;
    for(int year : {2016, 2017}) {
        for(size_t n_b = 0; n_b < 3; ++n_b) {
            for(int j_pt : j_pts[year]) {
                const std::string name = boost::str(boost::format("DY_%1%b_%2%JPt") % n_b % j_pt);
                bkg_DY_nb[year][n_b].push_back(name);
                all_DY_names.insert(name);
                nb_DY_names[n_b].insert(name);
            }
        }
    }
    bkg_DY_nb[2018] = bkg_DY_nb[2017];
    bkg_DY_1b.insert(bkg_DY_1b.end(), nb_DY_names[1].begin(), nb_DY_names[1].end());
    bkg_DY_2b.insert(bkg_DY_2b.end(), nb_DY_names[2].begin(), nb_DY_names[2].end());

    for(int year : {2016, 2017, 2018}) {
        bkg_DY[year] = analysis::tools::join(bkg_DY_nb[year][0], bkg_DY_nb[year][1], bkg_DY_nb[year][2]);
    }

    v_str all_DY_names_v(all_DY_names.begin(), all_DY_names.end());

    bkg_MC = analysis::tools::join(bkg_pure_MC, all_DY_names_v);
    //bkg_all = analysis::tools::join(bkg_MC, bkg_QCD);
    bkg_all = analysis::tools::join(bkg_MC, bkg_QCD, "ggHH_kl_1_kt_1_ttbb_LO");
    all_mc_processes = ch::JoinStr({ signal_processes, bkg_MC });
    all_processes = ch::JoinStr({ signal_processes, bkg_all });
}

void ttbb_base::AddProcesses(ch::CombineHarvester& cb)
{
    static const v_str unc_variations = { "Up", "Down" };
    static const v_str no_unc_variations = { "" };
    for(unsigned year : desc.years) {
        const v_str eras = { era + std::to_string(year) };
        for(const auto& channel : desc.channels) {
            const auto& ch_all_categories = GetChannelCategories(channel);
            const std::string ch_channel = GetCHChannel(year, channel);
            for(size_t n = 0; n < desc.categories.size(); ++n) {
                const auto& category = desc.categories.at(n);
                if(!IsValidCategory(year, channel, category)) continue;
                const std::string bin_name = ShapeNameRule::BinName(ch_channel, category);
                ch::Categories ch_categories;
                ch_categories.push_back({n, bin_name});
                cb.AddObservations(wildcard, ana_name, eras, {ch_channel}, ch_categories);
                cb.AddProcesses(wildcard, ana_name, eras, {ch_channel}, signal_processes,
                                ch_categories, true);
                for(const auto& bkg : bkg_all) {
                    bool all_ok = true;
                    for(auto suffix_begin = shape_suffixes.begin(); all_ok && suffix_begin != shape_suffixes.end();
                            ++suffix_begin) {
                        if(!suffix_begin->empty() && bkg == bkg_QCD) continue;
                        const auto suffix_endings = suffix_begin->empty() ? no_unc_variations : unc_variations;
                        for(const std::string& suffix_end : suffix_endings) {
                            std::string suffix;
                            if(!suffix_begin->empty()) {
                                suffix = boost::str(boost::format("_CMS_%1%_%2%%3%%4%")
                                         % (*suffix_begin) % era % year % suffix_end);
                            }
                            const auto yield = GetBackgroundYield(bkg + suffix, ch_channel, category);
                            if(!yield || yield->value < 1.1e-9) {
                                all_ok = false;
                                break;
                            }
                        }
                    }
                    if(all_ok)
                        cb.AddProcesses(wildcard, ana_name, eras, {ch_channel}, {bkg}, ch_categories, false);
                }
            }
        }
    }
}

void ttbb_base::AddSystematics(ch::CombineHarvester& cb)
{
    using ch::syst::SystMap;
    using CU = CommonUncertainties;
    static constexpr double unc_thr = 0.005;

    for(auto year : desc.years) {
        CU::lumi().UseEra(true).Channels(GetCHChannels({year}, desc.channels)).ApplyGlobal(cb, year,
            signal_processes, bkg_pure_MC);
    }
    CU::QCDscale_W().ApplyGlobal(cb, bkg_W);
    CU::QCDscale_WW().ApplyGlobal(cb, bkg_VV);
    CU::QCDscale_WZ().ApplyGlobal(cb, bkg_VV);
    CU::QCDscale_ZZ().ApplyGlobal(cb, bkg_VV);
    CU::QCDscale_EWK().ApplyGlobal(cb, bkg_EWK);
    CU::QCDscale_ttbar().ApplyGlobal(cb, bkg_TT);
    CU::QCDscale_tW().ApplyGlobal(cb, bkg_ST);
    CU::QCDscale_ZH().ApplyGlobal(cb, bkg_VH);

    CU::scale_j().UseEra(true).Apply(cb, all_mc_processes);
    //CU::scale_b().ApplyGlobal(cb, signal_processes, bkg_pure_MC);

    static constexpr double eff_b_unc = 0.02;
    CU::eff_b().UseEra(true).Apply(cb, eff_b_unc, bkg_DY_1b, bkg_VV, bkg_EWK, bkg_VH, bkg_ST);
    CU::eff_b().UseEra(true).Apply(cb, eff_b_unc * std::sqrt(2.), signal_processes, bkg_DY_2b, bkg_TT);

    for(unsigned year : desc.years) {
        CU::eff_e().UseEra(true).Channel(GetCHChannel(year, "eTau")).ApplyGlobal(cb, all_mc_processes);
        CU::eff_m().UseEra(true).Channel(GetCHChannel(year, "muTau")).ApplyGlobal(cb, all_mc_processes);
        CU::eff_t().UseEra(true).Channels({GetCHChannel(year, "eTau"), GetCHChannel(year, "muTau")})
                   .ApplyGlobal(cb, all_mc_processes);
        CU::eff_t().UseEra(true).Channel(GetCHChannel(year, "tauTau"))
                   .Apply(cb, CU::eff_t().GetValue() * std::sqrt(2.), all_mc_processes);
    }
    CU::scale_t().UseEra(true).UseEra(true).Apply(cb, all_mc_processes);

    // CU::topPt().Apply(cb, bkg_TT);


    for(unsigned year : desc.years) {
        auto dy_file = root_ext::OpenRootFile(boost::str(boost::format("HHStatAnalysis/Run2/data/DY_Scale_factor_LO_NbjetBins_ptBins_pt_H_tt_mh_%1%_full_run2v.root") % year));
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
            ss_unc_name << "DY_norm_unc_" << year << "_" << (n + 1);
            const std::string unc_name = ss_unc_name.str();
            std::cout << unc_name << std::endl;
            const Uncertainty DY_norm_unc(unc_name, CorrelationRange::Analysis, UncDistributionType::lnN);
            for(size_t k = 0; k < bkg_DY.size(); ++k) {
                const double unc_value = dy_w_inv[k][n] / dy_sf[k];
                if(std::round(std::abs(unc_value) * 1000.) > 0) {
                    const double cmb_unc = std::round((1. + unc_value) * 1000.) / 1000.;
                    std::cout << bkg_DY.at(year).at(k) << " = " << cmb_unc << std::endl;
                    DY_norm_unc.Apply(cb, unc_value, bkg_DY.at(year).at(k));
                }
            }
            std::cout << std::endl;
        }
    }

    static const std::map<std::string, std::tuple<double, double, double, double, double, double>> qcd_os_ss_sf = {
        { "eTau", std::make_tuple(1.24, 0.05, 1.87, 0.13, 2.663, 0.167) },
        { "muTau", std::make_tuple(1.363, 0.055, 2.108, 0.149, 4.252, 0.403) },
        { "tauTau", std::make_tuple(1.6, 0.1, 1.521, 0.172, 2.729, 0.260) }
    };


    const Uncertainty qcd_norm("qcd_norm", CorrelationRange::Category, UncDistributionType::lnN);
    for(unsigned year : desc.years) {
        for(const auto& channel : desc.channels) {
            const std::string ch_channel = GetCHChannel(year, channel);
            for(const auto& category : desc.categories) {
                const auto qcd_yield = GetBackgroundYield(bkg_QCD, ch_channel, category);
                if(qcd_yield) {
                    const double ss_qcd_yield = qcd_yield->value / std::get<0>(qcd_os_ss_sf.at(channel));
                    const double rel_error = 1 / std::sqrt(ss_qcd_yield);
                    if(rel_error >= unc_thr)
                        qcd_norm.Channel(ch_channel).Category(category).Apply(cb, rel_error, bkg_QCD);
                }
            }
        }
    }

    const Uncertainty qcd_sf_unc("qcd_sf_unc", CorrelationRange::Channel, UncDistributionType::lnN);
    for(unsigned year : desc.years) {
        for(const auto& sf_entry : qcd_os_ss_sf) {
            const double rel_stat_unc = std::get<1>(sf_entry.second) / std::get<0>(sf_entry.second);
            double rel_ext_unc = 0.3;
    //        if(std::abs(std::get<2>(sf_entry.second) - std::get<0>(sf_entry.second)) >
    //                std::get<1>(sf_entry.second) + std::get<3>(sf_entry.second))
    //            rel_ext_unc = std::get<2>(sf_entry.second) / std::get<0>(sf_entry.second) - 1;
            const double cmb_unc = std::sqrt(std::pow(rel_stat_unc, 2) + std::pow(rel_ext_unc, 2));
            const double cmb_unc_up = rel_ext_unc > 0 ? cmb_unc : rel_stat_unc;
            const double cmb_unc_down = rel_ext_unc > 0 ? -cmb_unc : -rel_stat_unc;
            const std::string ch_channel = GetCHChannel(year, sf_entry.first);
            qcd_sf_unc.Channel(ch_channel).Apply(cb, Uncertainty::UpDown(cmb_unc_up, cmb_unc_down), bkg_QCD);
            const auto prev_precision = std::cout.precision();
            std::cout << std::setprecision(4) << "ttbb/" << sf_entry.first << ": QCD OS/SS scale factor uncertainties:\n"
                      << "\tstat unc: +/- " << rel_stat_unc * 100 << "%\n"
                      << "\textrapolation unc: " << rel_ext_unc * 100 << "%\n"
                      << "\ttotal unc: +" << cmb_unc_up * 100 << "% / " << cmb_unc_down * 100 << "%."
                      << std::setprecision(prev_precision) << std::endl;
        }
    }
}

} // namespace Run2
} // namespace stat_models
} // namespace hh_analysis
