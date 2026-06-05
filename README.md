# Projeto de Monitoramento IoT: ESP32 e Google Sheets

## Visão Geral
Este projeto implementa um sistema de aquisição de dados via Internet das Coisas (IoT). O microcontrolador (ESP32) coleta informações de sensores e transmite os dados para uma planilha do Google via protocolo HTTP. O sistema é composto por uma API de backend, uma interface de visualização e o firmware do hardware.

## Estrutura do Repositório
- `Código.gs`: Backend e API (Google Apps Script).
- `index.html`: Interface de usuário (Dashboard).
- `esp.c++`: Código fonte do microcontrolador (ESP32).

## Detalhamento dos Componentes

### 1. Código.gs (Google Apps Script)
Atua como o servidor de API.
- **Função `doPost(e)`:** Recebe requisições HTTP POST vindas do ESP32, interpreta o payload (dados dos sensores) e grava uma nova linha na Planilha Google.
- **Função `doGet(e)`:** Gerencia requisições GET para servir a página `index.html` ou fornecer dados estruturados (JSON) para a interface.

### 2. index.html
Interface Web (Client-side).
- Renderiza o painel de controle e visualização dos dados.
- Utiliza JavaScript para realizar chamadas assíncronas ao `Código.gs` (via `google.script.run`) ou requisições Fetch para obter dados atualizados da planilha.

### 3. esp.ino (Firmware)
Código embarcado para o hardware ESP32.
- **Conectividade:** Gerencia a conexão Wi-Fi.
- **Leitura de Sensores:** Executa a leitura das portas analógicas/digitais.
- **Transmissão:** Utiliza a biblioteca `HTTPClient.h` para realizar envios periódicos de dados via requisições POST para a URL do Google Apps Script.

## Fluxo de Dados
1. **Coleta:** O ESP32 lê os valores dos sensores.
2. **Envio:** O ESP32 envia os dados via HTTP POST para o endpoint do Script.
3. **Armazenamento:** O `Código.gs` recebe os dados e os registra na Planilha Google vinculada.
4. **Visualização:** O `index.html` exibe as informações processadas ao usuário.

## Pré-requisitos
- **Google Sheets:** Planilha configurada com as colunas correspondentes aos dados enviados.
- **Google Apps Script:** Script implantado como "WebApp" (acesso público para "Qualquer pessoa" para permitir que o ESP32 envie dados).
- **Bibliotecas Arduino:** `WiFi.h` e `HTTPClient.h` instaladas no ambiente de desenvolvimento do ESP32.
