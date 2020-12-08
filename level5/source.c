void n(void)
{
    char *format;
    
    fgets(&format, 0x200, _reloc.stdin);
    printf(&format);
    exit(1);
    n(&stack0xfffffffc);
    return;
}
