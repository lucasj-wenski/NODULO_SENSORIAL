# README.md - Documentação Técnica

## Visão Geral do Projeto
Este projeto integra um sistema de automação para estufas baseado em monitoramento IoT. A estrutura é dividida em três pilares fundamentais para a comunicação e controle.

## Estrutura do Repositório

### Código.gs (Backend)
Atua como o servidor no ecossistema Google Apps Script. É responsável por receber os dados enviados pelo hardware, armazená-los de forma organizada na planilha de registro e mediar a comunicação entre a interface web e o microcontrolador.

### index.html (Interface)
Consiste no painel de controle (dashboard) acessível via navegador. Sua função é apresentar os dados coletados em tempo real, exibir o status operacional do sistema e fornecer os comandos de controle para que o usuário possa interagir com a estufa.

### esp.c++ (Firmware)
Código fonte embarcado no microcontrolador ESP32. Este componente gerencia a leitura dos sensores físicos, mantém a conexão com a rede Wi-Fi, transmite os dados de telemetria para o servidor e processa as instruções de controle recebidas para atuar nos dispositivos da estufa.

## Pré-requisitos
*   **Google Sheets:** Planilha configurada com as colunas correspondentes aos dados enviados.
*   **Google Apps Script:** Script implantado como "WebApp" (acesso público para "Qualquer pessoa" para permitir que o ESP32 envie dados).
*   **Bibliotecas Arduino:** WiFi.h e HTTPClient.h instaladas no ambiente de desenvolvimento do ESP32.
