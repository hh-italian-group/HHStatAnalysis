/*! Tool that provides a simple division-based HH model-dependent interpretation.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <TFile.h>
#include <TTree.h>
#include <TH2.h>
#include <TGraph2D.h>
#include <Math/Interpolator.h>
#include "HHStatAnalysis/Core/interface/program_main.h"
#include "HHStatAnalysis/Core/interface/exception.h"
#include "HHStatAnalysis/Core/interface/EnumNameMap.h"
#include "HHStatAnalysis/Core/interface/NumericPrimitives.h"
#include "HHStatAnalysis/Core/interface/RootExt.h"

namespace hh_analysis {
using namespace analysis;
using StrVec = std::vector<std::string>;
using InterpolatorPtr = std::shared_ptr<ROOT::Math::Interpolator> ;

const std::vector<double> all_limit_quantiles = { -1., 0.025, 0.16, 0.5, 0.84, 0.975 };
const StrVec all_limit_quantile_names = { "obs", "exp-2", "exp-1", "exp0", "exp+1", "exp+2" };

size_t GetQuantileId(double quantile)
{
    static const double delta = 0.01;
    for(size_t n = 0; n < all_limit_quantiles.size(); ++n) {
        if(std::abs(all_limit_quantiles[n] - quantile) < delta)
            return n;
    }
    throw exception("Unknown quantile = %1%") % quantile;
}

size_t GetQuantileId(const std::string& quantile_name)
{
    for(size_t n = 0; n < all_limit_quantile_names.size(); ++n) {
        if(all_limit_quantile_names[n] == quantile_name)
            return n;
    }
    throw exception("Unknown quantile = '%1%'.") % quantile_name;
}


double GetHiggsMass(const std::string& file_name)
{
    static const StrVec mass_patterns = { "mR([0-9]*)", "mH([0-9]*)" };
    for(const auto& pattern_str : mass_patterns) {
        const boost::regex pattern(pattern_str);
        boost::smatch mass_match;
        if(boost::regex_search(file_name, mass_match, pattern)) {
            std::istringstream ss_mass(mass_match[1]);
            double mass;
            ss_mass >> mass;
            return mass;
        }
    }
    throw exception("Bad file name %1%") % file_name;
}

std::set<std::pair<double, std::string>> GetOrderedFileList(const std::string& input_dir_name,
                                                            const std::string& pattern_str)
{
    using namespace boost::filesystem;
    std::set<std::pair<double, std::string>> files;
    const boost::regex pattern(pattern_str);
    const path input_dir(input_dir_name);
    const directory_iterator end_iter;
    for(directory_iterator file_iter(input_dir); file_iter != end_iter; ++file_iter) {
        const auto& name = file_iter->path().string();
        if(is_regular_file(file_iter->path()) && boost::regex_match(name, pattern))
            files.insert(std::make_pair(GetHiggsMass(name), name));
    }
    return files;
}

enum class Particle { H, A, h };
ENUM_NAMES(Particle) = {
    { Particle::H, "H" }, { Particle::A, "A" }, { Particle::h, "h" }
};

enum class Process { gg_H, H_hh, h_gammagamma, h_bb, h_tautau };
ENUM_NAMES(Process) = {
    { Process::gg_H, "gg->H" }, { Process::H_hh, "H->hh" }, { Process::h_gammagamma, "h->gammagamma" },
    { Process::h_bb, "h->bb" }, { Process::h_tautau, "h->tautau" }
};

enum class Units { pb, fb };
ENUM_NAMES(Units) = {
    { Units::pb, "pb" }, { Units::fb, "fb" }
};

enum class CompositProcess { ggH_hh_bbtautau, ggH_hh_bbgammagamma };
ENUM_NAMES(CompositProcess) = {
    { CompositProcess::ggH_hh_bbtautau, "ggH_hh_bbtautau" },
    { CompositProcess::ggH_hh_bbgammagamma, "ggH_hh_bbgammagamma" },
};

ENUM_OSTREAM_OPERATORS()
ENUM_ISTREAM_OPERATORS()

double GetUnitsFactor(Units units)
{
    static const std::map<Units, double> units_factors = {
        { Units::pb, 1. }, { Units::fb, 1e-3 }
    };
    if(!units_factors.count(units))
        throw exception("Unknown units '%1%'") % units;
    return units_factors.at(units);
}

class Model {
public:
    typedef std::vector<double> Point;
    typedef std::map<Particle, double> ParticleParamMap;
    typedef std::map<Process, double> ProcessParamMap;

    struct PointDescriptor {
        Point point;
        ParticleParamMap masses;
        ProcessParamMap cross_sections, branching_ratios;
        const Model* model;

        const std::string& GetParamName(size_t dim) const
        {
            if(!model)
                throw exception("Model is not available.");
            return model->GetParamName(dim);
        }

        std::string GetParamNameString() const
        {
            if(!model)
                throw exception("Model is not available.");
            return model->GetParamNameString();
        }

        std::string ToString() const { return Model::VectorToString(point); }

        double GetMass(Particle particle) const
        {
            return GetValue(masses, particle, "mass");
        }

        double GetCrossSection(Process process) const
        {
            return GetValue(cross_sections, process, "cross section");
        }

        double GetBranchingRatio(Process process) const
        {
            return GetValue(branching_ratios, process, "branching ratio");
        }

        double GetProcessBR_CS(CompositProcess process) const
        {
            if(process == CompositProcess::ggH_hh_bbgammagamma) {
                return 2.0 * GetCrossSection(Process::gg_H)
                           * GetBranchingRatio(Process::H_hh)
                           * GetBranchingRatio(Process::h_bb)
                           * GetBranchingRatio(Process::h_gammagamma);
            }
            else if(process == CompositProcess::ggH_hh_bbtautau) {
                return 2.0 * GetCrossSection(Process::gg_H)
                           * GetBranchingRatio(Process::H_hh)
                           * GetBranchingRatio(Process::h_bb)
                           * GetBranchingRatio(Process::h_tautau);
            }
            throw exception("Unsupported process '%1%'.") % process;
        }

    private:
        template<typename Map>
        double GetValue(const Map& map, const typename Map::key_type& key, const std::string& param_name) const
        {
            if(!map.count(key))
                throw exception("%1% %2% not found for the parameter phase space point %3% = %4%.")
                            % key % param_name % GetParamNameString() % ToString();
            return map.at(key);
        }
    };

    typedef std::list<PointDescriptor> PointDescriptorCollection;

    Model(const StrVec& _param_names)
        : param_names(_param_names), param_names_str(VectorToString(_param_names)) {}

    const PointDescriptorCollection& GetAvailablePoints() const { return points; }
    size_t GetNumberOfDimensions() const { return param_names.size(); }

    const std::string& GetParamName(size_t dim) const
    {
        if(!dim || dim > param_names.size())
            throw exception("Model parameter phase space do not contain dimension %1%") % dim;
        return param_names.at(dim - 1);
    }

    const std::string& GetParamNameString() const { return param_names_str; }

    template<typename Vector>
    static std::string VectorToString(const Vector& v)
    {
        return "(" + CollectionToString(v) + ")";
    }

protected:
    PointDescriptorCollection points;

private:
    StrVec param_names;
    std::string param_names_str;
};

class ModelReader {
public:
    virtual ~ModelReader() {}
    virtual Model::PointDescriptorCollection CollectAvailablePoints() const = 0;
    virtual std::shared_ptr<TH2F> GetReferenceHistogram() const = 0;
};

template<typename _Histogram>
class ModelReader_Hist2D : public ModelReader {
public:
    typedef _Histogram Hist;
    typedef std::shared_ptr<Hist> HistPtr;

    ModelReader_Hist2D(const std::string& file_name, size_t _version)
        : file(new TFile(file_name.c_str(), "READ")), version(_version)
    {
        static const std::map<Particle, std::vector<std::string>> Mass_dictionary {
            { Particle::H, { "h_mH", "m_H", "mH" } },
            { Particle::A, { "h_mA", "m_A", "mA" } },
            { Particle::h, { "h_mh", "m_h", "mh" } }
        };

        static const std::map<Process, std::vector<std::string>> CS_dictionary {
            { Process::gg_H, { "h_ggF_xsec_H", "xs_gg_H", "xs_ggH" } }
        };

        static const std::map<Process, std::vector<std::string>> BR_dictionary {
            { Process::H_hh, { "h_brh0h0_H", "br_H_hh", "br_Hhh" } },
            { Process::h_gammagamma, { "h_brgammagamma_h", "br_h_gamgam", "br_hgammagamma" } },
            { Process::h_bb, { "h_brbb_h", "br_h_bb", "br_hbb" } },
            { Process::h_tautau, { "h_brtautau_h", "br_h_tautau", "br_htautau" } }
        };

        ReadAllHistograms(Mass_dictionary, masses);
        ReadAllHistograms(CS_dictionary, cross_sections);
        ReadAllHistograms(BR_dictionary, branching_ratios);
        if(!all_hists.size())
            throw exception("No histograms is read.");
        if(!CheckHistsCompatibility(all_hists))
            throw exception("Not all model histograms have compatible binning");
    }

    virtual Model::PointDescriptorCollection CollectAvailablePoints() const override
    {
        Model::PointDescriptorCollection points;
        const auto ref_hist = *all_hists.begin();
        for(Int_t x_id = 1; x_id <= ref_hist->GetXaxis()->GetNbins(); ++x_id) {
            const double x = ref_hist->GetXaxis()->GetBinCenter(x_id);
            for(Int_t y_id = 1; y_id <= ref_hist->GetYaxis()->GetNbins(); ++y_id) {
                const double y = ref_hist->GetYaxis()->GetBinCenter(y_id);
                Model::PointDescriptor desc;
                desc.point = Model::Point({x, y});
                for(const auto& entry : masses)
                    desc.masses[entry.first] = entry.second->GetBinContent(x_id, y_id);
                for(const auto& entry : cross_sections)
                    desc.cross_sections[entry.first] = entry.second->GetBinContent(x_id, y_id);
                for(const auto& entry : branching_ratios)
                    desc.branching_ratios[entry.first] = entry.second->GetBinContent(x_id, y_id);
                points.push_back(desc);
            }
        }
        return points;
    }

    virtual std::shared_ptr<TH2F> GetReferenceHistogram() const override
    {
        return *all_hists.begin();
    }

private:
    HistPtr GetHist(const std::string& hist_name) const
    {
        HistPtr hist(dynamic_cast<Hist*>(file->Get(hist_name.c_str())));
        if(!hist)
            throw exception("Can't read histogram '%1%' from model file '%2%'.")
                % hist_name % file->GetName();
        return hist;
    }

    template<typename NameMap, typename OutputMap>
    void ReadAllHistograms(const NameMap& dictionary, OutputMap& values)
    {
        for(const auto& entry : dictionary) {
            const std::string& hist_name = entry.second.at(version);
            if(!hist_name.size()) continue;
            const auto hist = GetHist(hist_name);
            values[entry.first] = hist;
            all_hists.push_back(hist);
        }
    }

    template<typename HistContainer>
    static bool CheckHistsCompatibility(const HistContainer& hists)
    {
        if(!hists.size()) return true;
        auto ref_iter = hists.begin();
        for(auto hist_iter = std::next(ref_iter); hist_iter != hists.end(); ++hist_iter) {
            if(!CheckAxisCompatibility(*(*ref_iter)->GetXaxis(), *(*hist_iter)->GetXaxis())
                || !CheckAxisCompatibility(*(*ref_iter)->GetYaxis(), *(*hist_iter)->GetYaxis()))
                return false;
        }
        return true;
    }

    static bool CheckAxisCompatibility(const TAxis& a, const TAxis& b)
    {
        static const double delta_max = 0.001;
        static const double rel_delta_max = 0.01;

        if(a.GetNbins() != b.GetNbins()) return false;
        for(Int_t n = 1; n <= a.GetNbins(); ++n) {
            const double c_a = a.GetBinCenter(n);
            const double c_b = b.GetBinCenter(n);
            const double delta = c_a - c_b;
            if( ( c_a && std::abs(delta/c_a) > rel_delta_max ) || ( !c_a && std::abs(delta) > delta_max ) )
                return false;
        }
        return true;
    }

private:
    std::shared_ptr<TFile> file;
    size_t version;
    std::map<Particle, HistPtr> masses;
    std::map<Process, HistPtr> cross_sections, branching_ratios;
    std::list<HistPtr> all_hists;
};

class ModelReaderFactory {
public:
    static std::shared_ptr<ModelReader> Make(const std::string& file_name, size_t version)
    {
        if(version >= 3)
            throw exception("Model file version %1% is not supported.") % version;
        return std::shared_ptr<ModelReader>(new ModelReader_Hist2D<TH2F>(file_name, version));
    }
private:
    ModelReaderFactory() {}
};

class Model_MSSM : public Model {
public:
    Model_MSSM(const PointDescriptorCollection& _points)
        : Model({ "m_A", "tan_beta" })
    {
        points = _points;
    }
};

class Model_2HDM : public Model {
public:
    Model_2HDM(const PointDescriptorCollection& _points)
        : Model({ "m_H", "tan_beta" })
    {
        points = _points;
    }
};


class ModelFactory {
public:
    static std::shared_ptr<Model> Make(const std::string& name, std::shared_ptr<ModelReader> reader)
    {
        const auto points = reader->CollectAvailablePoints();
        if(name == "MSSM")
            return std::shared_ptr<Model>(new Model_MSSM(points));
        else if(name == "2HDM")
            return std::shared_ptr<Model>(new Model_2HDM(points));
        throw exception("Model name '%1%' is not supported.") % name;
    }

private:
    ModelFactory() {}
};

std::shared_ptr<TGraph2D> CreateTGraph2D(const std::string& name, const std::vector<double>& x_list,
                                         const std::vector<double>& y_list, const std::vector<double>& z_list)
{
    std::vector<double> x_list_copy(x_list), y_list_copy(y_list), z_list_copy(z_list);
    return std::shared_ptr<TGraph2D>(new TGraph2D(name.c_str(), name.c_str(), x_list.size(), x_list_copy.data(),
                                                  y_list_copy.data(), z_list_copy.data()));
}

std::shared_ptr<TH2D> CreateTH2D(const std::string& name, std::shared_ptr<TH2F> ref_hist)
{
    const auto x_bins = ref_hist->GetXaxis()->GetXbins(), y_bins = ref_hist->GetYaxis()->GetXbins();
    return std::shared_ptr<TH2D>(new TH2D(name.c_str(), name.c_str(), x_bins->GetSize() - 1, x_bins->GetArray(),
                                          y_bins->GetSize() - 1, y_bins->GetArray()));
}

void CreateOutput(const std::vector<double>& x_list, const std::vector<double>& y_list,
                  const std::vector<double>& z_list, std::shared_ptr<TFile> output_file, const std::string& name,
                  const std::shared_ptr<TH2F> ref_hist, double excl_threshold = -std::numeric_limits<double>::infinity(),
                  double graph_factor = 1, double graph_max = std::numeric_limits<double>::infinity(),
                  bool interpolate = false)
{
    std::ostringstream ss_name;
    ss_name << name << "_hist";
    auto hist = CreateTH2D(ss_name.str(), ref_hist);
    ss_name << "_excl";
    auto hist_excl = CreateTH2D(ss_name.str(), ref_hist);

    if(interpolate) {
        auto ref_graph = CreateTGraph2D("ref_tmp", x_list, y_list, z_list);
        for(Int_t x_id = 1; x_id <= hist->GetXaxis()->GetNbins(); ++x_id) {
            const double x = hist->GetXaxis()->GetBinCenter(x_id);
            for(Int_t y_id = 1; y_id <= hist->GetYaxis()->GetNbins(); ++y_id) {
                const double y = hist->GetYaxis()->GetBinCenter(y_id);
                const double z = ref_graph->Interpolate(x, y);
                hist->SetBinContent(x_id, y_id, z);
                if(z < excl_threshold)
                    hist_excl->SetBinContent(x_id, y_id, 1.0);
            }
        }
    } else {
        for(size_t n = 0; n < z_list.size(); ++n) {
            const Int_t bin_id = hist->FindBin(x_list.at(n), y_list.at(n));
            const double z =  z_list.at(n);
            hist->SetBinContent(bin_id, z);
            if(z < excl_threshold)
                hist_excl->SetBinContent(bin_id, 1.0);
        }
    }

    std::vector<double> z_list_graph(z_list.size());
    for(size_t n = 0; n < z_list.size(); ++n)
        z_list_graph.at(n) = std::min(graph_max, z_list.at(n) * graph_factor);
    auto graph = CreateTGraph2D(name, x_list, y_list, z_list_graph);

    output_file->WriteTObject(graph.get(), nullptr, "Overwrite");
    output_file->WriteTObject(hist.get(), nullptr, "Overwrite");
    output_file->WriteTObject(hist_excl.get(), nullptr, "Overwrite");
}

struct Arguments : run::ArgumentsBase {
    StrArg input{ "input", "input directory with limit files" };
    StrArg output{ "output", "output ROOT file with model dependent interpretation" };
    StrArg model{ "model", "physical model", "MSSM" };
    StrArg model_file{ "model-file", "ROOT file with model description" };
    Arg<size_t> model_file_version{ "model-file-version", "Version of the model file", 1 };
    Arg<CompositProcess> process{ "process", "process name" };
    Arg<Range<double>> range_x{ "range-x", "x range in format min:max" };
    Arg<Range<double>> range_y{ "range-y", "y range in format min:max" };
    Arg<Units> units{ "units", "units in which limits are given", Units::pb };
    Arg<bool> json{ "json", "input is a json file with limits", false};
};

class SimpleHHInterpret {
public:
    using Range = ::analysis::Range<double>;
    using RangeMultiD = ::analysis::RangeMultiD<Range>;
    using InterpolatorVec = std::vector<InterpolatorPtr>;

    SimpleHHInterpret(const Arguments& _args) : args(_args) {}

    void Run()
    {
        const RangeMultiD param_range({ args.range_x(), args.range_y() });
        Range m_H_range;
        const auto& limits = ReadInterpolatedLimits(args.input(), m_H_range, args.units());
        const auto model_reader = ModelReaderFactory::Make(args.model_file(), args.model_file_version());
        const auto model = ModelFactory::Make(args.model(), model_reader);
        std::vector<double> x_list, y_list, z_list_pred;
        std::vector<std::vector<double>> z_list(limits.size());

        for(const auto& desc : model->GetAvailablePoints()) {
            if(!param_range.Contains(desc.point)) continue;
            const double th_predicted = std::max(desc.GetProcessBR_CS(args.process()), 0.0);
            const double m_H = desc.GetMass(Particle::H);
            if(!m_H_range.Contains(m_H) || !th_predicted) continue;
            x_list.push_back(desc.point.at(0));
            y_list.push_back(desc.point.at(1));
            z_list_pred.push_back(th_predicted);
            for(size_t n = 0; n < limits.size(); ++n) {
                const double r = limits.at(n)->Eval(m_H) / th_predicted;
                z_list.at(n).push_back(r);
            }
        }

        const auto ref_hist = model_reader->GetReferenceHistogram();
        auto output_file = root_ext::CreateRootFile(args.output());
        for(size_t n = 0; n < limits.size(); ++n)
            CreateOutput(x_list, y_list, z_list.at(n), output_file, all_limit_quantile_names.at(n), ref_hist,
                         1.0, 0.05, 1.0);
        CreateOutput(x_list, y_list, z_list_pred, output_file, "predicted_CS_BR", ref_hist);

        std::cout << "File '" << args.output() << "' successfully created.\n";
    }

private:
    InterpolatorVec ReadInterpolatedLimits(const std::string& input_dir_name, Range& mass_range, Units units)
    {
        if(args.json()) return ReadInterpolatedLimitsFromJson(input_dir_name, mass_range, units);

        const auto& file_names = GetOrderedFileList(input_dir_name, ".*\\.root");
        std::vector<std::vector<double>> limits(all_limit_quantiles.size());
        std::set<double> masses;
        const double units_factor = GetUnitsFactor(units);

        if(!file_names.size())
            throw exception("No input files are found.");

        for(const auto& file_entry : file_names) {
            const std::string& file_name = file_entry.second;
            const double mass = file_entry.first;
            std::cout << "Reading " << file_name << std::endl;
            if(masses.count(mass))
                throw exception("More than one file with for the mass point = %1%") % mass;

            masses.insert(mass);
            std::shared_ptr<TFile> file(new TFile(file_name.c_str(), "READ"));
            std::shared_ptr<TTree> tree(dynamic_cast<TTree*>(file->Get("limit")));
            double limit;
            float quantileExpected;
            tree->SetBranchAddress("limit", &limit);
            tree->SetBranchAddress("quantileExpected", &quantileExpected);
            for(Long64_t n = 0; n < tree->GetEntries(); ++n) {
                tree->GetEntry(n);
                const size_t quantile_id = GetQuantileId(quantileExpected);
                limits.at(quantile_id).push_back(limit * units_factor);
            }
        }

        const std::vector<double> mass_vec(masses.begin(), masses.end());
        mass_range = Range(mass_vec.front(), mass_vec.back());
        InterpolatorVec interps;
        for(size_t n = 0; n < limits.size(); ++n) {
            if(limits[n].size() != masses.size())
                throw exception("Inconsistent input limits.");
            InterpolatorPtr interp(new ROOT::Math::Interpolator(mass_vec, limits[n], ROOT::Math::Interpolation::kCSPLINE));
            interps.push_back(interp);
        }
        return interps;
    }

    InterpolatorVec ReadInterpolatedLimitsFromJson(const std::string& limit_file, Range& mass_range, Units units)
    {
        boost::property_tree::ptree property_tree;
        boost::property_tree::json_parser::read_json(limit_file, property_tree);

        std::vector<std::vector<double>> limits(all_limit_quantiles.size());
        std::set<double> masses;
        const double units_factor = GetUnitsFactor(units);

        for(const auto& entry : property_tree) {
            const double mass = std::stod(entry.first);
            std::cout << "Adding " << mass << std::endl;
            if(masses.count(mass))
                throw exception("More than one file with for the mass point = %1%") % mass;
            masses.insert(mass);
            for(const auto& limit_entry : entry.second) {
                const size_t quantile_id = GetQuantileId(limit_entry.first);
                const double limit = limit_entry.second.get_value<double>();
                limits.at(quantile_id).push_back(limit * units_factor);
            }
        }

        const std::vector<double> mass_vec(masses.begin(), masses.end());
        mass_range = Range(mass_vec.front(), mass_vec.back());
        InterpolatorVec interps;
        for(size_t n = 0; n < limits.size(); ++n) {
            if(!limits[n].size())
                limits[n] = limits[GetQuantileId("exp0")];
            if(limits[n].size() != masses.size())
                throw exception("Inconsistent input limits.");
            InterpolatorPtr interp(new ROOT::Math::Interpolator(mass_vec, limits[n], ROOT::Math::Interpolation::kCSPLINE));
            interps.push_back(interp);
        }
        return interps;
    }

private:
    Arguments args;
};

} // namespace hh_analysis

PROGRAM_MAIN(hh_analysis::SimpleHHInterpret, hh_analysis::Arguments)
