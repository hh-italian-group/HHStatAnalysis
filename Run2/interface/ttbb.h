/*! Common part of stat models for the hh->bbtautau analyses.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#pragma once

#include "HHStatAnalysis/StatModels/interface/StatModel.h"
#include "HHStatAnalysis/StatModels/interface/ShapeNameRule.h"

namespace hh_analysis {
namespace stat_models {
namespace Run2 {

class ttbb_base : public StatModel {
public:
    static const v_str ana_name;
    static const std::string era;
    static const std::string bkg_TT, bkg_W, bkg_QCD, bkg_VH, bkg_ttH, bkg_H, bkg_EWK, bkg_ST, bkg_VVV,
                             bkg_ttV, bkg_ttVV, bkg_VV, bkg_DY_M_10_50;
    static const v_str bkg_pure_MC;
    static const v_str shape_suffixes;

    static constexpr double bbb_unc_threshold = 0.1;
    static constexpr double bin_merge_threashold = 0.5;

    ttbb_base(const StatModelDescriptor& _desc, const std::string& input_file_name);

    static std::string GetCHChannel(unsigned year, const std::string& channel);
    static v_str GetCHChannels(const std::vector<unsigned>& years, const v_str& channel);

    static bool IsValidCategory(unsigned year, const std::string& channel, const std::string& category);
    static bool IsValidCategory(const std::vector<unsigned>& years, const v_str& channels, const std::string& category);

protected:
    virtual void AddProcesses(ch::CombineHarvester& combine_harvester);
    virtual void AddSystematics(ch::CombineHarvester& combine_harvester);

    virtual const v_str& SignalProcesses() const override { return signal_processes; }
    virtual const v_str& BackgroundProcesses() const override { return bkg_all; }

    virtual ShapeNameRule SignalShapeNameRule() const override { return "$BIN/$PROCESS"; }
    virtual ShapeNameRule BackgroundShapeNameRule() const override { return "$BIN/$PROCESS"; }

protected:
    v_str signal_processes;
    std::map<int, std::map<size_t, v_str>> bkg_DY_nb;
    std::map<int, v_str> bkg_DY;
    v_str bkg_DY_1b, bkg_DY_2b, bkg_MC, bkg_all, all_mc_processes, all_processes;
};

} // namespace Run2
} // namespace stat_models
} // namespace hh_analysis
