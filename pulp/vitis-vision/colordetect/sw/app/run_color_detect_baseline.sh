#!/bin/bash

# ========= #
# FUNCTIONS #
# ========= #

# ===================================================================== #

# Error message 

error_exit()
{
  echo -e "\n$1\n" 1>&2
  exit 1
}

# Open a session using tmux

open_session()
{
  session_name=$1

  # Check if the session exists, discarding output
  # We can check $? for the exit status (zero for success, non-zero for failure)
  tmux has-session -t $session_name 2>/dev/null
  if [ $? != 0 ]; then
    echo "No session has been found"
  else 
    echo "Session <$session_name> has been found"
    echo "Do you want to close it? [ans=1,2,3]"

    select yn in "yes" "no"; do
      case $yn in
        yes ) echo -e "Closing session <$session_name>"; tmux kill-ses -t $session_name; break;;
        no ) 	error_exit "Aborting...";;
      esac
    done

  fi

  tmux new-session -d -s $session_name
  echo -e "Opening session <$session_name>"
}

# ===================================================================== #

# Setup experiment

setup_experiment()
{
  session_name=$1
  experiment_id=$2

  # Send commands
  tmux send-keys -t $session_name:$experiment_id "env_esim" C-m
  tmux send-keys -t $session_name:$experiment_id "hero" C-m
  tmux send-keys -t $session_name:$experiment_id "make sdk-arov-cfg" C-m
}

# ===================================================================== #

# Generate hardware

generate_hw()
{
  session_name=$1
  experiment_id=$2

  # genov
  genov_repo=$3
  target_ov=$4

  # Send commands
  tmux send-keys -t $session_name:$experiment_id "cd $genov_repo" C-m
  tmux send-keys -t $session_name:$experiment_id "deactivate" C-m
  tmux send-keys -t $session_name:$experiment_id "source $genov_repo/local_py_env/bin/activate" C-m
  tmux send-keys -t $session_name:$experiment_id "make clean all TARGET_OV=$target_ov" C-m
  tmux send-keys -t $session_name:$experiment_id "cp -rf $genov_repo/output/$target_ov $genov_repo/../ov_cfg; \
                                                    tmux wait -S end_generate_hw" C-m
  tmux send-keys -t $session_name:$experiment_id "deactivate" C-m

  # Wait for the end of hardware generation 
  tmux wait end_generate_hw 
}

# ===================================================================== #

# Compile experiment

compile_experiment()
{
  session_name=$1
  experiment_id=$2

  # experiment
  experiment_repo=$3
  experiment_name=$4
  executable_path=$5

  # Send commands
  tmux send-keys -t $session_name:$experiment_id "cd $experiment_repo" C-m
  tmux send-keys -t $session_name:$experiment_id "make clean build \
                                SRC_TARGET=$experiment_name \
                                EXE_PATH=$executable_path; \
                                                  tmux wait -S end_build_sw" C-m

  # Wait for the end of software compilation to prevent other tmux  
  # sessions (starting concurrently and in different windows) to clean 
  # the compilation files used by the current session   
  tmux wait end_build_sw 
}

# ===================================================================== #

# Run experiment

run_experiment()
{
  session_name=$1
  experiment_id=$2

  # experiment
  experiment_repo=$3
  experiment_name=$4
  executable_path=$5
  target_ov=$6

  # Send commands
  tmux send-keys -t $session_name:$experiment_id "arov" C-m
  tmux send-keys -t $session_name:$experiment_id "make vsim_clean vsim_hw \
                                TARGET_OV=$target_ov \
                                VSIM_SW_PATH=$experiment_repo/src/$executable_path \
                                VSIM_PRJ_NAME=$session_name\_$experiment_id\_$experiment_name \
                                VSIM_GUI=0; \
                                                  tmux wait -S end_build_hw" C-m

  # Wait for the end of hardware compilation (QuestaSim flow) to prevent other 
  # tmux sessions  (starting concurrently and in different windows) to clean 
  # the compilation files used by the current session                         
  tmux wait end_build_hw

  tmux send-keys -t $session_name:$experiment_id "make vsim_sim \
                                TARGET_OV=$target_ov \
                                VSIM_SW_PATH=$experiment_repo/src/$executable_path \
                                VSIM_PRJ_NAME=$session_name\_$experiment_id\_$experiment_name \
                                VSIM_GUI=0" C-m
}

# ===================================================================== #

# Close the tmux session

close_session()
{
  session_name=$1
  tmux kill-ses -t $session_name
  echo -e "Closing session <$session_name>"
}

# ===================================================================== #

          # Parameters #

session_name="color_detect_baseline"
genov_repo=$AROV/genov
experiment_repo=$HOME/workspace_tools/vitis/richie-acc-bench/colordetect/sw/app

declare -a experiment_name=( \
        "baseline" )

declare -a executable_path=( \
        "baseline" )

declare -a pulp_sdk_name=( \
        "arov_color_detect" )

declare -a target_ov=( \
        "agile_color_detect_baseline" )

n_experiment=${#experiment_name[@]}

          # Session - START #

# Launch new experiments session!
open_session $session_name

#           # Run experiments #

for ((experiment_id=0; experiment_id<n_experiment; experiment_id++)); do

  # Wait for previous window to advance its operations
  if [ "$experiment_id" -ge 1 ]; then
    sleep 30 ;
  fi
  
  echo -e "\nInitializing experiment #$experiment_id: <${experiment_name[$experiment_id]}>";

  # Create one window for each experiment of the current session
  if [ "$experiment_id" -ge 1 ]; then
    tmux new-window -t $session_name:$experiment_id -n ${experiment_name[$experiment_id]};
  else
    tmux rename-window -t $session_name:$experiment_id ${experiment_name[$experiment_id]}; 
  fi

  # Adapt HERO configuration file
  cfg_file=$HERO_OV_HOME_DIR/local.cfg
  if [ -f ${cfg_file} ] && grep -q OV_CFG_DEV ${cfg_file}; then
    echo "HERO configuration file -> Configured to ${pulp_sdk_name[$experiment_id]}"
    sed -c -i "s/\($OV_CFG_DEV *= *\).*/\1${pulp_sdk_name[$experiment_id]}/" $HERO_OV_HOME_DIR/local.cfg
  else
    echo "HERO configuration file -> Not found"
  fi

  # Experiment setup (env, SDK)
  setup_experiment $session_name $experiment_id

  # Generate HW
  generate_hw $session_name $experiment_id $genov_repo ${target_ov[$experiment_id]};

  # Compile SW (one per time)
  compile_experiment $session_name $experiment_id $experiment_repo ${experiment_name[$experiment_id]} ${executable_path[$experiment_id]};

  # Launch experiment
  run_experiment $session_name $experiment_id $experiment_repo ${experiment_name[$experiment_id]} ${executable_path[$experiment_id]} ${target_ov[$experiment_id]};
  
done

          # Session - END #

# Close experiments session once all experiments are terminated
# close_session $session_name