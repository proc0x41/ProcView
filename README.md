# procview

Ferramenta de linha de comando para inspeção de processos no Windows, construída com a Win32 API.

```
PID      PPID     NOME                                         THREADS
======   ======   ==========================================   ========
4        0        System                                       150
888      4        smss.exe                                     2
1234     888      notepad.exe                                  3

Modules (notepad.exe)
BASE               SIZE           PATH
================   ============   ============================================================
00007FF6482E0000   20.48MB        C:\Windows\System32\notepad.exe
00007FFB42B20000   1.82MB         C:\Windows\System32\ntdll.dll

Memory:
    Working Set:   18.40MB
    Private Bytes: 6.10MB
```

## Funcionalidades

- Lista todos os processos em execução com PID, PPID, nome e quantidade de threads
- Filtra por PID ou nome do processo
- Enumera módulos carregados com endereço base, tamanho e caminho
- Exibe uso de memória (Working Set e Private Bytes)

## Uso

```
procview                     Lista todos os processos
procview --pid <PID>         Inspeciona um processo pelo PID
procview --name <nome>       Inspeciona um processo pelo nome
```

**Exemplos:**

```
procview --pid 1234
procview --name notepad.exe
```

## Compilando

Requer Visual Studio com o Windows SDK.

1. Clone o repositório
2. Abra a solução no Visual Studio
3. Compile em Release ou Debug (x64)

## Dependências

- `tlhelp32.h` — snapshots de processos e módulos
- `psapi.h` — informações de memória do processo

## O que aprendi construindo isso

Este projeto foi desenvolvido como parte dos meus estudos de Win32 API. Conceitos cobertos:

- `HANDLE` e o modelo de objetos do Windows
- `CreateToolhelp32Snapshot` para enumeração de processos e módulos
- `OpenProcess` e flags de acesso 
- `PROCESS_MEMORY_COUNTERS_EX` e métricas de memória
- Wide strings (`wchar_t`, `LPCWSTR`) e saída Unicode no console
- Bitwise OR para combinação de flags de acesso
- Versionamento de structs com `cb`/`dwSize`
- Cast entre structs compatíveis (`PROCESS_MEMORY_COUNTERS` ↔ `_EX`)

---

> Parte da minha trilha de estudos de Windows internals. Mais ferramentas em breve.
