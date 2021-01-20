#!/usr/bin/env python

import os
import argparse
import json
import ROOT
import re

ROOT.gROOT.SetBatch(True)

parser = argparse.ArgumentParser(description='Draw sm limits.',
                  formatter_class = lambda prog: argparse.HelpFormatter(prog, max_help_position=30, width=90))
parser.add_argument('--input', required=True, type=str, metavar='PATH', help="input path")
args = parser.parse_args()


def DrawCMSLogo(pad, cmsText, extraText, iPosX, relPosX, relPosY, relExtraDY, extraText2='', cmsTextSize=0.8):
    """Blah

    Args:
        pad (TYPE): Description
        cmsText (TYPE): Description
        extraText (TYPE): Description
        iPosX (TYPE): Description
        relPosX (TYPE): Description
        relPosY (TYPE): Description
        relExtraDY (TYPE): Description
        extraText2 (str): Description
        cmsTextSize (float): Description

    Returns:
        TYPE: Description
    """
    pad.cd()
    cmsTextFont = 62  # default is helvetic-bold

    writeExtraText = len(extraText) > 0
    writeExtraText2 = len(extraText2) > 0
    extraTextFont = 52

    # text sizes and text offsets with respect to the top frame
    # in unit of the top margin size
    lumiTextOffset = 0.2
    # cmsTextSize = 0.8
    # float cmsTextOffset    = 0.1;  // only used in outOfFrame version

    # ratio of 'CMS' and extra text size
    extraOverCmsTextSize = 0.76

    outOfFrame = False
    if iPosX / 10 == 0:
        outOfFrame = True

    alignY_ = 3
    alignX_ = 2
    if (iPosX / 10 == 0):
        alignX_ = 1
    if (iPosX == 0):
        alignX_ = 1
    if (iPosX == 0):
        alignY_ = 1
    if (iPosX / 10 == 1):
        alignX_ = 1
    if (iPosX / 10 == 2):
        alignX_ = 2
    if (iPosX / 10 == 3):
        alignX_ = 3
    # if (iPosX == 0): relPosX = 0.14
    align_ = 10 * alignX_ + alignY_

    l = pad.GetLeftMargin()
    t = pad.GetTopMargin()
    r = pad.GetRightMargin()
    b = pad.GetBottomMargin()

    latex = ROOT.TLatex()
    latex.SetNDC()
    latex.SetTextAngle(0)
    latex.SetTextColor(ROOT.kBlack)

    extraTextSize = extraOverCmsTextSize * cmsTextSize
    pad_ratio = (float(pad.GetWh()) * pad.GetAbsHNDC()) / \
        (float(pad.GetWw()) * pad.GetAbsWNDC())
    if (pad_ratio < 1.):
        pad_ratio = 1.

    if outOfFrame:
        latex.SetTextFont(cmsTextFont)
        latex.SetTextAlign(11)
        latex.SetTextSize(cmsTextSize * t * pad_ratio)
        latex.DrawLatex(l, 1 - t + lumiTextOffset * t, cmsText)

    posX_ = 0
    if iPosX % 10 <= 1:
        posX_ = l + relPosX * (1 - l - r)
    elif (iPosX % 10 == 2):
        posX_ = l + 0.5 * (1 - l - r)
    elif (iPosX % 10 == 3):
        posX_ = 1 - r - relPosX * (1 - l - r)

    posY_ = 1 - t - relPosY * (1 - t - b)
    if not outOfFrame:
        latex.SetTextFont(cmsTextFont)
        latex.SetTextSize(cmsTextSize * t * pad_ratio)
        latex.SetTextAlign(align_)
        latex.DrawLatex(posX_, posY_, cmsText)
        if writeExtraText:
            latex.SetTextFont(extraTextFont)
            latex.SetTextAlign(align_)
            latex.SetTextSize(extraTextSize * t * pad_ratio)
            latex.DrawLatex(
                posX_, posY_ - relExtraDY * cmsTextSize * t, extraText)
            if writeExtraText2:
                latex.DrawLatex(
                    posX_, posY_ - 1.8 * relExtraDY * cmsTextSize * t, extraText2)
    elif writeExtraText:
        if iPosX == 0:
            posX_ = l + relPosX * (1 - l - r)
            posY_ = 1 - t + lumiTextOffset * t
        latex.SetTextFont(extraTextFont)
        latex.SetTextSize(extraTextSize * t * pad_ratio)
        latex.SetTextAlign(align_)
        latex.DrawLatex(posX_, posY_, extraText)

