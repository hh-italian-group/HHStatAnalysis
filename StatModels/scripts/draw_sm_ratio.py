#!/usr/bin/env python

import os
import argparse
import json
import ROOT
import CombineHarvester.CombineTools.plotting as plot

parser = argparse.ArgumentParser(description='Draw sm limits.',
                  formatter_class = lambda prog: argparse.HelpFormatter(prog, max_help_position=30, width=90))
parser.add_argument('--input', required=True, dest='input', type=str, metavar='PATH', help="input path")
parser.add_argument('--input_ref', required=True, dest='input_ref', type=str, metavar='PATH', help="input path ref ")
args = parser.parse_args()

output = '{}/limits_sm.pdf'.format(args.input)

ROOT.gROOT.SetBatch(True)

c1 = ROOT.TCanvas("", "", 800, 900)
c1.SetGridx()
c1.SetFrameLineWidth(2)
c1.SetLeftMargin(0.1)
c1.SetRightMargin(0.05)
c1.SetTopMargin(0.05)
c1.SetTicky(1)
ROOT.gStyle.SetEndErrorSize(0)

c1.Print(output + '[')

c1.Draw()
c1.cd()


main_pad = ROOT.TPad("main_pad", "", 0.0, 0.35, 1, 1)
main_pad.SetGridx()
main_pad.Draw()
main_pad.cd()
main_pad.SetLogy();

obs = False
n_bins = 4
frame = ROOT.TH1F('hframe', '', n_bins, 0, n_bins)
frame.GetXaxis().SetNdivisions(4)
frame.GetXaxis().SetBinLabel(1, ' ')
frame.GetXaxis().SetBinLabel(2, ' ')
frame.GetXaxis().SetBinLabel(3, ' ')
frame.GetXaxis().SetBinLabel(4, ' ')
frame.SetMinimum(7)
frame.SetMaximum(500)
frame.SetStats(0)



frame.GetYaxis().SetTicks("+")
frame.GetYaxis().SetLabelOffset(-0.04)
frame.GetYaxis().SetLabelSize(0.03)
frame.GetYaxis().SetTitle("95% CL limit on #sigma / #sigma_{SM}")
frame.GetYaxis().SetTitleOffset(-0.9)
frame.GetYaxis().SetTitleSize(0.05)

frame.Draw()

l = ROOT.TLine(0.5,0.5,1,1)
l.Draw()

graphs = {}
if obs:
    graphs['obs'] = ROOT.TGraphAsymmErrors()
graphs['exp0'] = ROOT.TGraphAsymmErrors()
graphs['exp1'] = ROOT.TGraphAsymmErrors()
graphs['exp2'] = ROOT.TGraphAsymmErrors()
n = 0
width = 0.95
for ch in [ 'eTau', 'muTau', 'tauTau', 'cmb' ]:
    with open('{}/limits_{}.json'.format(args.input, ch), 'r') as f:
        res = json.load(f)['1.0']
        if obs:
            graphs['obs'].SetPoint(n, n+0.5, res['obs'])
        exp = res['exp0']
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

        graphs['exp1'].SetPointEYlow(n, exp - res['exp-1'])
        graphs['exp1'].SetPointEYhigh(n, res['exp+1'] - exp)
        graphs['exp2'].SetPointEYlow(n, exp - res['exp-2'])
        graphs['exp2'].SetPointEYhigh(n, res['exp+2'] - exp)
    n += 1

if obs:
    graphs['obs'].SetMarkerColor(ROOT.kBlack)
    graphs['obs'].SetMarkerStyle(8)
    graphs['obs'].SetMarkerSize(0.8)

graphs['exp0'].SetLineColor(ROOT.kRed)
graphs['exp0'].SetMarkerColor(ROOT.kRed)
graphs['exp0'].SetMarkerStyle(8)
graphs['exp1'].SetFillColor(ROOT.kYellow)
graphs['exp2'].SetFillColor(ROOT.kGreen)

