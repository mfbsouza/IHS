extern write

SECTION .text
global led_on
led_on:
	enter 4, 0
	mov eax, [ebp+12]
	mov ebx, 18
	sub ebx, eax
	mov ecx, 262143
	shl ecx, ebx
	mov [ebp-4], ecx

	mov eax, [ebp+8]
	mov ebx, ebp-4
	mov ecx, 6
	push ecx
	push ebx
	push eax
	leave 4
	ret