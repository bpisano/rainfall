void o(void)
{
    undefined auStack552 [520];
    code *pcStack32;
    char *pcStack28;
    
    pcStack28 = "/bin/sh";
    pcStack32 = (code *)0x80484b6;
    system();
    pcStack28 = (char *)0x1;
    pcStack32 = n;
    _exit();
    pcStack32 = (code *)&stack0xfffffffc;
    fgets(auStack552, 0x200, _reloc.stdin);
    printf(auStack552);
    exit(1);
    n();
    return;
}
