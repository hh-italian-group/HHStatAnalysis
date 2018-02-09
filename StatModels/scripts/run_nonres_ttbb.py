#!/usr/bin/env python

import os
import shutil
import subprocess

kl_points = [ -20, -14, -10, -8, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 16, 20, 24, 30 ]

for kl in kl_points:
    output = 'output/2018-02-14-HTT_meeting/kl_scan/{}'.format(kl)
    if os.path.exists(output):
        shutil.rmtree(output)
    os.makedirs(output)
    cmd = './HHStatAnalysis/StatModels/scripts/run_hh_limits.py' \
          ' --cfg HHStatAnalysis/Resources/LimitSetups/Run2_2016/ttbb.cfg --model-desc ttbb_nonres --output {0}' \
          ' --signal-point {1} ttbb-it/2018-02-14-HTT_meeting/kl_scan/shapes/all_mva_score_mh_SMS{1}M125C-1.root' \
          .format(output, kl)
    print('% {}'.format(cmd))
    subprocess.call([cmd], shell=True)
