import os
import argparse
import json

parser = argparse.ArgumentParser(description='Combine signal point limits.',
                  formatter_class = lambda prog: argparse.HelpFormatter(prog, max_help_position=30, width=90))
parser.add_argument('--input', required=True, dest='input', type=str, metavar='PATH', help="input path")
args = parser.parse_args()

points = list(filter(lambda f: os.path.isdir('{}/{}'.format(args.input, f)), os.listdir(args.input)))

#print(os.listdir(args.input))
#print(points)
cmb_results = {}

for point in points:
    print(point)
    point_file_name = '{}/{}/limits_cmb.json'.format(args.input, point)
    with open(point_file_name, 'r')  as point_file:
        point_results = json.load(point_file)
        cmb_results.update(point_results)

output_name = '{}/limits_cmb.json'.format(args.input)
with open(output_name, 'w') as out_file:
    out_file.write(json.dumps(cmb_results, sort_keys=True, indent=4, separators=(',', ': ')))
