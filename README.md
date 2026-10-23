# VulkanTemplate


# À propos

Ceci est un programme recréant toute la pipeline 3D, afin de récréer l'équivalent du VulkanCube.
Il pourra servir comme base à de futur projets utilisant vulkan

## Pré-requis

Il faut le SDK vulkan 1.4.357.0

Pour le mettre en place, utilisez soit
[le .exe](https://sdk.lunarg.com/sdk/download/1.4.357.0/windows/vulkansdk-windows-X64-1.4.357.0.exe)
  |  [ou le .ZIP](https://sdk.lunarg.com/sdk/download/1.4.357.0/windows/VulkanRT-X64-1.4.357.0-Components.zip)
(à mettre dans C:\VulkanSDK\1.4.357.0)

## Contrôles

*En configuration fr_FR.UTF-8*
Les **flèches directionnelles** contrôlent la rotation du cube, ainsi que les touches **E** et **A**.
Les touches **Q** et **D** contrôlent la caméra.


# Fonctionnement interne

Le programme créer la fenêtre, l'application, va sélectionner le **processeur** (CPU), la **carte graphique** (GPU) en fonction des paramètres requis (capable d'afficher des images en 3D par exemple)

L'application va créer une **swap chain**, qui va stocker une liste d'images attendant d'être affiché à l'écran.

Une fois que tout cela est mis en place, la pipeline graphique va être mis en place, à savoir qu'elle va gérer les **shaders**, le **viewport** (ce qui sera visible à l'écran), le **rasterizer** (pour que le GPU puisse comprendre les informations données et créer des images 3D), ainsi que la **couleur**

Afin de pouvoir transférer des données plus aisément, sans trop empiéter sur la charge de la mémoire, on utilise les **buffers**. On l'utilise pour les vertices, les indexs (pour réutiliser des vertexs déjà établis) et pour uniformiser plusieurs vertices (on stocke plusieurs vertices dans un seul et même buffer, afin de les uniformiser comme un seul objet, que l'on peut modifier à souhait).

Tout les buffers sont gérés, commandés par les **command buffers**, qui vont gérer et allouer la mémoire pour stocker les buffers.

---

Pour l'instant, tout cela n'est que de la mise en place, afin de gérer efficacement chaque ressources à disposition.

Dernière étape avant la véritable mise en marche du programme, il faut créer les **descriptors**, des ressources permettant la communication entre les shaders et les ressources du GPU. Cela permet de gérer les différents paramètres des objets (position, couleur) dans un espace en 3D. 

Dès lors, il est possible de créer une image en 3D, la stocker dans la swap chain, la transformer en une image affichable pour un écran plat (donc en 2D), et enfin, l'afficher.

Le programme effectue une dernière chose, l'ajout d'une **texture**. Il rajoute scanne une image, la transforme en données pour la stocker dans un buffer, afin qu'elle puisse être incorporé au shaders, et placer sur un objet 3D.

