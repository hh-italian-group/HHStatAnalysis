/*! Stat model for h->hh->bbtautau.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#include <iostream>
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/CardWriter.h"
#include "CombineHarvester/CombinePdfs/interface/MorphFunctions.h"
#include "HHStatAnalysis/Run2/interface/ttbb_nonresonant.h"
#include "HHStatAnalysis/Core/interface/exception.h"
#include "HHStatAnalysis/Core/interface/Tools.h"
#include "HHStatAnalysis/Core/interface/TextIO.h"
#include "HHStatAnalysis/Core/interface/RootExt.h"
#include "HHStatAnalysis/Run2/interface/CommonUncertainties.h"
#include "HHStatAnalysis/StatModels/interface/PhysicalConstants.h"

namespace hh_analysis {
namespace stat_models {
namespace Run2 {

void ttbb_nonresonant::CreateDatacards(const std::string& output_path)
{
    // static constexpr double sf = phys_const::XS_HH_13TeV * phys_const::BR_HH_bbtautau;

    ch::CombineHarvester harvester;

    AddProcesses(harvester);
    AddSystematics(harvester);
    ExtractShapes(harvester);
    // if(desc.limit_type == LimitType::SM) {
    //     harvester.cp().process(signal_processes).ForEachProc([](ch::Process *p) {
    //         p->set_rate(p->rate() * sf);
    //     });
    // }
    static const std::map<unsigned, double> fixes = {
        { 2016, 1.119e+06 / 2.36716e+06 },
        { 2017, 458765 / 2.74441e+06 },
        { 2018, 864502 / 4.71399e+06 },
    };

    for(const auto& channel : desc.channels) {
        for(unsigned year : desc.years) {
            const std::string ch_channel = GetCHChannel(year, channel);
            harvester.cp().channel({ch_channel})
                     .process({ "ggHH_kl_0_kt_1_ttbb_LO", "ggHH_kl_1_kt_1_ttbb_LO", "ggHH_kl_5_kt_1_ttbb_LO" })
                     .ForEachProc([&](ch::Process *p) { p->set_rate(p->rate() * fixes.at(year)); });
        }
    }

    if(!desc.model_signal_process.empty())
        RenameProcess(harvester, desc.signal_processes.at(0), desc.model_signal_process);

    FixNegativeBins(harvester);
    harvester.cp().backgrounds().MergeBinErrors(bbb_unc_threshold, bin_merge_threashold);
    harvester.cp().backgrounds().AddBinByBin(bbb_unc_threshold, true, &harvester);
    ch::SetStandardBinNames(harvester);

    harvester.SetGroup("QCD_bbb", { ".*_QCD_bin_[0-9]+" });
    harvester.SetGroup("DY_bbb", { ".*_DY_[0-9]b_bin_[0-9]+" });

    std::string output_pattern = "/$TAG/$BIN.txt";

    ch::CardWriter writer(output_path + output_pattern, output_path + "/$TAG/hh_ttbb_input.root");
    writer.SetWildcardMasses({});
    if(desc.combine_channels)
        writer.WriteCards("cmb", harvester);
    if(desc.per_channel_limits && desc.channels.size() > 1) {
        for(const auto& channel : desc.channels) {
            v_str ch_channels;
            for(auto year : desc.years) {
                const auto& ch_channel = GetCHChannel(year, channel);
                ch_channels.push_back(ch_channel);
                if(desc.per_year_limits && desc.years.size() > 1)
                    writer.WriteCards(ch_channel, harvester.cp().channel({ch_channel}));
            }
            writer.WriteCards(channel, harvester.cp().channel(ch_channels));
            if(desc.per_category_limits && desc.categories.size() > 1) {
                for(size_t n = 0; n < desc.categories.size(); ++n) {
                    if(!IsValidCategory(desc.years, {channel}, desc.categories.at(n))) continue;
                    const std::string name = boost::str(boost::format("%1%_%2%") % channel % desc.categories.at(n));
                    writer.WriteCards(name, harvester.cp().bin_id({int(n)}).channel(ch_channels));
                    if(desc.per_year_limits && desc.years.size() > 1) {
                        for(auto year : desc.years) {
                            if(!IsValidCategory(year, channel, desc.categories.at(n))) continue;
                            const auto& ch_channel = GetCHChannel(year, channel);
                            const std::string full_name = boost::str(boost::format("%1%_%2%")
                                    % ch_channel % desc.categories.at(n));
                            writer.WriteCards(full_name, harvester.cp().bin_id({int(n)}).channel({ch_channel}));
                        }
                    }
                }
            }
        }
    }
    if(desc.per_category_limits && desc.categories.size() > 1) {
        for(size_t n = 0; n < desc.categories.size(); ++n) {
            if(!IsValidCategory(desc.years, desc.channels, desc.categories.at(n))) continue;
            writer.WriteCards(desc.categories.at(n), harvester.cp().bin_id({int(n)}));
        }
    }
    if(desc.per_year_limits && desc.years.size() > 1) {
        for(auto year : desc.years) {
            v_str ch_channels;
            for(const auto& channel : desc.channels)
                ch_channels.push_back(GetCHChannel(year, channel));
            writer.WriteCards(std::to_string(year) + "_cmb", harvester.cp().channel(ch_channels));
            if(desc.per_category_limits && desc.categories.size() > 1) {
                for(size_t n = 0; n < desc.categories.size(); ++n) {
                    if(!IsValidCategory({year}, desc.channels, desc.categories.at(n))) continue;
                    const std::string name = boost::str(boost::format("%1%_%2%") % year % desc.categories.at(n));
                    writer.WriteCards(name, harvester.cp().bin_id({int(n)}).channel(ch_channels));
                }
            }
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

} // namespace Run2
} // namespace stat_models
} // namespace hh_analysis
