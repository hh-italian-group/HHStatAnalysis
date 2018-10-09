/*! Common part of stat models for the hh->bbtautau analyses.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#pragma once

#include "HHStatAnalysis/StatModels/interface/StatModel.h"
#include "HHStatAnalysis/StatModels/interface/ShapeNameRule.h"

namespace hh_analysis {
namespace stat_models {
namespace Run2_2017 {

class ttbb_base : public StatModel {
public:
    static const v_str ana_name;
    static const v_str eras;
    static const std::string bkg_TT, bkg_tW, bkg_W, bkg_EWK, bkg_ZH, bkg_QCD, bkg_DY_0b, bkg_DY_1b, bkg_DY_2b,
                             bkg_WW, bkg_WZ, bkg_ZZ;
    static const v_str bkg_DY, bkg_VV;
    static const v_str bkg_pure_MC, bkg_MC, bkg_all;
    static const v_str shape_suffixes;

    static constexpr double bbb_unc_threshold = 0.1;
    static constexpr double bin_merge_threashold = 0.5;

    ttbb_base(const StatModelDescriptor& _desc, const std::string& input_file_name);

protected:
    virtual void AddProcesses(ch::CombineHarvester& combine_harvester);
    virtual void AddSystematics(ch::CombineHarvester& combine_harvester);

    virtual const v_str& SignalProcesses() const override { return signal_processes; }
    virtual const v_str& BackgroundProcesses() const override { return bkg_all; }

    virtual ShapeNameRule SignalShapeNameRule() const override { return "$BIN/$PROCESS_$PREFIX$POINT"; }
    virtual ShapeNameRule BackgroundShapeNameRule() const override { return "$BIN/$PROCESS"; }

protected:
    const v_str signal_processes, all_mc_processes, all_processes;
};

} // namespace Run2_2017
} // namespace stat_models
} // namespace hh_analysis
