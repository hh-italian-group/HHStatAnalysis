/*! Definition of physical constants used in HH analyses.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#pragma once

namespace hh_analysis {
namespace phys_const {

constexpr double pb = 1;
constexpr double fb = 1e-3;

constexpr double BR_H_tautau = 6.272e-02;
constexpr double BR_H_bb = 5.824e-01;
constexpr double BR_HH_bbbb = BR_H_bb * BR_H_bb;
constexpr double BR_HH_bbtautau = 2 * BR_H_tautau * BR_H_bb;
constexpr double XS_gg_HH_13TeV = 31.05 * fb;
constexpr double XS_VBF_HH_13TeV = 1.726 * fb;

} // namespace phys_const
} // namespace hh_analysis
