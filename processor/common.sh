#!/bin/bash -eu

#
# Copyright (c) 2017, Carnegie Mellon University.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
# HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
# WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#


#
# global variables we set/use:
#  $jobdir - per-job shared output directory (abspath)
#  $logfile - log shared by all exp runs (abspath)
#  $all_nodes - list of all nodes (string - sep: comma)
#  $num_all_nodes - number of nodes in all_nodes (int)
#

# environment variables we set/use:
#  $JOBDIRHOME - where to put job dirs (default: $HOME/jobs)
#                example: /lustre/ttscratch1/users/$USER
#  $JOBENV - operating env (one of moab, slurm, openmpi, or mpich)
#            we'll try and figure it out if not provided by user
#  $JOBRUNCMD - aprun/srun/mpirun command to use, should match $JOBENV
#               (only used if JOBENV is set by user)
#  $EXTRA_MPIOPTS - additional options that need to be passed to mpirun
#

#
# environment variables we use as input:
#  $HOME - your home directory
#
#  Cray env running moab:
#    $MOAB_JOBNAME - jobname
#    $PBS_JOBID - job id
#    $PBS_NODEFILE - file with list of all nodes
#
#  Cray env running slurm:
#    $SLURM_JOB_NAME - jobname
#    $SLURM_JOBID - job id
#    $SLURM_JOB_NODELIST - list of all nodes
#       XXX: this node list is in compact form, we need to expand it
#            to get a full list of nodes.  there is a helper perl script
#            called "generate_pbs_nodefile" that will do this for us
#

#
# files we create:
#  $jobdir/hosts.txt - list of hosts
#

# TODO:
# - Convert node lists to ranges on CRAY

### ensure definition of a set of global vars ###

#
# job-wise log - shared among all exp runs
# default: null (XXX: but we override this in get_jobdir)
#
logfile=${logfile-}

#
# message: echo message to stdout, cc it to a default job-wise log file.
# note that if $logfile is empty, tee will just print the message without
# writing to files
# uses: $logfile
#
message () { echo "$@" | tee -a $logfile; }

#
# die: emit a mesage and exit 1
#
die () { message "!!! ERROR !!! $@"; exit 1; }

#
# jobdir  ## Lustre
#
# get_jobdir: setup $jobdir var and makes sure $jobdir is present
# uses: $JOBENV, $MOAB_JOBNAME, $PBS_JOBID,
#       $SLURM_JOB_NAME, $SLURM_JOBID, $JOBDIRHOME
# sets: $jobdir  (XXX: also $logfile)
#
get_jobdir() {
    if [ x${JOBDIRHOME-} != x ]; then
        jobdirhome=${JOBDIRHOME}
    else
        jobdirhome=${HOME}/jobs
    fi

    if [ $JOBENV = moab ]; then
        jobdir=${jobdirhome}/${MOAB_JOBNAME}.${PBS_JOBID}
    elif [ $JOBENV = slurm ]; then
        jobdir=${jobdirhome}/${SLURM_JOB_NAME}.${SLURM_JOBID}
    elif [ x${MPIJOBNAME-} != x ]; then
        jobdir=${jobdirhome}/${MPIJOBNAME}.${MPIJOBID-$$}
    else
        jobdir=${jobdirhome}/`basename $0`.$$  # use top-level script name $0
    fi

    message "-INFO- creating jobdir..."
    mkdir -p ${jobdir} || die "cannot make jobdir ${jobdir}"
    message "-INFO- jobdir = ${jobdir}"

    # XXX: override default and auto set logfile
    logfile=${jobdir}/$(basename $jobdir).log
}