def PositionedLegend(width, height, pos, offset):
    o = offset
    w = width
    h = height
    l = ROOT.gPad.GetLeftMargin()
    t = ROOT.gPad.GetTopMargin()
    b = ROOT.gPad.GetBottomMargin()
    r = ROOT.gPad.GetRightMargin()
    if pos == 1:
        return ROOT.TLegend(l + o, 1 - t - o - h, l + o + w, 1 - t - o, '', 'NBNDC')
    if pos == 2:
        c = l + 0.5 * (1 - l - r)
        return ROOT.TLegend(c - 0.5 * w, 1 - t - o - h, c + 0.5 * w, 1 - t - o, '', 'NBNDC')
    if pos == 3:
        return ROOT.TLegend(1 - r - o - w, 1 - t - o - h, 1 - r - o, 1 - t - o, '', 'NBNDC')
    if pos == 4:
        return ROOT.TLegend(l + o, b + o, l + o + w, b + o + h, '', 'NBNDC')
    if pos == 5:
        c = l + 0.5 * (1 - l - r)
        return ROOT.TLegend(c - 0.5 * w, b + o, c + 0.5 * w, b + o + h, '', 'NBNDC')
    if pos == 6:
        return ROOT.TLegend(1 - r - o - w, b + o, 1 - r - o, b + o + h, '', 'NBNDC')


legend_dict = {
    'obs' : { 'Label' : 'Observed', 'LegendStyle' : 'LP', 'DrawStyle' : 'PLSAME'},
    'exp0' : { 'Label' : 'Expected', 'LegendStyle' : 'L', 'DrawStyle' : 'LSAME'},
    'exp1' : { 'Label' : '#pm1#sigma Expected', 'LegendStyle' : 'F', 'DrawStyle' : '3SAME'},
    'exp2' : { 'Label' : '#pm2#sigma Expected', 'LegendStyle' : 'F', 'DrawStyle' : '3SAME'}
}

output = '{}/limits_sm.pdf'.format(args.input)

VBF = True

patterns = [ ]
labels = []
maximums = []

years = [ 2016, 2017, 2018 ]
channels = [ 'eTau', 'muTau', 'tauTau' ]

patterns.append( [ '{}_cmb'.format(y) for y in years ] + ['cmb'] )
labels.append( [str(y) for y in years] + ['Run 2'] )
maximums.append(400 if not VBF else 4000)

patterns.append( [ '{}'.format(c) for c in channels ] + ['cmb'] )
labels.append( ['e#tau', '#mu#tau', '#tau#tau', 'Run 2' ] )
maximums.append(400 if not VBF else 4000)

for year in years:
    patterns.append( [ '{}_{}'.format(year, c) for c in channels ] + ['{}_cmb'.format(year)] )
    labels.append( ['e#tau', '#mu#tau', '#tau#tau', '{}'.format(year) ] )
    if year == 2017:
        maximums.append(4000 if not VBF else 10000)
    else:
        maximums.append(1000 if not VBF else 10000)

obs = False

c1 = ROOT.TCanvas("", "", 700, 700)
c1.SetGridx()
c1.SetFrameLineWidth(2)
c1.SetLeftMargin(0.15)
c1.SetRightMargin(0.05)
c1.SetTopMargin(0.05)
c1.SetTicky(1)
c1.SetLogy()
ROOT.gStyle.SetEndErrorSize(0)

c1.Draw()
c1.cd()

c1.Print(output + '[')

with open('{}/all_limits.json'.format(args.input), 'r') as f:
    limits_data = json.load(f)

