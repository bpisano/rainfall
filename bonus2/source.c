undefined4 main(char **argv, char **envp)
{
    undefined4 uVar1;
    char *pcVar2;
    int32_t iVar3;
    undefined4 *puVar4;
    undefined4 *puVar5;
    uint8_t uVar6;
    char *arg_8h;
    char *pcStack172;
    undefined4 uStack168;
    undefined4 auStack96 [19];
    char *pcStack20;
    int32_t var_ch;
    
    uVar6 = 0;
    if (argv == (char **)0x3) {
        iVar3 = 0x13;
        puVar4 = auStack96;
        while (iVar3 != 0) {
            iVar3 = iVar3 + -1;
            *puVar4 = 0;
            puVar4 = puVar4 + 1;
        }
        pcStack172 = envp[1];
        uStack168 = 0x28;
        strncpy();
        pcStack172 = envp[2];
        uStack168 = 0x20;
        strncpy();
        arg_8h = "LANG";
        pcVar2 = (char *)getenv();
        pcStack20 = pcVar2;
        if (pcVar2 != (char *)0x0) {
            uStack168 = 2;
            pcStack172 = (char *)0x804873d;
            iVar3 = memcmp();
            if (iVar3 == 0) {
                _language = 1;
                arg_8h = pcVar2;
            } else {
                uStack168 = 2;
                pcStack172 = (char *)0x8048740;
                arg_8h = pcStack20;
                iVar3 = memcmp();
                if (iVar3 == 0) {
                    _language = 2;
                }
            }
        }
        iVar3 = 0x13;
        puVar4 = auStack96;
        puVar5 = (undefined4 *)&stack0xffffff50;
        while (iVar3 != 0) {
            iVar3 = iVar3 + -1;
            *puVar5 = *puVar4;
            puVar4 = puVar4 + (uint32_t)uVar6 * -2 + 1;
            puVar5 = puVar5 + (uint32_t)uVar6 * -2 + 1;
        }
        uVar1 = greetuser((int32_t)arg_8h);
    } else {
        uVar1 = 1;
    }
    return uVar1;
}
