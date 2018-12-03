SECTION .text
global led_on
led_on:
	enter 0, 0
	mov eax, [ebp+12]
	mov ecx, 18
	sub ecx, eax
	mov ebx, 262143
	shl ebx, cl
	mov eax, ebx

	leave
	ret