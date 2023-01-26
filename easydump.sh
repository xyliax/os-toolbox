#!/bin/bash

objdump -C -S -l \
	--start-address=0x${1} \
	--stop-address=0x${2} \
	${3}
