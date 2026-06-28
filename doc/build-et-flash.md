# Build et flash

## Prérequis

Environnement ESP-IDF activé — voir [environnement-dev.md](environnement-dev.md).

## Commandes courantes

```bash
cd /Volumes/Data02/shift/iot/ha_tic_linky
source ./activate
```

Ou en une ligne :

```bash
./activate build
./activate -p PORT flash monitor
```

### Build seul

```bash
./activate build
```

Produit `build/HA_TIC_LINKY.bin` (+ bootloader, partition table).

Au premier build, le Component Manager télécharge `managed_components/` — voir [managed-components.md](managed-components.md).

### Flash

Trouver le port série :

```bash
# macOS
ls /dev/cu.usbmodem* /dev/tty.usbmodem*

# Linux
ls /dev/ttyUSB* /dev/ttyACM*
```

Flasher :

```bash
./activate -p /dev/tty.usbmodemFD3401 flash
```

Remplacer le port par celui de ta carte.

### Flash + moniteur série

```bash
./activate -p PORT flash monitor
```

Quitter le moniteur : `Ctrl+]`

### Moniteur seul

```bash
./activate -p PORT monitor
```

### Effacer toute la flash

Utile pour repartir de zéro (réseau Zigbee, NVS corrompu) :

```bash
./activate -p PORT erase-flash
```

Puis re-flasher :

```bash
./activate -p PORT flash
```

## Nettoyage

```bash
./activate clean          # supprime build/ (garde sdkconfig)
./activate fullclean      # supprime build/ + régénère cmake
```

## Première configuration

```bash
./activate set-target esp32h2   # une seule fois
./activate menuconfig           # optionnel — vérifier CONFIG_ZB_ZED=y
./activate build
```

## Configuration Kconfig importante

Dans `sdkconfig.defaults` :

```
CONFIG_ZB_ENABLED=y
CONFIG_ZB_ZED=y
```

Le firmware est compilé en **End Device**. Sans `CONFIG_ZB_ZED=y`, la compilation échoue avec :

```
#error Define ZB_ED_ROLE in idf.py menuconfig to compile light (End Device) source code.
```

## Taille binaire (dernier build)

| Artefact | Taille approximative |
| -------- | -------------------- |
| `HA_TIC_LINKY.bin` | ~550 Ko |
| Partition `factory` | 900 Ko (39 % libre) |

## Dépannage

| Symptôme | Solution |
| -------- | -------- |
| `idf.py: command not found` | `source ./activate` ou `./activate build` |
| Erreur CMake `EXTRA_COMPONENT_DIRS` | Le pilote `light_driver` est dans `main/` — ne pas référencer `esp-zigbee-sdk` en externe |
| Erreur Python / greenlet | Forcer Python 3.12 — voir [environnement-dev.md](environnement-dev.md) |
| `Permission denied` sur le port | `sudo` ou ajouter l'utilisateur au groupe `dialout` (Linux) |
| Build OK mais pas de join Zigbee | `erase-flash` puis re-flash — voir [zigbee-reseau.md](zigbee-reseau.md) |

## Logs attendus au démarrage

```
I ESP_ZB_TIC_LINKY: Zigbee stack initialized
I ESP_ZB_UART: UART initialized successfully
I ESP_ZB_UART: Teleinfo generator task started
I ESP_ZB_TIC_LINKY: Send data task started
I ESP_ZB_TIC_LINKY: Start network steering
I ESP_ZB_TIC_LINKY: Joined network successfully (...)
I ESP_ZB_TIC_LINKY: Données mises à jour via Zigbee : PAPP=... VA, IINST=... A
```
