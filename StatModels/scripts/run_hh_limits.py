#!/usr/bin/env python
# Run hh limits.
# This file is part of https://github.com/cms-hh/HHStatAnalysis.

import os
import argparse
import glob
import json
from sets import Set
from HHStatAnalysis.StatModels.run_hh_limits_helpers import *
import libHHStatAnalysisStatModels as HH

parser = argparse.ArgumentParser(description='Run hh limits.',
                  formatter_class = lambda prog: argparse.HelpFormatter(prog, max_help_position=30, width=90))
parser.add_argument('--cfg', required=True, dest='cfg', type=str, metavar='FILE', help="configuration file")
parser.add_argument('--model-desc', required=True, dest='model_desc', type=str, metavar='DESC',
                    help="name of the stat model descriptor in the config")
parser.add_argument('--output', required=True, dest='output_path', type=str, metavar='PATH',
                    help="path were to store limits")
parser.add_argument('--parallel', required=False, dest='n_parallel', type=int, default=8, metavar='N',
                    help="number of parallel jobs")
parser.add_argument('--plotOnly', action="store_true", help="Run only plots.")
parser.add_argument('--collectAndPlot', action="store_true", help="Run only plots.")
parser.add_argument('--impacts', action="store_true",
                    help="Compute impact of each nuissance parameter to the final result.")
parser.add_argument('--pulls', action="store_true", help="Compute pulls.")
parser.add_argument('--GoF', action="store_true", help="Evaluate goodness of fit.")
parser.add_argument('--signal-point', required=False, dest='signal_point', type=str, default="", metavar='M',
                    help="run on a single signal point")
parser.add_argument('shapes_file', type=str, nargs='+', help="file with input shapes")

args = parser.parse_args()

limit_json_name = "limits"
limit_json_file = limit_json_name + ".json"
limit_json_pattern = limit_json_name + "_*.json"

if not os.path.exists(args.output_path):
    os.makedirs(args.output_path)

model_desc = HH.LoadDescriptor(args.cfg, args.model_desc)

run_limits = not args.plotOnly and not args.collectAndPlot
collect_limits = run_limits or args.collectAndPlot

if run_limits:
    shapes_file = args.shapes_file[0]
    if len(args.shapes_file) > 1:
        shapes_file = '{}/shapes.root'.format(args.output_path)
        hadd_command = 'hadd -f9 {}'.format(shapes_file)
        for file in args.shapes_file:
            hadd_command += ' "{}"'.format(file)
        sh_call(hadd_command, "error while executing hadd for input files")
    create_cmd = 'create_hh_datacards --cfg {} --model-desc {} --shapes {} --output {}' \
                 .format(args.cfg, args.model_desc, shapes_file, args.output_path)
    if len(args.signal_point) > 0:
        create_cmd += ' --signal-point {}'.format(args.signal_point)
    sh_call(create_cmd, "error while executing create_hh_datacards")

