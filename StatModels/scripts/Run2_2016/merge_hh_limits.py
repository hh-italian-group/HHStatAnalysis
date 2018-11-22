#!/usr/bin/env python
# Merge hh limits.
# This file is part of https://github.com/cms-hh/HHStatAnalysis.

import sys
import os
import argparse
import glob
import json
import fnmatch
import shutil
from sets import Set
import ROOT

parser = argparse.ArgumentParser(description='Merge hh limits.',
                  formatter_class = lambda prog: argparse.HelpFormatter(prog, max_help_position=30, width=90))
parser.add_argument('--merge-output', required=True, dest='merge_output', type=str, metavar='PATH',
                    help="path were to store merged limits")
parser.add_argument('output', type=str, nargs='+', help="path were limits are stored")

args = parser.parse_args()

def FindQuantile(quantile_expected):
    for quantile_name,quantile_value in FindQuantile.all_quantiles.iteritems():
        if abs(quantile_value - quantile_expected) < 0.01:
            return quantile_name
    raise RuntimeError('Unknown expected quantile value = {}'.format(quantile_expected))
FindQuantile.all_quantiles = {
    'obs': -1., 'exp-2': 0.025, 'exp-1': 0.16, 'exp0': 0.5, 'exp+1': 0.84, 'exp+2': 0.975
}

def ReadLimits(root_file_name):
    file = ROOT.TFile(root_file_name, 'OPEN')
    tree = file.Get('limit')
    limits = {}
    for limit_entry in tree:
        quantile_name = FindQuantile(limit_entry.quantileExpected)
        limits[quantile_name] = limit_entry.limit
    return limits


work_dir = os.getcwd()

limit_pattern = 'higgsCombine.*.root'
limit_file_set = Set()
for output in args.output:
    os.chdir(output)
    for root, dir_names, file_names in os.walk('.'):
        for limit_file in fnmatch.filter(file_names, limit_pattern):
            full_name = os.path.join(root, limit_file)
            if full_name not in limit_file_set:
                limit_file_set.add(full_name)
    os.chdir(work_dir)
limit_files = sorted(limit_file_set)


max_file_name_length = max([ len(f[2:]) for f in limit_files ])

line_format = '{{0: <{}}}{{1: <20}}{{2}}'.format(max_file_name_length + 4)

print max_file_name_length
print line_format.format('File', 'Exp limit', 'Source')

for limit_file in limit_files:
    expected_limits = []
    for output in args.output:
        full_name = os.path.join(output, limit_file)
        if not os.path.isfile(full_name):
            raise RuntimeError('File not found {}'.format(full_name))
        exp = ReadLimits(full_name)['exp0']
        expected_limits.append([ output, exp ])
    expected_limits = sorted(expected_limits, key=lambda entry: entry[1])
    best_limits_file = os.path.join(expected_limits[0][0], limit_file)
    print line_format.format(limit_file[2:], expected_limits[0][1], expected_limits[0][0])
    limit_file_dir = os.path.dirname(limit_file)
    output_dir = os.path.join(args.merge_output, limit_file_dir)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    shutil.copy(best_limits_file, output_dir)
