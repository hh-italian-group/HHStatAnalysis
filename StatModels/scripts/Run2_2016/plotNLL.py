import argparse
import ROOT


parser = argparse.ArgumentParser(description='Plot NLL as a function of nuissance parameter.')
parser.add_argument('--diag-fit', required=True, type=str, metavar='PATH', help="diagnostics fit results")
parser.add_argument('--workspace', required=True, type=str, metavar='PATH', help="workspace")
parser.add_argument('--param', required=False, default='', type=str, metavar='NAME', help="nuissance parameter name")
parser.add_argument('--range', required=False, type=str, default='-3:3', metavar='min:max',
                    help="NP range in sigma units")
parser.add_argument('--n-points', required=False, type=int, default=31, metavar='N', help="Number of points")
parser.add_argument('--fit', required=False, type=str, default='fit_s', metavar='fit',
                    help="Fit: bkg only (fit_b) or sig+bkg (fit_s)")
parser.add_argument('--verbose', required=False, type=int, default=0, metavar='verb', help="verbosity level")

args = parser.parse_args()

range_vec = args.range.split(':')
x_min = float(range_vec[0])
x_max = float(range_vec[1])
step = (x_max-x_min) / args.n_points

ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptFit(0)
ROOT.gStyle.SetOptStat(0)
ROOT.gSystem.Load("libHiggsAnalysisCombinedLimit")


def CreateParamsDict(arg_set):
    params = {}

    iter = arg_set.createIterator()
    param = iter.Next()
    while param != None:
        params[param.GetName()] = [ param.getVal(), param.getError() ]
        param = iter.Next()

    return params

def CreateNLLGraph(ws, nuis_name, prefit_err, snapshot_name):
    ws.loadSnapshot(snapshot_name)
    nuis = ws.var(nuis_name)
    if nuis is None:
        raise RuntimeError("Nuissance parameter '{}' not found.".format(nuis_name))
    bf = nuis.getVal()
    nll_0 = nll.getVal()
    if args.verbose > 0:
        print "{}\nbf_x = {} nll_0 = {}".format(nuis_name, bf, nll_0)

    gr = ROOT.TGraph();

    for i in range(0, args.n_points):
    	nuis.setVal(bf + prefit_err * (i * step + x_min))
    	nll_v = nll.getVal();
        if args.verbose > 0:
            print "x = {} nll = {}".format(nuis.getVal(), nll_v)
    	gr.SetPoint(i, nuis.getVal(), nll_v - nll_0)

    gr.SetTitle("")
    gr.GetYaxis().SetTitle("NLL - obs data")
    gr.GetYaxis().SetTitleOffset(1.1)
    gr.GetXaxis().SetTitleSize(0.05)
    gr.GetYaxis().SetTitleSize(0.05)
    gr.GetXaxis().SetTitle(nuis_name)
    gr.GetXaxis().SetNdivisions(505)
    gr.SetLineColor(4)
    gr.SetLineWidth(2)
    gr.SetMarkerStyle(21)
    gr.SetMarkerSize(0.6)

    return gr

if args.verbose == 0:
    ROOT.gErrorIgnoreLevel = ROOT.kWarning
if len(args.param):
    output = '{}_NLL.pdf'.format(args.param)
else:
    output = 'all_NLL.pdf'
c1 = ROOT.TCanvas("", "", 700, 700)
c1.SetLeftMargin(0.15)
c1.SetRightMargin(0.05)
c1.SetTopMargin(0.05)
c1.Draw()
c1.cd()
c1.Print(output + '[')




fit_file = ROOT.TFile(args.diag_fit, 'READ')
bestfit = fit_file.Get(args.fit)
fit_args= bestfit.floatParsFinal()
prefit_args = fit_file.Get("nuisances_prefit")
prefit_params = CreateParamsDict(prefit_args)

if len(args.param) > 0 and args.param not in prefit_params:
    raise RuntimeError("Nuissance parameter '{}' not found.".format(nuis_name))

ws_file = ROOT.TFile(args.workspace, 'READ')
ws   = ws_file.Get("w")
data = ws.data("data_obs")
mc_model = ws.genobj("ModelConfig")

nll_args = ROOT.RooLinkedList()
nll_args.Add(ROOT.RooFit.Constrain(mc_model.GetNuisanceParameters()))
nll_args.Add(ROOT.RooFit.Extended(mc_model.GetPdf().canBeExtended()))
nll = mc_model.GetPdf().createNLL(data, nll_args)

ws.saveSnapshot("bestfitparams", ROOT.RooArgSet(fit_args), True)
for param_name in prefit_params:
    if len(args.param) and param_name != args.param: continue
    c1.Clear()
    gr = CreateNLLGraph(ws, param_name, prefit_params[param_name][1], 'bestfitparams')
    gr.Draw()

    c1.Print(output, 'Title:{}'.format(param_name))

c1.Print(output + ']')
