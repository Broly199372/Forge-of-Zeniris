#!/usr/bin/env bash
set -e

echo "Atualizando pacotes..."
sudo apt-get update -y

echo "Instalando ferramentas de build e bibliotecas de dependência..."
sudo apt-get install -y build-essential cmake pkg-config curl libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev libsdl2-dev

echo "Instalando Rust via rustup..."
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y

echo "Para ativar o Rust nesta sessão, execute: source ~/.cargo/env"
echo "Feito. Agora você tem Rust, cargo, g++, cmake e bibliotecas básicas instaladas para começar a engine."