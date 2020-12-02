void main(char **argv, char **envp)
{
    void *s1;
    code **arg_8h;
    int32_t var_bp_4h;
    
    if ((int32_t)argv < 2) {
        _exit(1);
    }
    s1 = (void *)operator new(unsigned int)(0x6c);
    N::N(int)((int32_t)s1, 5);
    arg_8h = (code **)operator new(unsigned int)(0x6c);
    N::N(int)((int32_t)arg_8h, 6);
    N::setAnnotation(char*)(s1, envp[1]);
    (**(code **)*arg_8h)(arg_8h, s1);
    return;
}