#
# all_nodes
#
# gen_hostfile: generate a list of hosts we have in $jobdir/hosts.txt
# one host per line.
# uses: $JOBENV, $PBS_NODEFILE
#       $SLURMJOB_NODELIST/generate_pbs_nodefile, $jobdir
# sets: $all_nodes, $num_all_nodes
# creates: $jobdir/hosts.txt
#
gen_hostfile() {
    message "-INFO- generating hostfile ${jobdir}/hosts.txt..."

    if [ $JOBENV = moab ]; then

        # Generate hostfile on CRAY and store on disk
        cat $PBS_NODEFILE | uniq | sort > $jobdir/hosts.txt || \
            die "failed to create hosts.txt file"

    elif [ $JOBENV = slurm ]; then

        # Generate hostfile on CRAY and store on disk
        tmp_nodefile=`generate_pbs_nodefile`
        cat $tmp_nodefile | uniq | sort > $jobdir/hosts.txt || \
            die "failed to create hosts.txt file"
        rm -f $tmp_nodefile

    else

        # Generate hostfile on Emulab and store on disk
        exp_hosts="`/share/testbed/bin/emulab-listall | tr ',' '\n'`"

        echo "$exp_hosts" > $jobdir/hosts.txt || \
            die "failed to create hosts.txt file"
    fi

    # Populate a variable with hosts
    all_nodes=$(cat ${jobdir}/hosts.txt)
    num_all_nodes=$(cat ${jobdir}/hosts.txt | tr ',' '\n' | sort | wc -l)
    message "-INFO- num hosts = ${num_all_nodes}"
}

