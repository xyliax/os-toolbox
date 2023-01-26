all: trace

trace: main.c
	$(CC) $^ -o $@

result: count.py fmaps pc_data
	python3 $^ 2>/dev/null | sed 's|\x1b\[[;0-9]*m||g' > $@
	@echo "written to $@."

.PHONY: clean
clean:
	@rm -f trace fmaps pc_data
