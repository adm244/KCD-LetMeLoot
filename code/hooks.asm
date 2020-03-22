;This is free and unencumbered software released into the public domain.
;
;Anyone is free to copy, modify, publish, use, compile, sell, or
;distribute this software, either in source code form or as a compiled
;binary, for any purpose, commercial or non-commercial, and by any
;means.
;
;In jurisdictions that recognize copyright laws, the author or authors
;of this software dedicate any and all copyright interest in the
;software to the public domain. We make this dedication for the benefit
;of the public at large and to the detriment of our heirs and
;successors. We intend this dedication to be an overt act of
;relinquishment in perpetuity of all present and future rights to this
;software under copyright law.
;
;THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
;EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
;MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
;IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
;OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
;ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
;OTHER DEALINGS IN THE SOFTWARE.

;IMPORTANT(adm244): SCRATCH VERSION JUST TO GET IT UP WORKING

extern baseAddress : qword

extern OpenInventory_Address : qword
extern NotifyInventoryClosed_Address : qword

extern C_UIMenuEvents_OpenInventory : proc
extern C_UIMenuEvents_NotifyInventoryClosed : proc

.code

  OpenInventory_Hook proc
    push rcx
    push rdx
    push r8
    push r9
    
    sub rsp, 32
    call C_UIMenuEvents_OpenInventory
    add rsp, 32
    
    pop r9
    pop r8
    pop rdx
    pop rcx
    
    ; original instructions
    push rbp
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    
    mov rax, OpenInventory_Address
    add rax, 12
    jmp rax
  OpenInventory_Hook endp
  
  NotifyInventoryClosed_Hook proc
    push rcx
    
    sub rsp, 8
    call C_UIMenuEvents_NotifyInventoryClosed
    add rsp, 8
    
    pop rcx
    
    ; original instructions
    mov [rsp + 10h], rbx
    mov [rsp + 18h], rbp
    mov [rsp + 20h], rsi
    
    mov rax, NotifyInventoryClosed_Address
    add rax, 12
    jmp rax
  NotifyInventoryClosed_Hook endp
  
end
