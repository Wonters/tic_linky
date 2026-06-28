# managed_components — d'où viennent ces libs ?

## Réponse courte

**Tu ne les as pas installées à la main.** Le dossier `managed_components/` est créé et rempli **automatiquement** par l'**IDF Component Manager** d'Espressif lors du premier `idf.py build` (ou `idf.py reconfigure`).

La seule action manuelle : déclarer les dépendances dans `main/idf_component.yml`.

## Comment ça marche

```
main/idf_component.yml          ← manifeste
        │
        ▼  idf.py build
IDF Component Manager
        │
        ├── Télécharge depuis le registry Espressif (HTTPS)
        └── Écrit dans managed_components/
        │
        ▼
dependencies.lock               ← versions exactes figées
```

### Fichier source de vérité : `main/idf_component.yml`

```yaml
dependencies:
  espressif/esp-zboss-lib: "1.0.9"
  espressif/esp-zigbee-lib: "1.0.9"
  espressif/led_strip: "~2.0.0"
  idf:
    version: ">=5.0.0"
```

### Fichier de lock : `dependencies.lock`

Généré automatiquement. Contient les versions exactes et les hash de chaque composant. **À committer** pour reproduire le même build sur une autre machine.

État actuel :

| Composant | Version | Source |
| --------- | ------- | ------ |
| `espressif/esp-zboss-lib` | 1.0.9 | [components.espressif.com](https://components.espressif.com/) |
| `espressif/esp-zigbee-lib` | 1.0.9 | [components.espressif.com](https://components.espressif.com/) |
| `espressif/led_strip` | 2.0.0 | [components.espressif.com](https://components.espressif.com/) |
| `idf` | 5.2.1 | ESP-IDF local |

## Composants dans le projet

### `espressif/esp-zboss-lib` + `espressif/esp-zigbee-lib`

Stack Zigbee (ZBOSS + API ESP Zigbee SDK). Utilisée par `esp_zb_tic_linky.c` pour le commissioning, les clusters ZCL et la publication des mesures.

### `espressif/led_strip`

Driver WS2812 via RMT. Utilisé par `main/light_driver.c` pour la LED de statut.

## Ce qui n'est PAS dans managed_components

Ces fichiers sont **dans `main/`** et versionnés avec le projet :

| Fichier | Rôle |
| ------- | ---- |
| `light_driver.c` / `.h` | Pilote LED (vendu depuis l'exemple Espressif) |
| `blinker.c` / `.h` | Clignotements LED au join réseau |
| `esp_zb_uart.c` / `.h` | Lecture et parsing TIC |
| `esp_battery.c` / `.h` | Mesure ADC batterie (désactivé) |

## Mettre à jour une dépendance

1. Modifier la version dans `main/idf_component.yml`
2. Rebuilder :

```bash
./activate build
```

3. Vérifier que `dependencies.lock` a changé
4. Committer `idf_component.yml` + `dependencies.lock`

## Supprimer et régénérer

```bash
rm -rf managed_components/
./activate fullclean
./activate build
```

## `.gitignore`

Le dossier `managed_components/` n'est en principe **pas versionné**. Seuls `idf_component.yml` et `dependencies.lock` suffisent pour le reconstruire.