limit_type = str(model_desc.limit_type)
if limit_type in Set(['model_independent', 'SM', 'NonResonant_BSM']):
    ch_dir(args.output_path)
    if run_limits:
        sh_call('combineTool.py -M T2W -i */* -o workspace.root --parallel {}'.format(args.n_parallel),
                "error while executing text to workspace")
        combine_cmd = 'combineTool.py -M Asymptotic -d */*/workspace.root --there -n .limit --parallel {}' \
                      ' --minimizerStrategy=1'.format(args.n_parallel)
        if model_desc.blind:
            combine_cmd += ' --run blind'
        if limit_type == 'SM':
            combine_cmd += ' --rMax 100'
        sh_call(combine_cmd, "error while executing combine")

    if collect_limits:
        sh_call('combineTool.py -M CollectLimits */*/*.limit.* --use-dirs -o {}'.format(limit_json_file),
                "error while collecting limits")

    if args.pulls:
        channels = filter(lambda f: os.path.isdir(f), os.listdir('.'))
        for channel in channels:
            points = filter(lambda f: os.path.isdir('{}/{}'.format(channel, f)), os.listdir(channel))
            for point in points:
                ch_dir('{}/{}'.format(channel, point))
                pulls_work_path = 'pulls'
                if not os.path.exists(pulls_work_path):
                    os.makedirs(pulls_work_path)
                ch_dir(pulls_work_path)

                max_likelihood_cmd = 'combine -M MaxLikelihoodFit ../workspace.root --robustFit=1 --expectSignal=1'
                if model_desc.blind:
                    max_likelihood_cmd += ' -t -1'

                sh_call(max_likelihood_cmd, "error while running MaxLikelihoodFit")

                diffNuisances = os.environ['CMSSW_BASE'] + '/src/HiggsAnalysis/CombinedLimit/test/diffNuisances.py'
                pulls_out = '../../../pulls_{}_{}'.format(channel, point)
                pulls_txt_out = '{}.txt'.format(pulls_out)
                pulls_root_out = '{}.root'.format(pulls_out)

                pulls_cmd = 'python {} mlfit.root -A -a -f text -g {} > {}'.format(diffNuisances,
                            pulls_root_out, pulls_txt_out)

                sh_call(pulls_cmd, "error while creating pulls")
                ch_dir('../../..')

    if args.GoF:
        channels = filter(lambda f: os.path.isdir(f), os.listdir('.'))
        for channel in channels:
            points = filter(lambda f: os.path.isdir('{}/{}'.format(channel, f)), os.listdir(channel))
            point = points[0]
            ch_dir('{}/{}'.format(channel, point))
            gof_work_path = 'GoF'
            if not os.path.exists(gof_work_path):
                os.makedirs(gof_work_path)
            ch_dir(gof_work_path)

            gof_algo = 'saturated'
            n_toys = 1000
            n_toys_per_job = n_toys / args.n_parallel

            gof_out = '../../../GoF_{}_{}'.format(channel, gof_algo)
            gof_json_out = '{}.json'.format(gof_out)

            gof_cmd = 'combineTool.py -M GoodnessOfFit --algorithm {} -d ../workspace.root --fixedSignalStrength=0' \
                      .format(gof_algo)
            if limit_type == 'SM':
                combine_cmd += ' --rMax 100'

            sh_call('{} -n .{}'.format(gof_cmd, gof_algo), "error while evaluating goodness of fit for data")
            sh_call('{} -n .{}.toys -t {} -s 0:{}:1 --parallel {}' \
                    .format(gof_cmd, gof_algo, n_toys_per_job, args.n_parallel - 1, args.n_parallel),
                    "error while generating toys for goodness of fit")

            sh_call('combineTool.py -M CollectGoodnessOfFit --input higgsCombine.{0}.GoodnessOfFit.mH120.root' \
                    ' higgsCombine.{0}.toys.GoodnessOfFit.mH120.*.root -o {1}' \
                    .format(gof_algo, gof_json_out),
                    "error while collecting toys for goodness of fit")

            plotGoF = os.environ['CMSSW_BASE'] + '/src/CombineHarvester/CombineTools/scripts/plotGof.py'
            sh_call('{} --statistic {} --mass 120.0 {} -o {}'.format(plotGoF, gof_algo, gof_json_out, gof_out),
                    "error while plotting goodness of fit")

            ch_dir('../../..')

    if args.impacts:
        channels = filter(lambda f: os.path.isdir(f), os.listdir('.'))
        for channel in channels:
            points = filter(lambda f: os.path.isdir('{}/{}'.format(channel, f)), os.listdir(channel))
            for point in points:
                ch_dir('{}/{}'.format(channel, point))
                impacts_work_path = 'impacts'
                if not os.path.exists(impacts_work_path):
                    os.makedirs(impacts_work_path)
                ch_dir(impacts_work_path)
                impact_cmd = 'combineTool.py -M Impacts -m {} -d ../workspace.root --expectSignal 10' \
                             ' --allPars --parallel {}'.format(point, args.n_parallel)
                if model_desc.blind:
                    impact_cmd += ' -t -1 --rMax 100'
                else:
                    impact_cmd += '--rMax 100'

                sh_call(impact_cmd + ' --doInitialFit', "error while doing initial fit for impacts")
                sh_call(impact_cmd + ' --robustFit 1 --doFits', "error while doing robust fit for impacts")
                sh_call('combineTool.py -M Impacts -m {} -d ../workspace.root -o impacts.json'.format(point),
                        "error while collecting impacts")
                impacts_out = '../../../impacts_{}_{}'.format(channel, point)
                sh_call('plotImpacts.py -i impacts.json -o {}'.format(impacts_out),
                        "error while plotting impacts")
                ch_dir('../../..')

    limits_to_show = "exp"
    if not model_desc.blind:
        limits_to_show += ",obs"

    for input_file in glob.glob(limit_json_pattern):
        output_name = os.path.splitext(input_file)[0]
        y_title = None
        if limit_type == 'SM':
            y_title = "95% CL limit on #sigma / #sigma(SM)"
        else:
            y_title = "95% CL limit on #sigma x BR (pb)"
        sh_call('plotLimits.py {} --output {} --auto-style --y-title \'{}\' --logy --show {}'.format(input_file,
                output_name, y_title, limits_to_show), "error while plotting limits")

