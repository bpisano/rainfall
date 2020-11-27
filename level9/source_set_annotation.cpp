void N::setAnnotation(char*)(void *s1, char *s)
{
    undefined4 uVar1;
    
    // N::setAnnotation(char*)
    uVar1 = strlen(s);
    memcpy((int32_t)s1 + 4, s, uVar1);
    return;
}
