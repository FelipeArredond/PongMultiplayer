# Pong Game

## Introducción

Este proyecto consiste en la implementación de un juego Pong con un servidor en lenguaje C y un cliente en lenguaje Python. El servidor está alojado en una instancia de Amazon EC2, lo que permite a los jugadores conectarse y jugar en línea.

## Desarrollo

### Servidor (Lenguaje C)

El servidor se encarga de gestionar la conexión de los jugadores y la lógica del juego Pong. Está alojado en una instancia de Amazon EC2 y se puede acceder a través de la dirección IP de la instancia.

- Escucha en un puerto especificado y espera la conexión de dos jugadores.
- Administra los puntajes de los jugadores y detecta cuándo se gana el juego.
- Utiliza hilos para manejar las interacciones entre los jugadores de forma concurrente.
- Comunica la velocidad de la pelota y las acciones de los jugadores a través de sockets.

### Cliente (Lenguaje Python)

El cliente es la interfaz gráfica para que los jugadores jueguen al Pong. Los jugadores pueden conectarse al servidor en la instancia EC2 y jugar en línea.

- Utiliza la biblioteca Pygame para crear la interfaz gráfica y manejar la entrada del jugador.
- Se conecta al servidor en la instancia de Amazon EC2 para unirse a la partida.
- Los jugadores pueden mover sus paletas arriba y abajo para golpear la pelota y ganar puntos.
- El cliente muestra el puntaje de los jugadores y detecta cuándo se ha ganado el juego.

## Conclusiones

Este proyecto destaca varios aspectos importantes:

- La concurrencia en el servidor permite gestionar múltiples jugadores de manera eficiente. Los hilos se utilizan para manejar las interacciones entre jugadores de forma simultánea, lo que garantiza una experiencia de juego sin interrupciones.

- El manejo del retraso en el juego es esencial para ofrecer una experiencia de juego fluida. La comunicación a través de sockets permite la sincronización de la velocidad de la pelota y las acciones de los jugadores, lo que evita retrasos notables.

- La combinación de un servidor en lenguaje C y un cliente en Python ilustra cómo crear un juego en línea completo con una arquitectura cliente-servidor, lo que facilita la participación de jugadores en línea desde diferentes ubicaciones.

## Referencias

- Wikipedia contributors. (2023). Berkeley Sockets. Wikipedia. https://en.wikipedia.org/wiki/Berkeley_sockets

- The Berkeley Sockets API. https://csperkins.org/teaching/2007-2008/networked-systems/lecture04.pdf.

- Jacob Sorber. How to create and join threads in C (Pthreads). YouTube. diciembre 2018. https://www.youtube.com/watch?v=uA8X5zNOGw8.
