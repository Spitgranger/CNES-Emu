#!/bin/bash

SESH="CNES-EMU"

tmux has-session -t $SESH 2>/dev/null

if [ $? != 0 ]; then
	tmux new-session -d -s $SESH -n "editor"
	tmux send-keys -t $SESH:editor "cd ~/Documents/Projects/CNES-Emu/" C-m
	tmux send-keys -t $SESH:editor "nvim ." C-m

	tmux new-window -t $SESH -n "cmake"
	tmux send-keys -t $SESH:server "cd ~/Documents/Projects/CNES-Emu/" C-m
	tmux select-window -t $SESH:editor
fi
tmux attach-session -t $SESH
