void n(void)
{
    char *s;
    
    fgets(&s, 0x200, _reloc.stdin);
    p((char *)&s);
    if (_m == 0x1025544) {
        system("/bin/cat /home/user/level5/.pass");
    }
    return;
}
