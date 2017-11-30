#!/usr/bin/python
import argparse     # For CL argument parsing
import sys          # For sys.exit
import os           # For os.path.*, os.walk
import re           # For re.match

# Process one file
def process_file(fp, ind, outd, dr):
    src = ind + fp

    res = re.match("/(\d*).(\d*)/part-(\d*)$", fp)
    if (res):
        dstd = outd + "/" + res.group(1)
        dst = outd + "/" + res.group(1) + "/part-" + \
              res.group(2) + "-" + res.group(3)
        if (dr):
            print "[DATA] " + src + " ~> " + dst
        else:
            if (os.path.exists(dstd) == False):
                os.makedirs(dstd)
            os.rename(src, dst)
        return 0

    res = re.match("/(\d*).(\d*).dict/part-(\d*)$", fp)
    if (res):
        dstd = outd + "/" + res.group(1) + ".dict"
        dst = outd + "/" + res.group(1) + ".dict/part-" + \
              res.group(2) + "-" + res.group(3)
        if (dr):
            print "[DICT] " + src + " ~> " + dst
        else:
            if (os.path.exists(dstd) == False):
                os.makedirs(dstd)
            os.rename(src, dst)
        return 0

    res = re.match("/(\d*).(\d*).statistics$", fp)
    if (res):
        dstd = outd + "/" + res.group(1) + ".stats"
        dst = outd + "/" + res.group(1) + ".stats/" + \
              res.group(1) + "." + res.group(2) + ".statistics"
        if (dr):
            print "[STAT] " + src + " ~> " + dst
        else:
            if (os.path.exists(dstd) == False):
                os.makedirs(dstd)
            os.rename(src, dst)
        return 0

    print "Unknown file type: ", fp
    return 1

# Configure command line option parser
parser = argparse.ArgumentParser(description = "Move CommonCrawl files around.")
parser.add_argument('-i', '--indir', dest = 'indir', action = 'store',
                    nargs = 1, required = True, help = 'input directory')
parser.add_argument('-o', '--outdir', dest = 'outdir', action = 'store',
                    nargs = 1, required = True, help = 'output directory')
parser.add_argument('-d', '--dry-run', dest = 'dryrun', action = 'store_true',
                    required = False, help = 'only print actions')

# Load arguments
args = parser.parse_args()
indir = args.indir[0]
outdir = args.outdir[0]
dryrun = args.dryrun

# Sanity-check arguments
if (indir == '') or (outdir == ''):
    sys.exit('Error: Input and output directories must be specified\n')

if (os.path.isdir(indir) == False) or (os.path.isdir(outdir) == False):
    sys.exit('Error: Input and output paths must point to directories')

print "indir: " + indir
print "outdir: " + outdir
print "dryrun: " + str(dryrun)

for dp, dn, fn in os.walk(indir):
    for f in fn:
        fp = str(dp) + "/" + str(f)
        fp = fp[len(indir):]
        process_file(fp, indir, outdir, dryrun)
