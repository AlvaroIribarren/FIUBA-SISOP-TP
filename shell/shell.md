# Lab: shell

### Búsqueda en $PATH
1. ¿Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estandar de C (libc) exec(3)? La diferencia es que la familia exec(3) son un front-end para la familia execve(2). Cada uno de los distintos miembros de esta familia brindan una comodidad extra para el usuario. Por ejemplo, hay variantes como execl y execv que pueden recibir los parámetros del comando a ejecutar como un vector o como una lista NULL terminated. Luego una mejora extra puede ser agregando una p, donde nos busca en la variable $PATH el comando a ejecutar, dándole la comodidad al programador de no insertar el path completo.
---

### Comandos built-in
1. ¿entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿por qué? ¿cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false):
La que se podría implementar sin ser un built-in es pwd. El cd tiene que ir si o sí como built-in porque lo que queremos modificar es el valor del directorio actual del proceso shell y un proceso hijo no puede cambiar el del padre, pero sí conocer cual es. El pwd se hace como built-in por razones de eficiencia, ya que es mas fácil que el mismo proceso imprima su directorio actual. En cambio de hacerlo en otro proceso perdería de muchas formas, debería crear un proceso que no era necesario, preguntar desde el hijo el parent pid, y luego llegar hasta la información interna del proceso y preguntar el current working directory. (desconozco si el hijo ya posee como atributo el directory del padre, pero de todas formas sigue siendo mas ineficiente este segundo approach).
---

### Variables de entorno adicionales
1.  ¿Por qué es necesario hacerlo luego de la llamada a fork(2)? 
    Esto es necesario porque de lo contrario las variables de entorno pasadas por parámetro terminarían guardadas en env, que es lo que queremos evitar. Es por esto que solo utilizamos el set una vez dentro del contexto del proceso hijo.
2. ¿el comportamiento es el mismo que en el primer caso? No, no lo es. Lo que sucede es que setenv agrega las variables a env, mientras que las llamadas de exec con variables de entorno no lo hace.
3.  Para que sea lo mismo lo que se tiene que hacer es llamar a setenv por cada elemento del array de variables de entorno, de esta manera sería lo mismo.
    
---

### Procesos en segundo plano
1. Detallar el mecanismo utilizado. Lo que hice fue quitar el wait del padre en caso de que sea un background process. De esta forma la shell vuelve a mostrar el prompt al usuario mientras el proceso hijo quedó ejecutando. Por cada comando que se ejecuta se recogen todos los hijos a los que no se les hizo wait (zombies).
---

### Flujo estándar
1. Investigar el significado de este tipo de redirección y explicar qué sucede con la salida de cat out.txt. Comparar con los resultados obtenidos anteriormente: En el ejemplo se presenta el comando:  
"ls -C /home /noexiste >out.txt 2>&1".
Y el out obtenido es el siguiente: 
```
ls: no se puede acceder a '/noexiste': No existe el archivo o el directorio
/home:
alvaro
```
Lo que sucedió aqui fue que encontró el directorio /home y por lo tanto escribió su salida a out.txt y luego fallo en encontrar el directorio /noexiste. Este se hubiese escrito en el archivo de error, pero aquí no tenemos uno sino que tenemos un &1. Esto significa que quiere que salga por salida estandar, que en este caso está referenciando al file desciptor del archivo out.txt. Finalmente quedarian apuntando stdout y stderr al archivo out.txt terminando toda la salida en este.

---

### Tuberías simples (pipes)
1. Investigar y describir brevemente el mecanismo que proporciona la syscall pipe(2), en particular la sincronización subyacente y los errores relacionados: 
Lo que nos proporciona la syscall pipe es, pasándole por parámetro un arreglo de 2 enteros, un canal de comunicación unidireccional. Este es utilizado para comunicar procesos una vez hecho el fork. El pipe o tubería posee 4 file descriptors aunque solo 2 de estos pueden ser utilizados ya que, como se dijo antes, la comunicación es unidireccional. La llamada pipe2, nos brinda un parámetro extra, que nos permite insertar flags a la función. Uno de estos es O_CLOEXEC, que indica que los file descriptors se cierren al realizar una llamada a exec.
---

### Pseudo-variables
Se investigaron e implementaron 3 variables mágicas estándar. 
- ?: Esta pseudo variable devuelve el estado de exit del proceso mas recientemente ejecutado.
- !: Devuelve el ID del proceso mas recientemente ubicado en el background.
- $: Devuelve el Id de la shell actual. Es similar a una llamada a getpid.
---

