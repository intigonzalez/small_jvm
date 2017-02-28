.section .text
.global getMethodAddressAndPatch
.global loadClassCompileMethodAndPath

getMethodAddressAndPatch:
	call getAddressForLoadedMethod
	addl $0x04, %esp
	jmp patch


loadClassCompileMethodAndPath:
	call highlevel_loadClassCompileMethodAndPath
	addl $0x04, %esp
	jmp patch
# TODO FIXME
patch:
	movl (%esp), %ecx
	movl %eax, -6(%ecx)
	jmp *%rax