graphs['exp2'].Draw("e2 same")
graphs['exp1'].Draw("e2 same")
graphs['exp0'].Draw("p same")
if obs:
    graphs['obs'].Draw("p same")


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

legend = PositionedLegend(0.3, 0.2, 3, 0.015)

for key in legend_dict:
    if key in graphs:
        legend.AddEntry(graphs[key], legend_dict[key]['Label'], legend_dict[key]['LegendStyle'])

legend.Draw()

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
            latex.DrawLatex(posX_, posY_ - relExtraDY * cmsTextSize * t, extraText)
        if writeExtraText2:
            latex.DrawLatex(posX_, posY_ - 1.8 * relExtraDY * cmsTextSize * t, extraText2)
    elif writeExtraText:
        if iPosX == 0:
            posX_ = l + relPosX * (1 - l - r)
            posY_ = 1 - t + lumiTextOffset * t
        latex.SetTextFont(extraTextFont)
        latex.SetTextSize(extraTextSize * t * pad_ratio)
        latex.SetTextAlign(align_)
        latex.DrawLatex(posX_, posY_, extraText)

DrawCMSLogo(c1, 'CMS', "Internal", 11, 0.045, 0.035, 1.2, '', 1.2)


c1.cd()
ratio_pad = ROOT.TPad("ratio_pad", "", 0.0, 0.05, 1, 0.35)
ratio_pad.SetGridx()
ratio_pad.Draw()
ratio_pad.cd()

obs = False
n_bins = 4
frame_1 = ROOT.TH1F('hframe_1', '', n_bins, 0, n_bins)
frame_1.SetMinimum(0.4)
frame_1.SetMaximum(1)
frame_1.SetStats(0)
frame_1.GetXaxis().SetNdivisions(4)
frame_1.GetXaxis().SetBinLabel(1, 'e#tau')
frame_1.GetXaxis().SetBinLabel(2, '#mu#tau')
frame_1.GetXaxis().SetBinLabel(3, '#tau#tau')
frame_1.GetXaxis().SetBinLabel(4, 'cmb')
frame_1.GetXaxis().SetLabelSize(0.15)
frame_1.GetXaxis().SetLabelOffset(0.01)


frame_1.GetYaxis().SetTicks("+")
frame_1.GetYaxis().SetLabelOffset(-0.03)
frame_1.GetYaxis().SetLabelSize(0.05)
frame_1.GetYaxis().SetTitle("Ratio")
frame_1.GetYaxis().SetTitleOffset(-0.4)
frame_1.GetYaxis().SetTitleSize(0.12)

frame_1.Draw()

graphs_1 = {}
if obs:
    graphs_1['obs'] = ROOT.TGraphAsymmErrors()
graphs_1['exp0'] = ROOT.TGraphAsymmErrors()
h = 0
width = 0.95
for ch in [ 'eTau', 'muTau', 'tauTau', 'cmb' ]:
    with open('{}/limits_{}.json'.format(args.input, ch), 'r') as f:
        res = json.load(f)['1.0']
        exp = res['exp0']
    with open('{}/CMS-HIG-17-002_SM_{}.json'.format(args.input_ref, ch), 'r') as f_1:
        res_1 = json.load(f_1)['1.0']
        exp_1 = res_1['exp0']
        graphs_1['exp0'].SetPoint(h, h+0.5, exp/exp_1)
        graphs_1['exp0'].SetPointEXlow(h, width/2)
        graphs_1['exp0'].SetPointEXhigh(h, width/2)
    
    h += 1

if obs:
    graphs_1['obs'].SetMarkerColor(ROOT.kBlack)
    graphs_1['obs'].SetMarkerStyle(8)
    graphs_1['obs'].SetMarkerSize(0.8)

graphs_1['exp0'].SetLineColor(ROOT.kRed)
graphs_1['exp0'].SetMarkerStyle(8)
graphs_1['exp0'].SetMarkerColor(ROOT.kRed)

graphs_1['exp0'].Draw("p same")
if obs:
    graphs_1['obs'].Draw("p same")


c1.Draw()

c1.Print(output)
c1.Print(output + ']')



