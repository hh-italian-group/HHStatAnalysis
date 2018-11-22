#!/usr/bin/env python

import os
import shutil
import subprocess

s0_points = {
    'lm': [ 250, 260, 270, 280, 300, 320 ],
    'mm': [ 340, 350, 400 ],
    'hm': [ 450, 500, 550, 600, 650, 750, 800, 900 ]
}

s2_points = {
    'lm': [ 250, 260, 270, 280, 300, 320 ],
    'mm': [ 340, 350, 400 ],
    'hm': [ 450, 500, 550, 600, 650, 750, 800 ]
}


mass_points = {
    0 : s0_points,
    2 : s2_points
}

spin = 2

s_points = mass_points[spin]
for region, masses in s_points.iteritems():
    for mass in masses:
        output = 'output/2018-01-29/Res_newMW/Res_s{}_MVA/{}_{}'.format(spin, region, mass)
        if os.path.exists(output):
            shutil.rmtree(output)
        os.makedirs(output)
        cmd = './HHStatAnalysis/StatModels/scripts/run_hh_limits.py' \
              ' --cfg HHStatAnalysis/Resources/LimitSetups/Run2_2016/ttbb.cfg --model-desc ttbb_res --output {}' \
              ' --signal-point {} /gpfs/ddn/cms/user/androsov/store/2018-01-31-HTT_meeting/Res_newMW/shapes/all_mva_score_mhVis_mva_{}newkinS{}M{}C-1.root' \
              .format(output, mass, region, spin, mass)
        print('% {}'.format(cmd))
        subprocess.call([cmd], shell=True)
