#! /bin/csh -x

    #Process input arguments for LOGDIR and PeSoRTADIR
    if ( $#argv == 0 ) then
        set LOGDIR="log"
        set PeSoRTADIR="/media/Data/Research/expt_February2013/PeSoRTA"
    else if ( $#argv == 1 ) then
        set LOGDIR=$argv[1]
        set PeSoRTADIR="/media/Data/Research/expt_February2013/PeSoRTA"
    else if ( $#argv == 2 ) then
        set LOGDIR=$argv[1]
        set PeSoRTADIR=$argv[2]
    else
        echo "Usage:"$argv[0]" [log directory] [PeSoRTA directory]"; exit 1
    endif

    #The relevant folder for CPU speed settings
    set freqdir="/sys/devices/system/cpu/cpu0/cpufreq/"
    #The list of available frequencies
    set availablefreqs=`cat ${freqdir}"scaling_available_frequencies"`
    @ freq_i = $#availablefreqs
    #Get the maximum frequency
    set freq=$availablefreqs[${freq_i}]
    echo "Setting CPU frequency to $freq"
    
    #Set the CPU to the maximum speed
    echo "userspace" > ${freqdir}"scaling_governor"
    echo ${freq} > ${freqdir}"scaling_setspeed"
    
    #The period of the allocator task
    set Ta="10416667"
    #The budget assigned to the allocator task over a reservation period
    set Qa="1000000"
    set sa="87000"
    
    #The name of the configuration
    set APPNAME="ffmpeg"
    set CONFIGNAME="dec.bigbuckbunnyfull.mov"
    set LOCALLOGDIR=${LOGDIR}"/"${APPNAME}"/"${CONFIGNAME}"/freqconstlow"
    mkdir -p ${LOCALLOGDIR}

    #The name of the configuration file for the PeSoRTA workload
    set W1="config/"${CONFIGNAME}".config"
    #The root directory for the PeSoRTA workload
    set D1=${PeSoRTADIR}"/"${APPNAME}
    #The maximum number of jobs to run from the PeSoRTA workload
    set J1="21313"
    #The predictor to be used for budget allocation by the SRT application
    set A1="mavslmsbank"
    #The task period (in ns) of the SRT application
    set p1="41666668"
    #The estimated mean execution time of the SRT application
    set c1="11000000"
    #Alpha values of the workload
    set alpha_array=("1.39533")
    
    set repetitions="1"
    
    #Initialize the experiment ID
    #Each repetition for each value of alpha should have a unique ID
    @ expt_id = 1
    #Loop over the values of alpha
    foreach alpha ($alpha_array)
    
        #The suffix of the log file to store the data collected by the SRT application
        set SRT_logfilesuffix="_SRT_PeSoRTA_"${APPNAME}"_"${CONFIGNAME}"_"${J1}"_"${A1}"_"${p1}"_"${c1}"_"${alpha}".log"        
        set ALC_logfilesuffix="_allocator_PeSoRTA_"${APPNAME}"_"${Ta}"_"${Qa}"_"${sa}".log"
        
        #Loop over the repetitions
        foreach rep (`seq 1 1 ${repetitions}`)
        
            #Display progress and estimated duration
            echo "Experiment "${expt_id}" of 30"
            echo "Approximate total duration of experiment: 14:48"
            
            #Names of LOG files
            set SRT_logfile=${LOCALLOGDIR}"/"${expt_id}"_freqconstlow"${SRT_logfilesuffix}
            set ALC_logfile=${LOCALLOGDIR}"/"${expt_id}${ALC_logfilesuffix}
            
            #Start the allocator            
            bin/pbsAllocator -f -S -s ${sa} -P ${Ta} -B ${Qa} &
            #Wait for half a second to let the allocator run a couple of scheduling periods
            sleep 0.5
            #Run the SRT task
            bin/${APPNAME}_pbsSRT -f -W ${W1} -D ${D1} -J ${J1} -A ${A1} -p ${p1} -c ${c1} -a ${alpha} -L 2 -R ${SRT_logfile} &
            #Wait for the allocator task to finish
            bin/poll_pbs_actv -I

            #Increment the experiment ID
            @ expt_id += 1
        end
    end
