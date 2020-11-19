## Level 0

On fait un `ls -la`
On voit que l'executable a les droits de level1
```
> ls -la
[...]
-rwsr-x---+ 1 level1 users  747441 Mar  6  2016 level0
```
On récupère le fichier level0 avec la commande scp : 

```
scp -P 4242 level0@192.168.56.1:/home/user/level0/level0 .
```
On décompile le fichier grace a cutter. On se rend compte en voyant le main, que le programme attend un argument. Si l’argument saisis correspond à `0x1a7`, alors un shell est lancé. 

`0x1a7` est égal à `403` en décimal

On fait donc `./level0 423`
Un shell s’ouvre. En faisant un `whoami` on voit qu'on est level1

On peut donc cat le fichier contenant le flag. 
```console
> ./level0 423  
$ whoami
[...]
level1
$ cat /home/user/level1/.pass
[...]
1fe8a524fa4bec01ca4ea2a869af2a02260d4a7d5fe7e7c24d8617e6dca12d3a
```
