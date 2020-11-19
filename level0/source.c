undefined4 main(undefined4 placeholder_0, char **envp)
{
    int32_t iVar1;
    undefined4 uStack32;
    undefined4 uStack28;
    undefined4 uStack24;
    undefined4 uStack20;
    
    iVar1 = atoi(envp[1]);
    if (iVar1 == 0x1a7) {
        uStack32 = __strdup("/bin/sh");
        uStack28 = 0;
        uStack20 = getegid();
        uStack24 = geteuid();
        __setresgid(uStack20, uStack20, uStack20);
        setresuid(uStack24, uStack24, uStack24);
        execv("/bin/sh", &uStack32);
    } else {
        _IO_fwrite("No !\n", 1, 5, _stderr);
    }
    return 0;
}
