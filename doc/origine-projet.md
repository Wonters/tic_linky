# Origine du projet

## En bref

`ha_tic_linky` est un firmware **ESP-IDF** pour **ESP32-H2** qui lit le signal **TIC** d'un compteur Linky (mode standard), en extrait l'intensité instantanée et la puissance apparente, puis les publie sur le réseau Zigbee via des clusters **Analog Output** et **Analog Value**.

## Base Espressif

Le point de départ est l'exemple Zigbee Home Automation d'Espressif, adapté pour exposer des mesures électriques :

- [HA_on_off_light](https://github.com/espressif/esp-zigbee-sdk/tree/main/examples/esp_zigbee_HA_sample/HA_on_off_light) — structure Zigbee (commissioning, signal handler)
- SDK : [esp-zigbee-sdk](https://github.com/espressif/esp-zigbee-sdk)

Fichiers encore reconnaissables de l'exemple d'origine :

| Fichier | Origine |
| ------- | ------- |
| `main/esp_zb_tic_linky.c` | Logique Zigbee (signal handler, clusters, commissioning) |
| `main/esp_zb_tic_linky.h` | Constantes Zigbee (endpoint, config radio, rôle ED) |
| `main/light_driver.c` / `.h` | Pilote LED WS2812 (exemple Espressif, vendu dans `main/`) |

## Personnalisations Shift

Ajouts / modifications par rapport à l'exemple Espressif :

| Élément | Détail |
| ------- | ------ |
| `main/esp_zb_uart.c` / `.h` | Lecture UART TIC 1200 7E1, parsing `IINST` et `PAPP` |
| `main/blinker.c` / `.h` | Clignotements LED après jonction réseau |
| `main/esp_battery.c` / `.h` | Mesure tension batterie via ADC (présent mais **désactivé** dans le firmware actuel) |
| `main/simulation_linky.c` | Simulateur TIC autonome (non compilé — référence / test) |
| Endpoint Zigbee | `10` |
| Fabricant / modèle | `Shift` / `linky` |
| Rôle réseau | **End Device** (`CONFIG_ZB_ZED=y`) |
| Clusters exposés | Basic, Identify, Analog Output (courant), Analog Value (puissance) |
| `activate` | Script shell pour activer ESP-IDF et lancer `idf.py` |

## Évolution par rapport à l'exemple d'origine

L'ancien `CMakeLists.txt` racine référençait un dépôt externe :

```
../esp-zigbee-sdk/examples/common/light_driver
```

Ce chemin n'existe pas sur la machine de dev. Le pilote LED a été **intégré directement** dans `main/`, comme dans le projet `relay-zigbee`.

## Fichiers versionnés vs générés

| Fichier / dossier | Dans Git ? | Rôle |
| ----------------- | ---------- | ---- |
| `main/` | ✅ | Code source du firmware |
| `main/idf_component.yml` | ✅ | Manifeste des dépendances |
| `dependencies.lock` | ✅ | Versions figées au dernier build réussi |
| `sdkconfig` | ✅ (si commité) | Configuration Kconfig compilée |
| `sdkconfig.defaults` | ✅ | Valeurs par défaut du projet |
| `activate` | ✅ | Activation ESP-IDF + wrapper `idf.py` |
| `managed_components/` | ❌ (non suivi) | Téléchargé automatiquement au build |
| `build/` | ❌ | Artefacts de compilation |

## Dev Container (optionnel)

Le dossier `.devcontainer/` fournit un environnement Docker basé sur `espressif/idf`. Utile si tu ne veux pas configurer ESP-IDF en local. Voir [environnement-dev.md](environnement-dev.md).
