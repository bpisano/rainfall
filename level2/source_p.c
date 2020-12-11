void p(void)
{
    uint32_t unaff_retaddr;
    char *src;
    int32_t var_ch;
    
    fflush(_reloc.stdout);
    gets(&src);
    if ((unaff_retaddr & 0xb0000000) == 0xb0000000) {
        printf("(%p)\n", unaff_retaddr);
        _exit(1);
    }
    puts(&src);
    strdup(&src);
    return;
}
