.code

AntiDebug proc
	mov rax, qword ptr gs:[60h]
	movzx eax, byte ptr [rax+2]
	ret
AntiDebug endp

end