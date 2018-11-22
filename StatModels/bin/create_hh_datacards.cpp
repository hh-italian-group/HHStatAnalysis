/*! Tool to create HH datacards.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#include <dlfcn.h>
#include "HHStatAnalysis/Core/interface/program_main.h"
#include "HHStatAnalysis/Core/interface/exception.h"
#include "HHStatAnalysis/StatModels/interface/Config.h"
#include "HHStatAnalysis/StatModels/interface/StatModel.h"

namespace {

struct Arguments {
    run::Argument<std::string> cfg{"cfg", "configuration file"};
    run::Argument<std::string> model_desc{"model-desc", "name of the stat model descriptor in the config"};
    run::Argument<std::string> shapes{"shapes", "file with input shapes"};
    run::Argument<std::string> output_path{"output", "path where to store created datacards"};
    run::Argument<std::string> signal_point{"signal-point", "run on a single signal point", ""};
};

} // anonymous namespace

namespace hh_analysis {

class CreateHHDatacards {
public:
    CreateHHDatacards(const Arguments& _args) : args(_args) {}

    void Run()
    {
        static const std::string creator_fn_name = "create_stat_model";
        StatModelDescriptor model_desc = LoadDescriptor(args.cfg(), args.model_desc());
        if(args.signal_point().size()) {
            model_desc.signal_points.clear();
            model_desc.signal_points.push_back(args.signal_point());
        }
        const auto stat_model_ref = SplitValueList(model_desc.stat_model, true, "/");
        if(stat_model_ref.size() != 2)
            throw exception("Bad stat model name '%1%'") % model_desc.stat_model;

        const std::string library_name = boost::str(boost::format("libHHStatAnalysis%1%.so") % stat_model_ref.at(0));
        const std::string stat_model_name = stat_model_ref.at(1);
        void* handle = dlopen(library_name.c_str(), RTLD_LAZY);
        if(!handle)
            throw exception("Unknown library reference '%1%' in the model descriptor."
                            " Stat model library '%2%' not found.") % stat_model_ref.at(0) % library_name;
        auto creator = (stat_models::StatModelCreator) dlsym(handle, creator_fn_name.c_str());
        if(!creator)
            throw exception("Unable to load %1% function from %2%.") % creator_fn_name % library_name;
        auto model = creator(stat_model_name.c_str(), &model_desc, args.shapes().c_str());
        if(!model)
            throw exception("Unable to create an object for stat model '%1%'.") % model_desc.stat_model;
        std::cout << boost::format("Creating datacards for %1% unc model using %2% shapes...")
                     % model_desc.stat_model % args.shapes() << std::endl;
        std::cout<<"CIAO"<<std::endl;             
        model->CreateDatacards(args.output_path());
        std::cout << boost::format("Datacards are successfully created into '%1%'.") % args.output_path() << std::endl;
    }

private:
    Arguments args;
};

} // namespace hh_analysis

PROGRAM_MAIN(hh_analysis::CreateHHDatacards, Arguments)
