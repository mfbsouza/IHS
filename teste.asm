extern write
extern read
extern delay

SECTION .text
global teste
teste:
	enter 0, 0
	pusha

	; da um read do display e salva na variavel
	mov eax, [ebp+8]
	mov ebx, [ebp+12] ;endereço da variavel
	mov ecx, [ebp+16]
	push ecx
	push ebx
	push eax
	call read
	add esp, 12

	; modifica a o que leu e "apaga" os 16 bits mais significativos
	mov ecx, [ebp+12]
	mov eax, dword[ecx]
	mov edx, eax ; backup
	mov ebx, 0xFFFF0000
	or eax, ebx
	mov dword[ecx], eax
	
	; escreve valor modificado no fpga
	mov eax, [ebp+8]
	mov ebx, [ebp+12] ;endereço da variavel
	mov ecx, [ebp+16]
	push ecx
	push ebx
	push eax
	call write
	add esp, 12

	;delay
	mov ecx, [ebp+20]
	push ecx
	call delay
	add esp, 4

	; da um read do display e salva na variavel
	mov eax, [ebp+8]
	mov ebx, [ebp+12] ;endereço da variavel
	mov ecx, [ebp+16]
	push ecx
	push ebx
	push eax
	call read
	add esp, 12

	; modifica a o que leu e "acende" os 16 bits mais significativos
	mov ecx, [ebp+12]
	mov eax, dword[ecx]
	mov edx, eax ; backup
	mov ebx, 0x0000FFFF
	and eax, ebx
	mov dword[ecx], eax

	; escreve valor modificado no fpga
	mov eax, [ebp+8]
	mov ebx, [ebp+12] ;endereço da variavel
	mov ecx, [ebp+16]
	push ecx
	push ebx
	push eax
	call write
	add esp, 12

	popa
	leave
	ret