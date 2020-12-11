void run(void)
{
    fwrite("Good... Wait what?\n", 1, 0x13, _reloc.stdout);
    system("/bin/sh");
    return;
}