elif limit_type == 'MSSM':
    th_model_file_full_path = os.path.abspath(model_desc.th_model_file)
    th_models_path, th_model_file = os.path.split(th_model_file_full_path)
    ch_dir(args.output_path)

    if run_limits or collect_limits:
        grid_dict = { 'opts'  : '--singlePoint 1.0', 'POIs'  : [ 'mA', 'tanb' ], 'grids' : [] }
        grid_dict['grids'].append([
            '{}:{}|{}'.format(model_desc.grid_x.min(), model_desc.grid_x.max(), model_desc.grid_x.step()),
            '{}:{}|{}'.format(model_desc.grid_y.min(), model_desc.grid_y.max(), model_desc.grid_y.step()),
            ''
        ])
        grid_file_name = 'asymptotic_grid.json'
        grid_file = open(grid_file_name, 'w')
        grid_file.write(json.dumps(grid_dict))
        grid_file.close()

    plotLimitGrid = os.environ['CMSSW_BASE'] + '/src/CombineHarvester/CombineTools/scripts/plotLimitGrid.py'
    contours = "exp-2,exp-1,exp0,exp+1,exp+2"
    if not model_desc.blind:
        contours += ",obs"
    workspace_file = "MSSM.root"

    channels = filter(lambda f: os.path.isdir(f), os.listdir('.'))
    for channel in channels:

        if run_limits:
            sh_call('combineTool.py -M T2W -o {} -P {} --PO filePrefix={}/ --PO modelFiles=13TeV,{},1 -i {}'.format(
                    workspace_file, 'CombineHarvester.CombinePdfs.MSSM:MSSM', th_models_path, th_model_file, channel),
                    "error while executing text to workspace")

        work_path = channel + '/work'
        if not os.path.exists(work_path):
            os.makedirs(work_path)
        ch_dir(work_path)

        if collect_limits:
            asymptoticGrid_cmd = 'combineTool.py -M AsymptoticGrid ../../{} -d ../{} --parallel {} --minimizerStrategy=1'.format(
                                 grid_file_name, workspace_file, args.n_parallel)
            if model_desc.blind:
                asymptoticGrid_cmd += ' -t -1'

            if run_limits:
                sh_call(asymptoticGrid_cmd, "error while executing combine")
            sh_call(asymptoticGrid_cmd, "error while collecting jobs")

        output_name = '../../limits_{}_{}'.format(model_desc.limit_type, channel)
        output_root = '{}.root'.format(output_name)
        draw_range_x = '{},{}'.format(model_desc.draw_range_x.min(), model_desc.draw_range_x.max())
        draw_range_y = '{},{}'.format(model_desc.draw_range_y.min(), model_desc.draw_range_y.max())
        draw_cmd = '{} {} --output {} --debug-output {} --contours {} --cms-sub "{}" --scenario-label "{}"' \
                   ' --title-right "{}" --x-range {} --y-range {} --x-title "{}" --y-title "{}"'.format(
                   plotLimitGrid, 'asymptotic_grid.root', output_name, output_root, contours, model_desc.label_status,
                   model_desc.label_scenario, model_desc.label_lumi, draw_range_x, draw_range_y, model_desc.title_x,
                   model_desc.title_y)
        if model_desc.draw_mh_exclusion or model_desc.draw_mH_isolines:
            draw_cmd += ' --model_file "{}"'.format(th_model_file_full_path)
            if model_desc.draw_mh_exclusion:
                draw_cmd += ' --draw_mh_exclusion'
            if model_desc.draw_mH_isolines:
                signal_point_list = ','.join(model_desc.signal_points)
                draw_cmd += ' --draw_mH_isolines --mH_values {} --iso_label_draw_margin {}' \
                            .format(signal_point_list, model_desc.iso_label_draw_margin)
        sh_call(draw_cmd, "error while plotting limits")

        ch_dir('../..')

else:
    run_failed("limit type '{}' not supported".format(model_desc.limit_type))

run_succeed()
