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
    static const v_str eras;
    static const std::string bkg_TT, bkg_W, bkg_QCD, bkg_VH, bkg_ttH, bkg_H, bkg_EWK, bkg_ST, bkg_VVV,
                             bkg_ttV, bkg_ttVV, bkg_VV, bkg_DY_M_10_50;
    static const std::string bkg_DY_0b_0JPt, bkg_DY_0b_10JPt, bkg_DY_0b_30JPt, bkg_DY_0b_50JPt, bkg_DY_0b_100JPt,
                             bkg_DY_0b_200JPt;
    static const std::string bkg_DY_1b_0JPt, bkg_DY_1b_10JPt, bkg_DY_1b_30JPt, bkg_DY_1b_50JPt, bkg_DY_1b_100JPt,
                             bkg_DY_1b_200JPt;
    static const std::string bkg_DY_2b_0JPt, bkg_DY_2b_10JPt, bkg_DY_2b_30JPt, bkg_DY_2b_50JPt, bkg_DY_2b_100JPt,
                             bkg_DY_2b_200JPt;
    static const v_str bkg_DY, bkg_DY_0b, bkg_DY_1b, bkg_DY_2b;
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

} // namespace Run2
} // namespace stat_models
} // namespace hh_analysis
