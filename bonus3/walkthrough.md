# `Atoi` Exploitation

En décompilant l'exécutable, on remarque un appel à `execl` avec comme argument `/bin/sh`. Pour être appelée, le programme va comparer 2 chaînes de caractères. Cette comparaison doit retourner `0`.
```c
iVar2 = strcmp(auStack152, envp[1]);
if (iVar2 == 0) {
    execl("/bin/sh", 0x8048707, 0);
}
```
Il est important de noter que nous ne pouvons pas utiliser `gdb` pour étudier la mémoire. En effet, le programme essaye d'ouvrir le fichier `.pass` de l'utilisateur `end` dont nous n'avons pas les droits.
```c
iStack20 = fopen("/home/user/end/.pass", 0x80486f0);
```
`fopen` retourne ici `0`. Une vérification est effectuée quelques lignes après pour vérifier que nous avons les droits sur le fichier `.pass`.
```c
if ((iStack20 == 0) || (argv != (char **)0x2)) {
    uVar1 = 0xffffffff;
} else {
    fread(auStack152, 1, 0x42, iStack20);
    [...]
    iVar2 = strcmp(auStack152, envp[1]);
    if (iVar2 == 0) {
        execl("/bin/sh", 0x8048707, 0);
    }
    [...]
}
```
Or, en exécutant le programme avec `gdb`, on l'exécute avec les droits de `bonus3`, ce qui nous interdit d'ouvrir le fichier `.pass` et ainsi d'observer ce qu'il se passe avant l'appel à `execl`. Un `jump` ou même un changement de variable est impossible puisqu'un `file descriptor` fixé manuellement provoque un `SEGFAULT` lors de l'appel à la fonction `fread`.

Le code étant relativement court et facile à comprendre, nous allons donc l'expliquer pas à pas.

Il nous parait important de noter 2 parties dans le code.

---

**Lecture du fichier `.pass`**
```c
fread(auStack152, 1, 0x42, iStack20);
uStack87 = 0;
iVar2 = atoi(envp[1]);
*(undefined *)((int32_t)auStack152 + iVar2) = 0;
fread(auStack86, 1, 0x41, iStack20);
fclose(iStack20);
```
Cette partie copie le contenu du fichier `.pass` dans la variable `auStack152`. On remarque un appel à `atoi` qui permet de convertir notre premier argument en `integer`. Cet entier sera utilisé pour décaler l'adresse de `auStack152` afin d'insérer un `\0`. Autrement dit, l'instruction suivant l'appel à `atoi` place un `\0` à l'index `iVar2` dans `auStack152`. Pour résumer :
```c
*(undefined *)((int32_t)auStack152 + iVar2) = 0;
``` 
Équivaut à :
```c
auStack152[iVar2] = 0;
```

**Comparaison**
```c
iVar2 = strcmp(auStack152, envp[1]);
if (iVar2 == 0) {
    execl("/bin/sh", 0x8048707, 0);
} else {
    puts(auStack86);
}
```
Cette partie compare les chaînes de caractère `auStack152` à notre premier argument. Si le retour de `strcmp` est `0`, un `shell` est lancé.

---

Il semble évident, à partir de ces constatations, que nous allons devoir envoyer un argument précis à notre programme pour passer les différentes conditions menant à l'appel de la fonction `execl`.

La fonction `strcmp` renvoie `0` si les deux chaînes sont identiques. Dans le cas contraire, elle renvoie la différence entre les deux caractères qui diffèrent. Il pourrait être intéressant de placer le `\0` au début de `auStack152` et ainsi comparer une chaîne de caractère vide avec notre premier argument vide lui aussi. Seulement, en envoyant `0` en argument de notre programme, `atoi` va bien convertir notre premier argument en `integer` `0`. En revanche, la comparaison va se faire entre la chaîne vide `auStack152`, et la chaine de caractère contenant le caractère `0`.

Il nous faut alors trouver un argument qui permet à `atoi` de renvoyer `0`, et qui permet de construire une chaîne vide pour `strcmp`.

`atoi` renvoi `0` si il reçoit en argument une chaîne de caractère vide. C'est parfait puisque cela nous permettra de :
1. Placer le `\0` au début de `auStack152`.
2. Comparer 2 chaînes vides.

Ainsi, en passant en argument une chaîne vide à notre programme, nous sommes capable de passer toutes les conditions et de lancer un `shell` :
```
> ./bonus3 ""
$ cat /home/user/end/.pass
3321b6f81659f9a71c76616f606e4b50189cecfea611393d5d649f75e157353c
```
