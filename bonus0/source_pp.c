void pp(char *dest)
{
    char cVar1;
    uint32_t uVar2;
    char *pcVar3;
    uint8_t uVar4;
    char *var_3ch;
    int32_t var_30h;
    int32_t var_1ch;
    
    uVar4 = 0;
    p((char *)&var_30h, (char *)0x80486a0);
    p((char *)&var_1ch, (char *)0x80486a0);
    strcpy(dest, &var_30h);
    uVar2 = 0xffffffff;
    pcVar3 = dest;
    do {
        if (uVar2 == 0) break;
        uVar2 = uVar2 - 1;
        cVar1 = *pcVar3;
        pcVar3 = pcVar3 + (uint32_t)uVar4 * -2 + 1;
    } while (cVar1 != '\0');
    *(undefined2 *)(dest + (~uVar2 - 1)) = *(undefined2 *)0x80486a4;
    strcat(dest, &var_1ch);
    return;
}