#
# do_mpirun: Run CRAY MPICH, ANL MPICH, or OpenMPI run command
#
# Arguments:
# @1 number of processes
# @2 number of processes per node
# @3 array of env vars: ("name1", "val1", "name2", ... )
# @4 host list (comma-separated)
# @5 executable (and any options that don't fit elsewhere)
# @6 extra_opts: extra options to mpiexec (optional)
# @7 log: primary log file for mpi stdout (optional)
#
do_mpirun() {
    procs=$1
    ppnode=$2
    if [ ! -z "$3" ]; then
        declare -a envs=("${!3}")
    else
        envs=()
    fi
    hosts="$4"
    exe="$5"
    ### extra options to mpiexec ###
    extra_opts=${6-}
    ### log files ###
    log=${7-$logfile}

    envstr=""; npstr=""; hstr=""

    if [ $JOBENV = moab ]; then

        # CRAY running moab....  we need to use aprun
        if [ ${#envs[@]} -gt 0 ]; then
            envstr=`printf -- "-e %s=%s " ${envs[@]}`
        fi

        if [ $ppnode -gt 0 ]; then
            npstr="-N $ppnode"
        fi

        if [ ! -z "$hosts" ]; then
            hstr="-L $hosts"
        fi

        message "[MPIEXEC]" "aprun -n $procs" $npstr $hstr $envstr \
                $extra_opts ${DEFAULT_MPIOPTS-} $exe
        aprun -n $procs $npstr $hstr $envstr $extra_opts \
            ${DEFAULT_MPIOPTS-} $exe 2>&1 | tee -a $log

    elif [ $JOBENV = slurm ]; then

        # CRAY running slurm....  we need to use srun
        if [ ${#envs[@]} -gt 0 ]; then
            envstr=`printf -- "%s=%s," ${envs[@]}`
            # XXX: "ALL" isn't documented in srun(1) man page, but it works.
            # without it, all other env vars are removed (e.g. as described
            # in the sbatch(1) man page ...).
            envstr="--export=${envstr}ALL"
        fi

        if [ $ppnode -gt 0 ]; then
            nnodes=$(( procs / ppnode ))
            npstr="-N $nnodes --ntasks-per-node=$ppnode"
        fi

        if [ ! -z "$hosts" ]; then
            hstr="-w $hosts"
        fi

        message "[MPIEXEC]" "srun -n $procs" $npstr $hstr $envstr \
            $extra_opts ${DEFAULT_MPIOPTS-} $exe
        srun -n $procs $npstr $hstr $envstr $extra_opts \
            ${DEFAULT_MPIOPTS-} $exe 2>&1 | tee -a $log

    elif [ $JOBENV = mpich ]; then

        if [ ${#envs[@]} -gt 0 ]; then
            envstr=`printf -- "-env %s %s " ${envs[@]}`
        fi

        if [ $ppnode -gt 0 ]; then
            npstr="-ppn $ppnode"
        fi

        if [ ! -z "$hosts" ]; then
            hstr="--host $hosts"
        fi

        message "[MPIEXEC]" "mpirun.mpich -np $procs" $npstr $hstr $envstr \
            $extra_opts ${DEFAULT_MPIOPTS-} $exe
        mpirun.mpich -np $procs $npstr $hstr $envstr $extra_opts \
            ${DEFAULT_MPIOPTS-} $exe 2>&1 | tee -a $log

    elif [ $JOBENV = openmpi ]; then

        if [ ${#envs[@]} -gt 0 ]; then
            envstr=`printf -- "-x %s=%s " ${envs[@]}`
        fi

        if [ $ppnode -gt 0 ]; then
            npstr="-npernode $ppnode"
        fi

        if [ ! -z "$hosts" ]; then
            if [ $ppnode -gt 1 ]; then
                hhstr="`printf '&,%.0s' $(seq 1 $(($ppnode-1)))`"
                hhstr="`echo $hosts | sed -e 's/\([^,]*\)/'"$hhstr&"'/g'`"
                hstr="--host $hhstr"
            else
                hstr="--host $hosts"
            fi
        fi

        message "[MPIEXEC]" "mpirun.openmpi -n $procs" $npstr $hstr $envstr \
            $extra_opts ${DEFAULT_MPIOPTS-} $exe
        mpirun.openmpi -n $procs $npstr $hstr $envstr $extra_opts \
            ${DEFAULT_MPIOPTS-} $exe 2>&1 | tee -a $log

    else

        die "could not find a supported mpirun or aprun command"

    fi
}

#
# common_teardown: tear down the common.sh layer
#
common_teardown() {
    # overall time
    timeout=`date`

    message "Script complete!"
    message "start: ${timein}"
    message "  end: ${timeout}"
}

#
# common_init: init the common.sh layer (mainly detecting env)
# uses/sets: $JOBENV, $JOBRUNCMD
#
common_init() {
    message "Script begin..."

    # keep track of start time so we can see how long this takes
    timein=`date`

    if [ x${JOBENV-} = x ]; then
        # try and guess our job environment
        export JOBRUNCMD=""         # only allowed through if JOBENV set by usr
        if [ x${PBS_JOBID-} != x ]; then
            export JOBENV=moab
        elif [ x${SLURM_JOBID-} != x ]; then
            export JOBENV=slurm
        elif [ `which mpirun.mpich` ]; then
            export JOBENV=mpich
        elif [ `which mpirun.openmpi` ]; then
            export JOBENV=openmpi
        else
            die "common.sh UNABLE TO DETERMINE ENV - ABORTING"
        fi
    else
        # verify user selected something we know about
        if [ $JOBENV != moab -a $JOBENV != slurm -a \
             $JOBENV != mpich -a $JOBENV != openmpi ]; then
            die "bad JOBENV ($JOBENV) provided by user - ABORTING"
        fi
    fi

    # set JOBRUNCMD to the default if not provided by user
    if [ x${JOBRUNCMD-} = x ]; then
        if [ $JOBENV = moab ]; then
            export JOBRUNCMD=aprun
        elif [ $JOBENV = slurm ]; then
            export JOBRUNCMD=srun
        elif [ $JOBENV = mpich ]; then
            export JOBRUNCMD=mpirun.mpich
        elif [ $JOBENV = openmpi ]; then
            export JOBRUNCMD=mpirun.openmpi
        fi
    fi

    message "-INFO- common_init: JOBENV=$JOBENV, JOBRUNCMD=$JOBRUNCMD"

    # verify cray env is set as expected
    if [ $JOBENV = moab ]; then
        if [ x${PBS_JOBID-} = x -o x${MOAB_JOBNAME-} = x \
             -o ! -f "$PBS_NODEFILE" ]; then
            die "bad moab setup - check jobname and nodefile"
        fi
    elif [ $JOBENV = slurm ]; then
        if [ x${SLURM_JOBID-} = x -o x${SLURM_JOB_NAME-} = x -o \
             x${SLURM_JOB_NODELIST-} = x ]; then
            die "bad slurm setup - check jobname and nodelist"
        fi
        if [ ! `which generate_pbs_nodefile` ]; then
            die "slurm helper script generate_pbs_nodefile not found in path"
        fi
    fi

    # Set up job dir
    get_jobdir

    # Get machine list
    gen_hosts
}
