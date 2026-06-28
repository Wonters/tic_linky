# Environnement de développement

## Vue d'ensemble

| Élément | Valeur sur cette machine |
| ------- | ------------------------ |
| ESP-IDF | `/Volumes/Data02/programmes/HSP_zigbee_smarthome/esp-idf` |
| Version IDF | 5.2.1 (figée dans `dependencies.lock`) |
| Cible | `esp32h2` |
| Outils compilateur | `~/.espressif/tools/` (riscv32-esp-elf, openocd, esptool…) |
| Python ESP-IDF | `~/.espressif/python_env/idf5.2_py3.12_env` |
| Extension VS Code | `espressif.esp-idf-extension` (configurée dans `.vscode/`) |

## Script `activate`

À la racine du projet, le script `activate` simplifie l'activation de l'environnement :

```bash
cd /Volumes/Data02/shift/iot/ha_tic_linky

# Activer l'env dans le shell courant
source ./activate

# Lancer idf.py directement
./activate build
./activate -p /dev/tty.usbmodemFD3401 flash monitor

# Ouvrir un shell interactif avec l'env actif
./activate
```

Variables surchargeables :

| Variable | Défaut | Rôle |
| -------- | ------ | ---- |
| `IDF_PATH` | `/Volumes/Data02/programmes/HSP_zigbee_smarthome/esp-idf` | Chemin ESP-IDF |
| `PYTHON3` | `/usr/local/Caskroom/miniconda/base/bin/python` | Interpréteur Python 3.12 |

## Activation manuelle (sans script)

```bash
cd /Volumes/Data02/shift/iot/ha_tic_linky

# IMPORTANT : python3 système = 3.14, incompatible ESP-IDF 5.2
python3() { /usr/local/Caskroom/miniconda/base/bin/python "$@"; }

source /Volumes/Data02/programmes/HSP_zigbee_smarthome/esp-idf/export.sh
```

### Pourquoi forcer Python 3.12 ?

| Environnement | État |
| ------------- | ---- |
| `python3` système (Homebrew) | 3.14.x — **non supporté** par ESP-IDF 5.2 |
| `idf5.2_py3.12_env` | ✅ Fonctionnel (Miniconda 3.12.8) |

Emplacement des venvs Python ESP-IDF :

```
~/.espressif/python_env/
└── idf5.2_py3.12_env/   ← utiliser celui-ci
```

### Réinstaller l'environnement Python ESP-IDF (si nécessaire)

```bash
cd /Volumes/Data02/programmes/HSP_zigbee_smarthome/esp-idf
python3.12 ./install.sh esp32h2
```

Ne pas lancer `install.sh` avec Python 3.14.

## Première configuration du projet

```bash
./activate set-target esp32h2    # une seule fois
./activate menuconfig            # optionnel
./activate build                 # télécharge managed_components + compile
```

`set-target` génère / met à jour `sdkconfig` pour ESP32-H2.

## Dev Container (alternative Docker)

Fichiers : `.devcontainer/devcontainer.json` + `Dockerfile`

- Image de base : `espressif/idf:latest`
- ESP-IDF dans le conteneur : `/opt/esp/idf`
- Extensions VS Code : `espressif.esp-idf-extension`, `espressif.esp-idf-web`

Utile si tu ne veux pas maintenir ESP-IDF en local. Le chemin HSP Zigbee local n'est pas utilisé dans ce cas.

## Extension ESP-IDF (VS Code / Cursor)

Fichiers de config présents :

- `.vscode/settings.json`
- `.vscode/c_cpp_properties.json`
- `.vscode/launch.json`
- `.vscode/tasks.json`

Port série configuré : `/dev/tty.usbmodemFD3401` (à adapter selon ta carte).

## Vérifier que tout est OK

```bash
./activate --version
python --version          # doit afficher 3.12.x dans le venv ESP-IDF
riscv32-esp-elf-gcc --version
```

## Structure `~/.espressif/`

```
~/.espressif/
├── python_env/           # Virtualenvs Python pour idf.py, esptool, etc.
├── tools/                # Chaînes de compilation, OpenOCD, esptool
├── espidf.constraints.v5.2.txt
└── dist/                 # Archives téléchargées par install.sh
```

Ces outils sont **partagés entre tous les projets ESP-IDF** de la machine — pas spécifiques à `ha_tic_linky`.
