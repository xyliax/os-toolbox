import os

x = dict()
addresses = open("pc_data").read()
for pc in addresses.split():
	x[pc] = x.get(pc, 0) + 1
sorted_pc = dict(sorted(x.items(), key=lambda item: (item[1], int(item[0], 16))))
print('finish sorting {} pc adresses'.format(len(addresses.split())), file = sys.stderr)

mapping = dict()
lines = open("fmaps").readlines()
for line in lines:
	(rg, owner) = (line.split()[0], line.split()[-1])
	if owner == '0' or owner[0] == '[':
		continue
	mapping[rg] = owner
	print('finish mapping address range - {}'.format(owner), file = sys.stderr)

insmap = dict()
for owner in mapping.values():
	if owner == '0' or owner[0] == '[':
		continue
	insmap[owner] = dict()
	res = os.popen("objdump -C -d {}".format(owner))
	for line in res.readlines():
		if line[0] != ' ':
			continue
		insmap[owner][int(line.split()[0][:-1], 16)] = line
	print('finish mapping instructions - {}'.format(owner), file = sys.stderr)

for pc in sorted_pc.keys():
	owner = "???"
	for rg in mapping.keys():
		(lb, hb) = rg.split('-')
		if int(pc, 16) >= int(lb, 16) and int(pc, 16) < int(hb, 16):
			owner = mapping[rg]
			break
	print(pc + ' ' + '\033[33m' + str(sorted_pc[pc]) + '\t' + '\033[32m' + owner + '\033[0m')
	if owner != "???":
		offset = hex(int(pc, 16) - int(lb, 16))
		#cmd = "objdump --start-address={0} --stop-address={1} -C -S {2}".format(offset, hex(int(offset, 16) + 4), owner)
		#res=os.popen(cmd)
		print('\033[31m' + insmap[owner][int(offset, 16)] + '\033[0m')
