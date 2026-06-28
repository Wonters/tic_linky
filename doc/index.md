# Documentation ha_tic_linky

Index de la documentation technique du projet. Chaque fichier couvre un sujet précis.

| Fichier | Sujet |
| ------- | ----- |
| [origine-projet.md](origine-projet.md) | D'où vient le projet, base Espressif, personnalisations Shift |
| [managed-components.md](managed-components.md) | `managed_components/` : origine, installation, mise à jour |
| [environnement-dev.md](environnement-dev.md) | ESP-IDF, Python, script `activate` |
| [build-et-flash.md](build-et-flash.md) | Compiler, flasher, monitorer, effacer la flash |
| [architecture-code.md](architecture-code.md) | Structure du code, tâches FreeRTOS, flux d'exécution |
| [teleinfo-tic.md](teleinfo-tic.md) | Protocole TIC Linky, UART, parsing, simulation |
| [zigbee-reseau.md](zigbee-reseau.md) | Rôle réseau, clusters, commissioning, Zigbee2MQTT |
| [materiel.md](materiel.md) | ESP32-H2, UART TIC, LED WS2812, GPIO |
| [partitions-nvs.md](partitions-nvs.md) | Table de partitions, stockage Zigbee / NVS |

## Démarrage rapide

Si tu reprends le projet sans te souvenir de grand-chose :

1. Lire [origine-projet.md](origine-projet.md) — contexte général
2. Lire [managed-components.md](managed-components.md) — les libs Zigbee sont téléchargées automatiquement au build
3. Suivre [environnement-dev.md](environnement-dev.md) puis [build-et-flash.md](build-et-flash.md)
4. Pour la TIC Linky : [teleinfo-tic.md](teleinfo-tic.md)
5. Pour le réseau Zigbee : [zigbee-reseau.md](zigbee-reseau.md)
