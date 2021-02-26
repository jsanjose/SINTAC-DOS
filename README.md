# SINTAC

Todas las versiones de SINTAC para MS-DOS.

## SINTAC T1

Primera versión del sistema SINTAC basado en el tan usado PAWS para Spectrum.

El SINTAC pretende ser una revisión del PAWS para ordenadores de la gama PC con importantes mejoras sobre este antiguo, pero aún válido, sistema como son los saltos, posibilidad de hacer bucles, ventanas, ...

Esta primera versión del sistema sólo soporta el modo texto de 80x25 en cualquier tarjeta compatible CGA.

## SINTAC T2

La versión T2 incorpora importantes mejoras. Ahora el compilador se ajusta automáticamente a la cantidad de memoria disponible, además el orden de las secciones dentro de la base de datos es indiferente y se permite el uso de constantes simbólicas lo cual dota de mayor flexibilidad al sistema.

Se ha corregido un pequeño error del compilador que provocaba que las bases de datos compiladas fuesen 1 byte más largas de lo debido. La gestión de ventanas también ha sido mejorada, permitiendo ahora que estas tengan marco.

Se ha ampliado la rutina de INPUT permitiendo mayor control del cursor (arriba, abajo, fin, origen...). Además ahora el condacto INPUT devuelve un código si se pulsó una tecla de función lo que permite asignar acciones especiales a las teclas de función.

Como consecuencia de la inclusión de juegos de caracteres se ha añadido una nueva utilidad al sistema: el editor de juegos de caracteres; ahora el sistema permite el uso de diferentes juegos de caracteres si se dispone de tarjeta EGA, VGA o compatible.

Se ha incrementado el número de letras significativas de las palabras de vocabulario a 6 y se han incorporado nuevos condactos, entre ellos destaca un condacto EXTERN para ejecutar programas externos.

El sistema se suministra con algunas utilidades externas; entre ellas un visualizador de ficheros gráficos PCX que permite por medio del condacto EXTERN incluir gráficos en las aventuras.

## SINTAC G1

La versión G1 permite la incorporación de gráficos en formato PCX dentro de la aventura por medio del nuevo condacto GRAPHIC.

Esta versión soporta dos modos gráficos, el de 640x480 a 16 colores y el de 320x200 a 256 colores los cuales pueden seleccionarse a través del condacto MODE. Esta nueva versión sólo funcionará con tarjetas VGA o compatibles. También se puede modificar la paleta de colores usando el condacto REMAPC.

Además de este se han añadido condactos para el control del tiempo como el condacto GTIME que devuelve la hora real del sistema y los condactos TIME y TIMEOUT que permiten ajustar la cantidad de tiempo de que se dispone para teclear una frase por medio del condacto INPUT.

El condacto INPUT ha sido revisado y ahora permite repetir la última frase introducida sin más que pulsar la tecla de cursor arriba al inicio de la línea.
En esta versión se dispone de un completo entorno de programación con editor integrado. El entorno ha sido desarrollado para facilitar la tarea de crear la base de datos, compilarla y ejecutarla, tareas estas que se pueden desarrollar todas ellas desde dentro del entorno, además incorpora un completo sistema de ayuda en línea que incluye, entre otras cosas, una completa lista de condactos y variables del sistema.

El compilador también ha sido mejorado y ya no es necesario definir los mensajes, localidades, objetos o procesos consecutivos, se pueden dejar 'huecos' con mensajes (localidades, objetos o procesos) sin definir.

Se incluye una nueva versión del generador de juegos de caracteres que soporta las nuevas fuentes de 8x16 y de 8x8 y además incluye cuadros de selección de ficheros y cuadros diálogo.

## SINTAC G2

Corregido un fallo en la documentación que hablaba sobre el condacto COPYOF (inexistente en el SINTAC) cuando en realidad debería tratarse del condacto COPYOV.

Se ha modificado el condacto GRAPHIC que ahora puede cargar gráficos en memoria y presentarlos al final de la carga. Ha sido añadido el condacto GETRGB que devuelve las componentes RGB de un color. Los condactos LISTAT y LISTOBJ han sido ampliados para permitir que si no hay objetos que listar no se imprima nada.

Se han expandido el número de mensajes disponibles. En esta versión se dispone de 255 tablas de mensajes con 255 mensajes cada tabla. A consecuencia de ello también han sido modificados los condactos MES y MESSAGE a los que se añade como nuevo parámetro el número de tabla del mensaje a imprimir.

Se soporta indirección en todos los parámetros y no sólo en los dos primeros como sucedía en versiones anteriores.

El entorno ha sido notablemente mejorado con soporte de ratón, cuadros de diálogo y ayuda en línea mejorada.

Se incluye un 'linkador' que permite generar ficheros ejecutables a partir de las bases de datos compiladas.

## SINTAC G3

Los gráficos ahora pueden agruparse. Cada fichero gráfico ahora puede contener hasta 256 gráficos lo cual elimina el problema de tener una gran cantidad de ficheros.

Se ha añadido un completo soporte del ratón. Además se ha añadido sonido a través del altavoz del PC. Se pueden reproducir ficheros con melodías para lo cual se suministra un programa que permite su creación.

Ahora se pueden utilizar juegos de caracteres proporcionales. El generador de caracteres se ha modificado para soportar esta característica.

El entorno de programación ha sufrido ligeras mejoras. La más destacable es que ahora, cuando se ejecuta un programa externo desde el entorno se realiza un volcado de memoria a disco, o a EMS si el controlador correspondiente ha sido cargado, para liberar la máxima cantidad de memoria posible. Esto libera la mayoría de la memoría excepto unos 20K para el programa externo.
