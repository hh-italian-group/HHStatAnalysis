/*! Definition of the class that represents shape name rule.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#include "HHStatAnalysis/StatModels/interface/ShapeNameRule.h"

namespace hh_analysis {

const std::string ShapeNameRule::Analysis = "$ANALYSIS";
const std::string ShapeNameRule::Channel = "$CHANNEL";
const std::string ShapeNameRule::Bin = "$BIN";
const std::string ShapeNameRule::Process = "$PROCESS";
const std::string ShapeNameRule::Point = "$POINT";
const std::string ShapeNameRule::Mass = "$MASS";
const std::string ShapeNameRule::Era = "$ERA";
const std::string ShapeNameRule::Systematic = "$SYSTEMATIC";
const std::string ShapeNameRule::Category = "$CATEGORY";
const std::string ShapeNameRule::Region = "$REGION";
const std::string ShapeNameRule::Prefix = "$PREFIX";

const std::set<std::string> ShapeNameRule::AllVariables = {
    Analysis, Channel, Bin, Process, Point, Mass, Era, Systematic, Category, Region, Prefix
};

std::string ShapeNameRule::NumToName(double x)
{
    std::ostringstream ss;
    ss << x;
    std::string str = ss.str();
    // std::replace(str.begin(), str.end(), '-', 'm');
    // std::replace(str.begin(), str.end(), '.', 'p');
    return str;
}

std::string ShapeNameRule::BinName(const std::string& channel, const std::string& category, const std::string& region)
{
    std::ostringstream bin;
    bin << channel << "_" << category;
    if(region.size())
        bin << "_" << region;
    return bin.str();
}

std::string ShapeNameRule::AddDimSuffix(const std::string& variable, size_t dim)
{
    std::ostringstream var;
    var << variable;
    if(dim > 0)
        var << dim;
    return var.str();
}

ShapeNameRule ShapeNameRule::SetVariable(const std::string& variable, const std::string& value) const
{
    std::string new_rule = rule;
    boost::replace_all(new_rule, variable, value);
    return ShapeNameRule(new_rule);
}

ShapeNameRule ShapeNameRule::SetBin(const std::string& channel, const std::string& category,
                                    const std::string& region) const
{
    return SetBin(BinName(channel, category, region));
}

ShapeNameRule ShapeNameRule::SetPrefix(const std::string& prefix, size_t dim) const
{
    return SetVariable(AddDimSuffix(Prefix, dim), prefix);
}

ShapeNameRule ShapeNameRule::SetPoint(double point, size_t dim) const
{
    return SetVariable(AddDimSuffix(Point, dim), NumToName(point));
}

ShapeNameRule ShapeNameRule::AddSystematicVariable() const
{
    std::ostringstream new_rule;
    new_rule << rule << "_" << Systematic;
    return ShapeNameRule(new_rule.str());
}

ShapeNameRule ShapeNameRule::SetSystematic(const std::string& systematic, UncVariation variation) const
{
    std::ostringstream value;
    value << systematic << variation;
    return SetVariable(Systematic, value.str());
}

} // namespace hh_analysis
