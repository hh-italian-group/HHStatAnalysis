/*! Shared library exports.
This file is part of https://github.com/cms-hh/StatAnalysis. */

#include "HHStatAnalysis/Run2_2016/interface/ttbb_resonant.h"
#include "HHStatAnalysis/Run2_2016/interface/ttbb_nonresonant.h"
#include "HHStatAnalysis/Run2_2016/interface/bbbb_nonresonant.h"

#define MAKE_MODEL(model) if(name == #model) return std::make_shared<model>(*model_descriptor, shapes_file_name)

extern "C" hh_analysis::stat_models::StatModelPtr create_stat_model(const char* stat_model_name,
                const hh_analysis::StatModelDescriptor* model_descriptor, const char* shapes_file_name)
{
    using namespace hh_analysis::stat_models::Run2_2016;

    if(!stat_model_name || !model_descriptor || !shapes_file_name)
        throw analysis::exception("Null pointer is passed to create_stat_model function.");

    const std::string name(stat_model_name);
    MAKE_MODEL(ttbb_resonant);
    MAKE_MODEL(ttbb_nonresonant);
    MAKE_MODEL(bbbb_nonresonant);
    throw analysis::exception("Stat model '%1%' not found.") % name;
}
