/*! Python interface for HH stat models.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include "HHStatAnalysis/StatModels/interface/Config.h"

namespace {
template <typename T>
boost::python::list ToPythonList(const std::vector<T>& v)
{
    boost::python::list l;
    for(const auto& item : v)
        l.append(item);
    return l;
}

template <typename Key, typename Value>
boost::python::dict ToPythonDict(const std::map<Key, Value>& m)
{
    boost::python::dict d;
    for(const auto& item : m)
        d[item.first] = item.second;
    return d;
}
} // anonymous namespace

BOOST_PYTHON_MODULE(libHHStatAnalysisStatModels)
{
    using StrVector = std::vector<std::string>;
    using StrStrMap = std::map<std::string, std::string>;
    using StrVectMap = std::map<std::string, StrVector>;
    using namespace boost::python;
    using namespace hh_analysis;
    class_<StrVector>("StrVector")
        .def(vector_indexing_suite<StrVector>());
    class_<StrStrMap>("StrStrMap")
        .def(map_indexing_suite<StrStrMap>());
    class_<StrVectMap>("StrVectMap")
        .def(map_indexing_suite<StrVectMap>());
    enum_<LimitType>("LimitType")
        .value("model_independent", LimitType::ModelIndependent)
        .value("SM", LimitType::SM)
        .value("MSSM", LimitType::MSSM)
        .value("NonResonant_BSM", LimitType::NonResonant_BSM);
    class_<Range<double>>("Range", init<>())
        .def(init<double, double>())
        .def("min", &Range<double>::min)
        .def("max", &Range<double>::max);
    class_<RangeWithStep<double>>("RangeWithStep", init<>())
        .def(init<double, double, double>())
        .def("min", &RangeWithStep<double>::min)
        .def("max", &RangeWithStep<double>::max)
        .def("step", &RangeWithStep<double>::step);
    class_<StatModelDescriptor>("StatModelDescriptor")
        .def_readwrite("name", &StatModelDescriptor::name)
        .def_readwrite("stat_model", &StatModelDescriptor::stat_model)
        .def_readwrite("years", &StatModelDescriptor::years)
        .def_readwrite("channels", &StatModelDescriptor::channels)
        .def_readwrite("categories", &StatModelDescriptor::categories)
        .def_readwrite("signal_processes", &StatModelDescriptor::signal_processes)
        .def_readwrite("model_signal_process", &StatModelDescriptor::model_signal_process)
        .def_readwrite("signal_point_prefix", &StatModelDescriptor::signal_point_prefix)
        .def_readwrite("signal_points", &StatModelDescriptor::signal_points)
        .def_readwrite("limit_type", &StatModelDescriptor::limit_type)
        .def_readwrite("th_model_file", &StatModelDescriptor::th_model_file)
        .def_readwrite("blind", &StatModelDescriptor::blind)
        .def_readwrite("morph", &StatModelDescriptor::morph)
        .def_readwrite("combine_channels", &StatModelDescriptor::combine_channels)
        .def_readwrite("per_channel_limits", &StatModelDescriptor::per_channel_limits)
        .def_readwrite("per_category_limits", &StatModelDescriptor::per_category_limits)
        .def_readwrite("grid_x", &StatModelDescriptor::grid_x)
        .def_readwrite("grid_y", &StatModelDescriptor::grid_y)
        .def_readwrite("label_status", &StatModelDescriptor::label_status)
        .def_readwrite("label_scenario", &StatModelDescriptor::label_scenario)
        .def_readwrite("label_lumi", &StatModelDescriptor::label_lumi)
        .def_readwrite("title_x", &StatModelDescriptor::title_x)
        .def_readwrite("title_y", &StatModelDescriptor::title_y)
        .def_readwrite("draw_range_x", &StatModelDescriptor::draw_range_x)
        .def_readwrite("draw_range_y", &StatModelDescriptor::draw_range_y)
        .def_readwrite("draw_mh_exclusion", &StatModelDescriptor::draw_mh_exclusion)
        .def_readwrite("draw_mH_isolines", &StatModelDescriptor::draw_mH_isolines)
        .def_readwrite("iso_label_draw_margin", &StatModelDescriptor::iso_label_draw_margin)
        .def_readwrite("custom_params", &StatModelDescriptor::custom_params);
    def("LoadDescriptor", LoadDescriptor);
    def("ToList", ToPythonList<std::string>);
    def("ToDict", ToPythonDict<std::string, std::string>);
}
