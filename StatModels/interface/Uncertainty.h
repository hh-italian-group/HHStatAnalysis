/*! Definition of uncertainty descriptor.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#pragma once

#include "CombineHarvester/CombineTools/interface/CombineHarvester.h"
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "HHStatAnalysis/Core/interface/EnumNameMap.h"
#include "HHStatAnalysis/Core/interface/Tools.h"

namespace hh_analysis {

ENUM_OSTREAM_OPERATORS()

enum class CorrelationRange { LHC = 4, Experiment = 3, Analysis = 2, Channel = 1, Category = 0 };

inline bool operator<=(CorrelationRange a, CorrelationRange b)
{
    return static_cast<int>(a) <= static_cast<int>(b);
}

enum class UncDistributionType { lnN, lnU, shape };
ENUM_NAMES(UncDistributionType) = {
    { UncDistributionType::lnN, "lnN" },
    { UncDistributionType::lnU, "lnU" },
    { UncDistributionType::shape, "shape" }
};

enum class UncVariation { Up, Down };
ENUM_NAMES(UncVariation) = {
    { UncVariation::Up, "Up" },
    { UncVariation::Down, "Down" },
};

struct Uncertainty {
    static constexpr double NaN = std::numeric_limits<double>::quiet_NaN();
    static constexpr unsigned WildcardYear = 0;
    struct UpDown {
        double up, down;
        UpDown() : up(NaN), down(NaN) {}
        UpDown(double _up) : up(_up), down(NaN) {}
        UpDown(double _up, double _down) : up(_up), down(_down) {}
        bool IsAsymmetric() const { return !std::isnan(down); }
    };
    using UpDownMap = std::map<unsigned, UpDown>;

    std::string name;
    CorrelationRange correlation_range = CorrelationRange::Experiment;
    UncDistributionType distr_type = UncDistributionType::lnN;
    std::vector<std::string> analysis_names, channel_names, category_names;
    bool use_era;

    UpDownMap up_down_values;

    Uncertainty() {}
    Uncertainty(const std::string& _name, CorrelationRange _correlation_range, UncDistributionType _distr_type,
                const std::vector<std::string>& _analysis_names = {},
                const std::vector<std::string>& _channel_names = {},
                const std::vector<std::string>& _category_names = {}) :
        name(_name), correlation_range(_correlation_range), distr_type(_distr_type), analysis_names(_analysis_names),
        channel_names(_channel_names), category_names(_category_names), use_era(false)
    {}

    Uncertainty(const std::string& _name, CorrelationRange _correlation_range, UncDistributionType _distr_type,
                double up_value, double down_value = NaN)
        : Uncertainty(_name, _correlation_range, _distr_type)
    {
        up_down_values[WildcardYear] = UpDown(up_value, down_value);
    }

    Uncertainty(const std::string& _name, CorrelationRange _correlation_range, UncDistributionType _distr_type,
                const UpDownMap& _up_down_values)
        : Uncertainty(_name, _correlation_range, _distr_type)
    {
        up_down_values = _up_down_values;
    }

    template<typename ...Processes>
    void ApplyGlobal(ch::CombineHarvester& cb, const Processes& ...processes) const
    {
        ApplyGlobal(cb, WildcardYear, processes...);
    }

    template<typename ...Processes>
    void ApplyGlobal(ch::CombineHarvester& cb, unsigned year, const Processes& ...processes) const
    {
        if(distr_type == UncDistributionType::shape)
            Apply(cb, processes...);
        else if(IsAsymmetric(year))
            Apply(cb, up_down_values.at(year), processes...);
        else
            Apply(cb, up_down_values.at(year).up, processes...);
    }

    void CheckGlobalValuePresence(unsigned year = WildcardYear) const
    {
        if(!up_down_values.count(year))
            throw analysis::exception("Global value for uncertainty '%1%' is not defined.") % FullName();
    }

    double GetValue(unsigned year = WildcardYear) const
    {
        CheckGlobalValuePresence(year);
        const UpDown& up_down = up_down_values.at(year);
        if(up_down.IsAsymmetric())
            throw analysis::exception("Global value for uncertainty '%1%' is asymmetric.") % FullName();
        return up_down.up;
    }

    const UpDown& GetValueAsym(unsigned year = WildcardYear) const
    {
        CheckGlobalValuePresence(year);
        const UpDown& up_down = up_down_values.at(year);
        if(!up_down.IsAsymmetric())
            throw analysis::exception("Global value for uncertainty '%1%' is not asymmetric.") % FullName();
        return up_down;
    }

    bool IsAsymmetric(unsigned year = WildcardYear) const
    {
        CheckGlobalValuePresence(year);
        return up_down_values.at(year).IsAsymmetric();
    }


    Uncertainty DistrType(UncDistributionType _distr_type) const
    {
        Uncertainty u(*this);
        u.distr_type = _distr_type;
        return u;
    }

    Uncertainty Analysis(const std::string& analysis) const
    {
        Uncertainty u(*this);
        u.analysis_names = { analysis };
        return u;
    }

    Uncertainty Analyses(const std::vector<std::string>& analyses) const
    {
        Uncertainty u(*this);
        u.analysis_names = analyses;
        return u;
    }

    Uncertainty Channel(const std::string& channel) const
    {
        Uncertainty u(*this);
        u.channel_names = { channel };
        return u;
    }

    Uncertainty Channels(const std::vector<std::string>& channels) const
    {
        Uncertainty u(*this);
        u.channel_names = channels;
        return u;
    }

    Uncertainty Category(const std::string& category) const
    {
        Uncertainty u(*this);
        u.category_names = { category };
        return u;
    }

    Uncertainty Categories(const std::vector<std::string>& categories) const
    {
        Uncertainty u(*this);
        u.category_names = categories;
        return u;
    }

    Uncertainty UseEra(bool use_era) const
    {
        Uncertainty u(*this);
        u.use_era = use_era;
        return u;
    }

    std::string FullName() const
    {
        static const std::string exp_name = "CMS";
        static const std::string era = "$ERA";
        static const std::string analysis = "$ANALYSIS";
        static const std::string channel = "$CHANNEL";
        static const std::string category = "$BIN";
        static const std::string sep = "_";
        std::ostringstream ss;
        if(correlation_range <= CorrelationRange::Experiment)
            ss << exp_name << sep;
//        if(distr_type == UncDistributionType::shape)
//            ss << distr_type << sep;
        if(name.size())
            ss << name << sep;
        if(correlation_range <= CorrelationRange::Analysis)
            ss << analysis << sep;
        if(correlation_range == CorrelationRange::Channel)
            ss << channel << sep;
        if(correlation_range <= CorrelationRange::Category)
            ss << category << sep;
        if(use_era)
            ss << era;
        else if(ss.cur > 0)
            ss.seekp(-1, ss.cur);

        std::string full_name = ss.str();
        if(!use_era)
            full_name.erase(full_name.size() - 1);
        return full_name;
    }

    std::string FullNameBinByBin(const std::string& process, size_t bin) const
    {
        static const std::string sep = "_";
        std::ostringstream ss;
        ss << FullName() << sep << process << sep << "bin" << sep << bin;
        return ss.str();
    }

    bool operator<(const Uncertainty& other) const
    {
        if(correlation_range != other.correlation_range) return correlation_range < other.correlation_range;
        if(name != other.name) return name < other.name;
        return FullName() < other.FullName();
    }

    double ConvertToDatacardUncValue(double value) const
    {
        return distr_type == UncDistributionType::lnN ? 1 + value : value;
    }

    template<typename SystMap>
    void ApplyMap(ch::CombineHarvester& cb, const SystMap& syst_map,
               const std::vector<std::string>& all_processes) const
    {
        auto cb_copy = cb.cp().process(all_processes);
        if(analysis_names.size())
            cb_copy = cb_copy.analysis(analysis_names);
        if(channel_names.size())
            cb_copy = cb_copy.channel(channel_names);
        if(category_names.size())
            cb_copy = cb_copy.bin(category_names);
        std::ostringstream ss_distr;
        ss_distr << distr_type;
        cb_copy.AddSyst(cb, FullName(), ss_distr.str(), syst_map);
    }

    template<typename SystMap, typename ...Processes>
    void ApplyMap(ch::CombineHarvester& cb, const SystMap& syst_map, const Processes& ...processes) const
    {
        std::vector<std::string> all_processes;
        analysis::tools::put_back(all_processes, processes...);
        ApplyMap(cb, syst_map, all_processes);
    }

    template<typename ...Processes>
    void Apply(ch::CombineHarvester& cb, double value, const Processes& ...processes) const
    {
        ApplyMap(cb, ch::syst::SystMap<>::init(ConvertToDatacardUncValue(value)), processes...);
    }

    template<typename ...Processes>
    void Apply(ch::CombineHarvester& cb, UpDown up_down_values, const Processes& ...processes) const
    {
        const UpDown up_down(ConvertToDatacardUncValue(up_down_values.up),
                             ConvertToDatacardUncValue(up_down_values.down));
        ApplyMap(cb, ch::syst::SystMapAsymm<>::init(up_down.down, up_down.up), processes...);
    }

    template<typename ...Processes>
    void Apply(ch::CombineHarvester& cb, const Processes& ...processes) const
    {
        Apply(cb, 1.0, processes...);
    }

    void ApplyBinByBin(ch::CombineHarvester& cb, const std::string& process, size_t bin_id)
    {
        const auto syst_map = ch::syst::SystMap<>::init(1.0);
        auto cb_copy = cb.cp().process({process});
        if(analysis_names.size())
            cb_copy = cb_copy.analysis(analysis_names);
        if(channel_names.size())
            cb_copy = cb_copy.channel(channel_names);
        if(category_names.size())
            cb_copy = cb_copy.bin(category_names);
        std::ostringstream ss_distr;
        ss_distr << distr_type;
        cb_copy.AddSyst(cb, FullNameBinByBin(process, bin_id), ss_distr.str(), syst_map);
    }
};

} // namespace hh_analysis
