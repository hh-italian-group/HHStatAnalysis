/*! Implementation of the base class for HH stat models.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#include "HHStatAnalysis/StatModels/interface/StatModel.h"

namespace hh_analysis {
namespace stat_models {

const StatModel::v_str StatModel::wildcard = { "*" };

StatModel::StatModel(const StatModelDescriptor& _desc, const std::string& input_file_name) :
    desc(_desc), input_file(root_ext::OpenRootFile(input_file_name))
{
}

void StatModel::FixNegativeBins(ch::CombineHarvester& harvester)
{
    harvester.ForEachProc([](ch::Process *p) {
        if(!ch::HasNegativeBins(p->shape())) return;
        std::cout << "[Negative bins] Fixing negative bins for " << p->bin() << "," << p->process() << "\n";
        auto new_shape = p->ClonedShape();
        ch::ZeroNegativeBins(new_shape.get());
        p->set_shape(std::move(new_shape), false);
    });
}

void StatModel::RenameProcess(ch::CombineHarvester& harvester, const std::string& old_name, const std::string& new_name)
{
    harvester.ForEachObj([&](ch::Object *obj) {
        if(obj->process() == old_name)
            obj->set_process(new_name);
    });
}

ch::Categories StatModel::GetChannelCategories(const std::string& channel)
{
    ch::Categories ch_categories;
    for(size_t n = 0; n < desc.categories.size(); ++n) {
        const std::string bin_name = ShapeNameRule::BinName(channel, desc.categories.at(n));
        ch_categories.push_back({n, bin_name});
    }
    return ch_categories;
}

void StatModel::ExtractShapes(ch::CombineHarvester& cb) const
{
    const auto signal_rule = SignalShapeNameRule().SetPrefix(desc.signal_point_prefix);
    for(const std::string& point_str : desc.signal_points) {
        const double point = Parse<double>(point_str);
        const auto point_rule = signal_rule.SetPoint(point);
        cb.cp().process(SignalProcesses()).mass({point_str})
               .ExtractShapes(input_file->GetName(), point_rule, point_rule.AddSystematicVariable());
    }

    const auto bkg_rule = BackgroundShapeNameRule();
    cb.cp().process(BackgroundProcesses())
           .ExtractShapes(input_file->GetName(), bkg_rule, bkg_rule.AddSystematicVariable());
}

const StatModel::Hist* StatModel::GetSignalHistogram(const std::string& process, double point,
                                                     const std::string& channel, const std::string& category,
                                                     const std::string& region) const
{
    const auto name_rule = SignalShapeNameRule().SetProcess(process).SetPrefix(desc.signal_point_prefix)
                                                .SetPoint(point).SetBin(channel, category, region);
    if(name_rule.HasVariables())
        throw exception("Insufficient information to make full histogram name for signal process '%1%' at point %2%"
                        " in bin '%3%'.") % process % point % ShapeNameRule::BinName(channel, category, region);
    return ReadObject<Hist>(name_rule);
}

const StatModel::Hist* StatModel::GetBackgroundHistogram(const std::string& process, const std::string& channel,
                                   const std::string& category, const std::string& region) const
{
    const auto name_rule = BackgroundShapeNameRule().SetProcess(process).SetBin(channel, category, region);
    if(name_rule.HasVariables())
        throw exception("Insufficient information to make full histogram name for background process '%1%'"
                        " in bin '%2%'.") % process % ShapeNameRule::BinName(channel, category, region);
    return ReadObject<Hist>(name_rule);
}

Yield StatModel::GetYield(const Hist& hist)
{
    Yield yield;
    yield.value = hist.IntegralAndError(1, hist.GetNbinsX(), yield.error);
    return yield;
}

Yield StatModel::GetSignalYield(const std::string& process, double point, const std::string& channel,
                                const std::string& category, const std::string& region) const
{
    const auto hist = GetSignalHistogram(process, point, channel, category, region);
    return GetYield(*hist);
}

Yield StatModel::GetBackgroundYield(const std::string& process, const std::string& channel,
                                    const std::string& category, const std::string& region) const
{
    const auto hist = GetBackgroundHistogram(process, channel, category, region);
    return GetYield(*hist);
}

} // namespace stat_models
} // namespace hh_analysis
