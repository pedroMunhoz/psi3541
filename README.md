# Projeto PSI3541 - Sistemas Embarcados Distribuídos

Este repositório contém o projeto desenvolvido para a disciplina **PSI3541 - Sistemas Embarcados Distribuídos**. O objetivo é implementar funcionalidades específicas em sistemas embarcados, com versionamento do código e avanço conforme as atividades e o projeto proposto.

## Estrutura do Projeto

Os arquivos do projeto estão organizados da seguinte forma:

- **`include/`**: Contém os arquivos de cabeçalho (`.h`) com as definições e declarações utilizadas no projeto.
- **`src/`**: Contém os arquivos de código-fonte (`.c`) com a implementação das funcionalidades.

## Funcionalidades

O projeto implementa as seguintes funcionalidades:

1. **Controle e leitura de GPIO**: Manipulação de pinos de entrada e saída do microcontrolador.
2. **Leitura do sensor DHT11**: Coleta de dados de temperatura e umidade.
3. **Servidor Web com API REST**: Permite a interação com o sistema embarcado via HTTP.
4. **Sistema de Arquivos**: Armazena os arquivos necessários para o funcionamento da interface web.

## Compilação

Para compilar o projeto, siga os passos abaixo:

1. Clone o repositório:
   ```bash
   git clone https://github.com/seu-usuario/psi3541.git
   cd psi3541
2. Configure o ambiente ESP-IDF:
    ```bash
    . $IDF_PATH/export.sh
3. Configure o projeto:
    ```bash
    idf.py menuconfig
    ```
    - Configure o WiFi (SSID e senha) em "PSI3541 Configurations"
4. Compile o projeto, faça o upload do sistema de arquivos e do firmware
    ```bash
    idf.py build
    idf.py spiffs_flash
    idf.py flash
5. Monitore sua saída
    ```bash
    idf.py monitor

## Acessando a interface web

Após o ESP32 conectar-se à rede Wi-Fi, o endereço IP será exibido no monitor serial. Acesse o endereço IP no navegador para visualizar a interface web.