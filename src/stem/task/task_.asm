[GLOBAL read_eip]
read_eip:
	pop eax
	jmp eax

[GLOBAL task_idle]
task_idle:
	sti
	hlt
	jmp task_idle
