void main(undefined4 placeholder_0, char **envp)
{
    undefined4 uVar1;
    code **ppcVar2;
    
    uVar1 = malloc(0x40);
    ppcVar2 = (code **)malloc(4);
    *ppcVar2 = m;
    strcpy(uVar1, envp[1]);
    (**ppcVar2)();
    return;
}