for pattern_id,pattern in enumerate(patterns):
    has_all_keys = True

    for key in pattern:
        if key not in limits_data:
            has_all_keys = False
    if not has_all_keys:
        continue

    n_bins = len(pattern)

    frame = ROOT.TH1F('hframe', '', n_bins, 0, n_bins)
    frame.SetMinimum(1 if not VBF else 100)
    frame.SetMaximum(maximums[pattern_id])
    frame.SetStats(0)
    frame.GetXaxis().SetNdivisions(n_bins)
    for n,key in enumerate(pattern):
        frame.GetXaxis().SetBinLabel(n+1, labels[pattern_id][n])
    frame.GetXaxis().SetLabelSize(0.1)
    frame.GetXaxis().SetLabelOffset(0.01)

    frame.GetYaxis().SetTicks("+")
    frame.GetYaxis().SetLabelOffset(-0.04)
    frame.GetYaxis().SetLabelSize(0.03)
    frame.GetYaxis().SetTitle("95% CL limit on #sigma / #sigma_{SM}")
    frame.GetYaxis().SetTitleOffset(-1.2)
    frame.GetYaxis().SetTitleSize(0.05)

    frame.Draw()

    graphs = {}
    if obs:
        graphs['obs'] = ROOT.TGraphAsymmErrors()
    graphs['exp0'] = ROOT.TGraphAsymmErrors()
    graphs['exp1'] = ROOT.TGraphAsymmErrors()
    graphs['exp2'] = ROOT.TGraphAsymmErrors()
    width = 0.95
    for n,key in enumerate(pattern):
        data = limits_data[key]
        if obs:
            graphs['obs'].SetPoint(n, n+0.5, data['obs'])
        exp = data['exp0']
        graphs['exp0'].SetPoint(n, n+0.5, exp)
        graphs['exp1'].SetPoint(n, n+0.5, exp)
        graphs['exp2'].SetPoint(n, n+0.5, exp)

        if obs:
            graphs['obs'].SetPointEXlow(n, width/2)
            graphs['obs'].SetPointEXhigh(n, width/2)

        graphs['exp0'].SetPointEXlow(n, width/2)
        graphs['exp0'].SetPointEXhigh(n, width/2)
        graphs['exp1'].SetPointEXlow(n, width/2)
        graphs['exp1'].SetPointEXhigh(n, width/2)
        graphs['exp2'].SetPointEXlow(n, width/2)
        graphs['exp2'].SetPointEXhigh(n, width/2)

        graphs['exp1'].SetPointEYlow(n, exp - data['exp-1'])
        graphs['exp1'].SetPointEYhigh(n, data['exp+1'] - exp)
        graphs['exp2'].SetPointEYlow(n, exp - data['exp-2'])
        graphs['exp2'].SetPointEYhigh(n, data['exp+2'] - exp)

    if obs:
        graphs['obs'].SetMarkerColor(ROOT.kBlack)
        graphs['obs'].SetMarkerStyle(8)
        graphs['obs'].SetMarkerSize(0.8)

    graphs['exp0'].SetLineColor(ROOT.kRed)
    graphs['exp0'].SetMarkerColor(ROOT.kRed)
    graphs['exp1'].SetFillColor(ROOT.kGreen)
    graphs['exp2'].SetFillColor(ROOT.kYellow)

    graphs['exp2'].Draw("e2 same")
    graphs['exp1'].Draw("e2 same")
    graphs['exp0'].Draw("p same")
    if obs:
        graphs['obs'].Draw("p same")

    legend = PositionedLegend(0.3, 0.2, 3, 0.015)

    for key in legend_dict:
        if key in graphs:
            legend.AddEntry(graphs[key], legend_dict[key]['Label'], legend_dict[key]['LegendStyle'])

    legend.Draw()

    DrawCMSLogo(c1, 'CMS', "Internal", 11, 0.045, 0.035, 1.2, '', 1.2)

    c1.Print(output)
c1.Print(output + ']')
