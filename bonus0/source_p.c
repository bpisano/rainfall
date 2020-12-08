void p(char *dest, char *s)
{
    undefined *puVar1;
    char *var_1008h;
    
    puts(s);
    read(0, &var_1008h, 0x1000);
    puVar1 = (undefined *)strchr(&var_1008h, 10);
    *puVar1 = 0;
    strncpy(dest, &var_1008h, 0x14);
    return;
}
