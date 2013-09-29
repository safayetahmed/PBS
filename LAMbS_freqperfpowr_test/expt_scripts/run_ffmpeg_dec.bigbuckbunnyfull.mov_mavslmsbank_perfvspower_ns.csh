#! /bin/csh -x
    
    #Set default values for optional input arguments
    set repetitions="1"
    set LOGDIR="data/"
    set PeSoRTADIR="/media/Data/Research/expt_February2013/PeSoRTA"
    set BINDIR="/home/gtg778w/Desktop/bin/"
    
    #Process input arguments for repetitions BINDIR LOGDIR and PeSoRTADIR
    if ( $#argv == 1 ) then
        set repetitions=$argv[1]
    else if ( $#argv == 2 ) then
        set repetitions=$argv[1]
        set LOGDIR=$argv[2]
    else if ( $#argv == 3 ) then
        set repetitions=$argv[1]
        set LOGDIR=$argv[2]
        set PeSoRTADIR=$argv[3]
    else if ( $#argv == 4 ) then
        set repetitions=$argv[1]
        set LOGDIR=$argv[2]
        set PeSoRTADIR=$argv[3]
        set BINDIR=$argv[4]
    else if ( $#argv > 4 ) then
        echo "Usage:"$argv[0]" [repetitions] [log directory] [PeSoRTA directory] [bin dir]"
        exit 1
    endif
    
    #The relevant folder for CPU speed settings
    set freqdir="/sys/devices/system/cpu/cpu0/cpufreq/"
    #The list of available frequencies
    set availablefreqs=`cat ${freqdir}"scaling_available_frequencies"`
    
    #The period of the allocator task
    set Ta="10416667"
    #The budget assigned to the allocator task over a reservation period
    set Qa="1000000"
    set sa="28800"
    #The budget type should be "" for ns and "-VIC" for VIC.
    set BUDGET_TYPE=""
    
    #The name of the configuration
    set APPNAME="ffmpeg"
    set CONFIGNAME="dec.bigbuckbunnyfull.480p.mov"
    
    #The name of the configuration file for the PeSoRTA workload
    set W1="config/"${CONFIGNAME}".config"
    #The root directory for the PeSoRTA workload
    set D1=${PeSoRTADIR}"/"${APPNAME}
    #The maximum number of jobs to run from the PeSoRTA workload
    set J1="6480"
    #The predictor to be used for budget allocation by the SRT application
    set A1="mavslmsbank"
    #The task period (in ns) of the SRT application
    set p1="41666668"
    #The estimated mean execution time of the SRT application
    set c1="11000000"
    #The alpha parameter
    set alpha="2.3"
    
    @ duration_secs = ((((${Ta} / 1000) * ${sa}) / 1000) * ${repetitions}) / 1000
    
    #Loop over the values of alpha
    foreach freq ( $availablefreqs )
        
        echo "Setting CPU frequency to $freq"        
        echo "userspace" > ${freqdir}"scaling_governor"
        echo ${freq} > ${freqdir}"scaling_setspeed"

        set LOCALLOGDIR=${LOGDIR}"/"${APPNAME}"/"${CONFIGNAME}"/freqconstcycle/ns/${freq}"
        mkdir -p ${LOCALLOGDIR}
        
        #The suffix of the log file to store the data collected by the SRT application
        set SRT_logfilesuffix="_allocator_SRT_PeSoRTA_"${APPNAME}"_"${CONFIGNAME}"_"${J1}"_"${A1}"_"${p1}"_"${c1}"_"${alpha}".log"        
        
        #Loop over the repetitions
        foreach rep (`seq 1 1 ${repetitions}`)
            
            #Display progress and estimated duration
            echo "Experiment: ${LOCALLOGDIR}"
            echo "Approximate total duration of experiment: ${duration_secs}"
            echo "CPU frequency: ${freq}"
            echo "Alpha value: "${alpha}
            echo "Repetition "${rep}" of ${repetitions}"
            
            #Names of LOG files
            set allocator_logfile=${LOCALLOGDIR}"/"${rep}${SRT_logfilesuffix}
            
            #Start the allocator            
            ${BINDIR}/pbsAllocator -f -s ${sa} -P ${Ta} -B ${Qa} ${BUDGET_TYPE} -L 1 > ${allocator_logfile} &
            #Wait for half a second to let the allocator run a couple of scheduling periods
            sleep 0.5
            #Run the SRT task
            ${BINDIR}/${APPNAME}_pbsSRT -f -W ${W1} -D ${D1} -J ${J1} -A ${A1} -p ${p1} -c ${c1} -a ${alpha} -L 0 &
            #Wait for the allocator task to finish
            ${BINDIR}/poll_pbs_actv -I
            
        end
    end

