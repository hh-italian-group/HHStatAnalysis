/*! Definition of the base class for HH stat models.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#pragma once

#include "CombineHarvester/CombineTools/interface/CombineHarvester.h"
#include "HHStatAnalysis/Core/interface/RootExt.h"
#include "StatModelDescriptor.h"
#include "ShapeNameRule.h"

namespace hh_analysis {
namespace stat_models {

struct Yield {
    double value, error;
    Yield() : value(0), error(0) {}
    Yield(double _value, double _error) : value(_value), error(_error) {}
};

class StatModel {
public:
    using v_str = std::vector<std::string>;
    using v_double = std::vector<double>;
    using Hist = TH1;
    using Hist2D = TH2;

    static const v_str wildcard;

    StatModel(const StatModelDescriptor& _desc, const std::string& input_file_name);

    virtual ~StatModel() {}
    virtual void CreateDatacards(const std::string& output_path) = 0;

protected:

    static void FixNegativeBins(ch::CombineHarvester& harvester);
    static void RenameProcess(ch::CombineHarvester& harvester, const std::string& old_name,
                              const std::string& new_name);

    virtual const v_str& SignalProcesses() const = 0;
    virtual const v_str& BackgroundProcesses() const = 0;

    virtual ShapeNameRule SignalShapeNameRule() const = 0;
    virtual ShapeNameRule BackgroundShapeNameRule() const = 0;

    virtual ch::Categories GetChannelCategories(const std::string& channel);
    virtual void ExtractShapes(ch::CombineHarvester& cb) const;

    template<typename T>
    const T* ReadObject(const std::string& name) const { return root_ext::ReadObject<T>(*input_file, name); }
    template<typename T>
    const T* TryReadObject(const std::string& name) const
    {
        return root_ext::TryReadObject<T>(*input_file, name);
    }
    virtual const Hist* GetSignalHistogram(const std::string& process, double point, const std::string& channel,
                                           const std::string& category, const std::string& region = "") const;
    virtual const Hist* GetBackgroundHistogram(const std::string& process, const std::string& channel,
                                               const std::string& category, const std::string& region = "") const;

    static Yield GetYield(const Hist& hist);
    Yield GetSignalYield(const std::string& process, double point, const std::string& channel,
                         const std::string& category, const std::string& region = "") const;
    boost::optional<Yield> GetBackgroundYield(const std::string& process, const std::string& channel,
                                              const std::string& category, const std::string& region = "") const;

protected:
    StatModelDescriptor desc;
    std::shared_ptr<TFile> input_file;
};

using StatModelPtr = std::shared_ptr<StatModel>;
using StatModelCreator = StatModelPtr (*)(const char*, const StatModelDescriptor*, const char*);

} // namespace stat_models
} // namespace hh_analysis
