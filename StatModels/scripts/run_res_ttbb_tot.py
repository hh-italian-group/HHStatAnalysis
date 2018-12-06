#!/usr/bin/env python

import os
import shutil
import subprocess

s0_points = {
    'HM': [ 250, 260, 270, 280, 300, 320, 350, 400, 450, 500, 600, 650, 700, 750, 800, 850, 900, 1000, 1250, 1500. 1750, 2000, 2500, 3000  ]
}

s2_points = {
    'HM': [ 250, 260, 270, 280, 350, 400, 450, 550, 600, 650, 750, 800 ]
}


mass_points = {
    0 : s0_points,
    2 : s2_points
}

spin = 0

s_points = mass_points[spin]
for region, masses in s_points.iteritems():
    for mass in masses:
        output = 'output/2018-01-29/Res_tot_newMW/Res_s{}_MVA/{}_{}'.format(spin, region, mass)
        if os.path.exists(output):
            shutil.rmtree(output)
        os.makedirs(output)
        cmd = './HHStatAnalysis/StatModels/scripts/Run2_2017/run_hh_limits.py' \
              ' --cfg HHStatAnalysis/Resources/LimitSetups/Run2_2017/ttbb.cfg --model-desc ttbb_res --output {}' \
              ' --signal-point {} /gpfs/ddn/cms/user/androsov/store/2018-01-31-HTT_meeting/Res_tot_newMW/shapes/all_mva_score_mhVis_{}S{}M{}C-1.root' \
              .format(output, mass, region, spin, mass)
        print('% {}'.format(cmd))
        subprocess.call([cmd], shell=True)
