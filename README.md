# VulkanTemplate


## À propos

Ce programme recrée les principales étapes d’un pipeline graphique 3D avec Vulkan, afin de produire un résultat comparable à celui de l’exemple *Vulkan Cube*.

Il pourra servir de base à de futurs projets utilisant Vulkan.

## Pré-requis

Il faut le SDK vulkan **1.4.357.0**

Vous pouvez l’installer à l’aide de l’un des fichiers suivants :

- [Installateur Windows (.exe)](https://sdk.lunarg.com/sdk/download/1.4.357.0/windows/vulkansdk-windows-X64-1.4.357.0.exe)
- [Archive de composants (.zip)](https://sdk.lunarg.com/sdk/download/1.4.357.0/windows/VulkanRT-X64-1.4.357.0-Components.zip)
  
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



## Initialisation des ressources

Au démarrage, le programme initialise Vulkan dans l’ordre suivant :

1. Il crée une **instance Vulkan**, puis configure le callback de débogage.
2. Il crée la **surface** associée à la fenêtre.
3. Il sélectionne un périphérique physique compatible, puis crée le **périphérique logique** et les files d’attente nécessaires.
4. Il crée la **swap chain**, puis une *image view* pour chacune de ses images.
5. Il crée la disposition des descripteurs (*descriptor set layout*), puis la pipeline graphique.
6. Il crée le **command pool**, qui servira à allouer les command buffers.
7. Il crée les ressources de profondeur.
8. Il charge la texture dans une image Vulkan, puis crée l’image view et le sampler associés.
9. Il crée les buffers de sommets, d’indices et uniformes.
10. Il crée le pool de descripteurs, puis les descripteurs qui relient les ressources — notamment les buffers uniformes et la texture — aux shaders.
11. Il alloue les command buffers.
12. Enfin, il crée les objets de synchronisation utilisés pour coordonner le CPU, le GPU et la présentation des images.

Une fois l’initialisation terminée, le programme peut enregistrer et soumettre les commandes de rendu, puis présenter les images de la swap chain à l’écran.
