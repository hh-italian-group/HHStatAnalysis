/*! Stat model for X->hh->bbtautau.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#include <iostream>
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/CardWriter.h"
#include "CombineHarvester/CombinePdfs/interface/MorphFunctions.h"
#include "HHStatAnalysis/Run2/interface/ttbb_resonant.h"
#include "HHStatAnalysis/Core/interface/exception.h"
#include "HHStatAnalysis/Core/interface/Tools.h"
#include "HHStatAnalysis/Core/interface/TextIO.h"
#include "HHStatAnalysis/Core/interface/RootExt.h"
#include "HHStatAnalysis/Run2/interface/CommonUncertainties.h"

namespace hh_analysis {
namespace stat_models {
namespace Run2 {

void ttbb_resonant::CreateDatacards(const std::string& output_path)
{
    ch::CombineHarvester harvester;

    AddProcesses(harvester);
    AddSystematics(harvester);
    ExtractShapes(harvester);

    if(!desc.model_signal_process.empty())
        RenameProcess(harvester, desc.signal_processes.at(0), desc.model_signal_process);

    FixNegativeBins(harvester);
    harvester.cp().backgrounds().MergeBinErrors(bbb_unc_threshold, bin_merge_threashold);
    harvester.cp().backgrounds().AddBinByBin(bbb_unc_threshold, true, &harvester);
    ch::SetStandardBinNames(harvester);

    std::shared_ptr<RooWorkspace> workspace;
    RooRealVar mH("mH", "mH", Parse<double>(desc.signal_points.front()), Parse<double>(desc.signal_points.back()));
    std::string output_pattern = "/$TAG/$MASS/$BIN.txt";
    if(desc.morph) {
        workspace = std::make_shared<RooWorkspace>("hh_ttbb", "hh_ttbb");
        for(const auto& bin : harvester.bin_set())
            ch::BuildRooMorphing(*workspace, harvester, bin, desc.model_signal_process, mH, "norm", true, true, true);
        harvester.AddWorkspace(*workspace);
        harvester.cp().process({desc.model_signal_process}).ExtractPdfs(harvester, workspace->GetName(),
                                                                        "$BIN_$PROCESS_morph");
        output_pattern = "/$TAG/$BIN.txt";
    }

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

} // namespace Run2
} // namespace stat_models
} // namespace hh_analysis
