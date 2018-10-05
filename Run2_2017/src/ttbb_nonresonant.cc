/*! Stat model for h->hh->bbtautau.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#include <iostream>
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/CardWriter.h"
#include "CombineHarvester/CombinePdfs/interface/MorphFunctions.h"
#include "HHStatAnalysis/Run2_2017/interface/ttbb_nonresonant.h"
#include "HHStatAnalysis/Core/interface/exception.h"
#include "HHStatAnalysis/Core/interface/Tools.h"
#include "HHStatAnalysis/Core/interface/TextIO.h"
#include "HHStatAnalysis/Core/interface/RootExt.h"
#include "HHStatAnalysis/Run2_2017/interface/CommonUncertainties.h"
#include "HHStatAnalysis/StatModels/interface/PhysicalConstants.h"

namespace hh_analysis {
namespace stat_models {
namespace Run2_2017 {

void ttbb_nonresonant::CreateDatacards(const std::string& output_path)
{
    static constexpr double sf = phys_const::XS_HH_13TeV * phys_const::BR_HH_bbtautau;

    ch::CombineHarvester harvester;

    AddProcesses(harvester);
    AddSystematics(harvester);
    ExtractShapes(harvester);

    if(desc.limit_type == LimitType::SM) {
        harvester.cp().process(signal_processes).ForEachProc([](ch::Process *p) {
            p->set_rate(p->rate() * sf);
        });
    }

    if(desc.model_signal_process.size())
        RenameProcess(harvester, desc.signal_process, desc.model_signal_process);

    FixNegativeBins(harvester);
    harvester.cp().backgrounds().MergeBinErrors(bbb_unc_threshold, bin_merge_threashold);
    harvester.cp().backgrounds().AddBinByBin(bbb_unc_threshold, true, &harvester);
    ch::SetStandardBinNames(harvester);

    harvester.SetGroup("QCD_bbb", { ".*_QCD_bin_[0-9]+" });
    harvester.SetGroup("DY_bbb", { ".*_DY_[0-9]b_bin_[0-9]+" });

    std::string output_pattern = "/$TAG/$MASS/$BIN.txt";

    ch::CardWriter writer(output_path + output_pattern, output_path + "/$TAG/hh_ttbb_input.root");
    if(desc.morph)
        writer.SetWildcardMasses({});
    if(desc.combine_channels)
        writer.WriteCards("cmb", harvester);
    if(desc.per_channel_limits) {
        for(const auto& chn : desc.channels)
            writer.WriteCards(chn, harvester.cp().channel({chn}));
    }
    if(desc.per_category_limits) {
        for(size_t n = 0; n < desc.categories.size(); ++n) {
            writer.WriteCards(desc.categories.at(n), harvester.cp().bin_id({int(n)}));
        }
    }
}

void ttbb_nonresonant::AddSystematics(ch::CombineHarvester& cb)
{
    using CU = CommonUncertainties;
    ttbb_base::AddSystematics(cb);
    if(desc.limit_type != LimitType::SM) return;

    CU::QCDscale_ggHH().ApplyGlobal(cb, signal_processes);
    CU::pdf_ggHH().ApplyGlobal(cb, signal_processes);
    CU::BR_SM_H_bb().ApplyGlobal(cb, signal_processes);
    CU::BR_SM_H_tautau().ApplyGlobal(cb, signal_processes);
}

} // namespace 2017
} // namespace stat_models
} // namespace hh_analysis
