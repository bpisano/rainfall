undefined4 main(undefined4 placeholder_0, char **envp)
{
    undefined4 *puVar1;
    undefined4 uVar2;
    undefined4 *puVar3;
    
    puVar1 = (undefined4 *)malloc(8);
    *puVar1 = 1;
    uVar2 = malloc(8);
    puVar1[1] = uVar2;
    puVar3 = (undefined4 *)malloc(8);
    *puVar3 = 2;
    uVar2 = malloc(8);
    puVar3[1] = uVar2;
    strcpy(puVar1[1], envp[1]);
    strcpy(puVar3[1], envp[2]);
    uVar2 = fopen("/home/user/level8/.pass", 0x80486e9);
    fgets(c, 0x44, uVar2);
    puts(0x8048703);
    return 0;
}
