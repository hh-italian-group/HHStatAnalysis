/*! Definition of physical constants used in HH analyses.
This file is part of https://github.com/cms-hh/HHStatAnalysis. */

#pragma once

namespace hh_analysis {
namespace phys_const {

constexpr double pb = 1;
constexpr double fb = 1e-3;

constexpr double BR_H_tautau = 6.256e-02;
constexpr double BR_H_bb = 5.809e-01;
constexpr double BR_HH_bbbb = BR_H_bb * BR_H_bb;
constexpr double BR_HH_bbtautau = 2 * BR_H_tautau * BR_H_bb;
constexpr double XS_HH_13TeV = 33.53 * fb; // m_h = 125 GeV arxiv:1610.07922 Table 53

} // namespace phys_const
} // namespace hh_analysis
