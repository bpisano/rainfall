undefined4 main(undefined4 placeholder_0, char **envp)
{
    undefined4 uVar1;
    undefined auStack60 [40];
    int32_t iStack20;
    
    iStack20 = atoi(envp[1]);
    if (iStack20 < 10) {
        memcpy(auStack60, envp[2], iStack20 * 4);
        if (iStack20 == 0x574f4c46) {
            execl("/bin/sh", 0x8048580, 0);
        }
        uVar1 = 0;
    } else {
        uVar1 = 1;
    }
    return uVar1;
}