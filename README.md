# Forge-of-Zeniris

## Configuração de desenvolvimento

O ambiente foi preparado para um motor de jogos em C++ e Rust.

Ferramentas instaladas:
- `g++` / `build-essential`
- `cmake`
- `pkg-config`
- `SDL2` e dependências de OpenGL
- `rustup` / `rustc` / `cargo`

Para reproduzir o setup em outro ambiente, execute:

```bash
./setup-dev.sh
source ~/.cargo/env
```

## Estrutura de pastas da engine

- `Core`
  - `Rendering`
  - `Physics`
  - `Input`
  - `Audio`
  - `SceneManagement`
  - `AssetLoading`
- `SecurityPersistence`
  - `SaveFiles`
  - `CryptoValidation`
  - `DataVersionControl`
  - `CorruptionProtection`
- `Extensibility`
  - `Plugins`
  - `ExternalAPI`
  - `ScriptingModding`
- `Zeniris`
  - `Language`
  - `ProjectScripts`
  - `EventsVariablesComponents`
