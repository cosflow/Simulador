#!/bin/bash

set -e

LGPIO_REPO="https://github.com/joan2937/lg.git"

INSTALL_USER="$(logname)"
INSTALL_HOME="$(eval echo ~$INSTALL_USER)"

echo "Instalando dependencias..."
sudo apt update
sudo apt install -y 
git 
unzip 
wget 
swig 
build-essential 
python3-dev 
python3-setuptools

echo "Instalando lgpio..."
cd /tmp

if [ -d lg] ; then
rm -rf lg
fi

git clone "$LGPIO_REPO"

cd lg

make clean
make

sudo make install


cd "$INSTALL_HOME"

if [ -d Simulador ]; then
rm -rf Simulador
fi

git clone "$SIM_REPO"

echo "Instalando servicios..."

sudo cp 
"$INSTALL_HOME/Simulador/Servicios/"*.service 
/etc/systemd/system/

sudo systemctl daemon-reload

sudo systemctl enable vibrar.service
sudo systemctl enable raton.service

sudo systemctl restart vibrar.service
sudo systemctl restart raton.service

echo
echo "Instalación completada."
echo
