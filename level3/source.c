void v(void)
{
    char *format;
    
    fgets(&format, 0x200, _reloc.stdin);
    printf(&format);
    if (_m == 0x40) {
        fwrite("Wait what?!\n", 1, 0xc, _reloc.stdout);
        system("/bin/sh");
    }
    return;
}
