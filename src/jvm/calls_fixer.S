.section .text
.global getMethodAddressAndPatch

getMethodAddressAndPatch:
	call getAddressForLoadedMethod
	addl $0x04, %esp
	movl (%esp), %ecx
	movl %eax, -6(%ecx)
	jmp *%eax
