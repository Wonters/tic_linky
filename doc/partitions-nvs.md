# Partitions et stockage

## Table de partitions

Fichier : `partitions.csv`

| Nom | Type | Sous-type | Offset | Taille | Rôle |
| --- | ---- | --------- | ------ | ------ | ---- |
| `nvs` | data | nvs | 0x9000 | 24 Ko | Config générale ESP-IDF |
| `phy_init` | data | phy | 0xF000 | 4 Ko | Données calibration RF |
| `factory` | app | factory | 0x10000 | 900 Ko | **Firmware application** |
| `zb_storage` | data | fat | 0xF1000 | 16 Ko | Stockage stack Zigbee (ZBOSS) |
| `zb_fct` | data | fat | 0xF5000 | 1 Ko | Factory data Zigbee |

Config Kconfig associée (`sdkconfig.defaults`) :

```
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"
CONFIG_PARTITION_TABLE_OFFSET=0x8000
```

## Ce qui est stocké où

### NVS (`nvs`)

- Données générales ESP-IDF
- Initialisé dans `app_main()` via `nvs_flash_init()`

### `zb_storage` + `zb_fct`

- Credentials réseau Zigbee (PAN ID, clés, adresse courte…)
- Persistance entre reboots
- C'est ce qui permet à l'appareil de **se reconnecter** sans re-pairing complet

### `factory`

- Image `HA_TIC_LINKY.bin` (~550 Ko au dernier build)
- ~39 % d'espace libre restant dans la partition (900 Ko alloués)

## Effacer le stockage

| Action | Effet |
| ------ | ----- |
| `./activate -p PORT erase-flash` | Efface **tout** (NVS + Zigbee + firmware) |
| Re-flash sans erase | Garde les credentials Zigbee si la partition n'est pas écrasée |

Pour forcer un nouveau pairing Zigbee, faire `erase-flash` puis `flash`.

## Taille flash

Flash configurée en **2 Mo** (`CONFIG_ESPTOOLPY_FLASHSIZE_2MB`).

Offsets flash lors du programmation :

| Offset | Contenu |
| ------ | ------- |
| 0x0 | Bootloader |
| 0x8000 | Partition table |
| 0x10000 | Application |

Commande générée par le build :

```bash
python -m esptool --chip esp32h2 -b 460800 \
  --before default_reset --after hard_reset write_flash \
  --flash_mode dio --flash_size 2MB --flash_freq 48m \
  0x0 build/bootloader/bootloader.bin \
  0x8000 build/partition_table/partition-table.bin \
  0x10000 build/HA_TIC_LINKY.bin
```

## Modifier la table de partitions

1. Éditer `partitions.csv`
2. Vérifier que les offsets ne se chevauchent pas
3. `./activate fullclean && ./activate build`
4. Re-flasher

> Augmenter la partition `factory` réduit l'espace disponible pour `zb_storage` — attention aux credentials Zigbee.

## Relation avec la TIC

Les mesures TIC (`IINST`, `PAPP`) ne sont **pas stockées** en flash. Elles sont :

1. Lues en temps réel depuis l'UART
2. Publiées sur Zigbee toutes les 5 secondes
3. Perdues au reboot (pas d'historique local)

Pour de l'historisation, utiliser le coordinateur (Zigbee2MQTT, Home Assistant, etc.).
