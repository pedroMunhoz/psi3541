# Projeto PSI3541 - Sistemas Embarcados Distribuídos

Este projeto tem como objetivo o **controle de um carrinho robótico** utilizando tanto uma **interface web** quanto comandos via **broker MQTT**. O sistema foi desenvolvido para a disciplina PSI3541 - Sistemas Embarcados Distribuídos, com foco em modularidade, comunicação eficiente e integração com sistemas modernos de IoT.

---

## Funcionalidades Principais

- **Controle do Carrinho:**  
  O carrinho pode ser comandado para andar para frente, para trás, girar à esquerda, à direita, rotacionar em ambos os sentidos ou parar, tanto pela interface web quanto por comandos MQTT.

- **Configuração dos Parâmetros de Controle:**  
  É possível definir e ajustar os parâmetros do controlador (Kp, Ki, Kd, Kp_total) em tempo real, tanto pela interface web quanto via MQTT. Isso permite tunar o comportamento do carrinho conforme a necessidade.

- **Consulta de Status e Configuração:**  
  O usuário pode consultar o status atual do carrinho (posição, referência, estado, finalizado ou não) e também obter as configurações atuais do controlador, por ambos os meios (web e MQTT).

- **Interface Web Responsiva:**  
  A interface web é salva na flash do dispositivo, com arquivos HTML, JS e CSS localizados na pasta `/data`. Ela é servida diretamente pelo ESP32, permitindo controle e monitoramento em tempo real via navegador.

- **API REST:**  
  A comunicação entre a interface web e o carrinho é feita por meio de uma API REST, que expõe endpoints para comandos de movimento, configuração e consulta de status.

- **Integração MQTT:**  
  O sistema publica periodicamente (a cada 500ms) o status e as configurações do carrinho em tópicos MQTT específicos, permitindo integração com sistemas externos, dashboards ou automações.

- **Modularidade:**  
  O código é organizado em módulos independentes (carro, servidor web, MQTT, mensageiro, etc). Cada módulo se comunica com os demais por meio de um **mensageiro** (Messenger), que garante desacoplamento e facilita a manutenção e expansão do sistema.

---

## Estrutura do Projeto

- **`include/`**: Arquivos de cabeçalho (`.h`) com definições e declarações dos módulos.
- **`src/`**: Código-fonte principal dos módulos (`.c`).
- **`data/`**: Arquivos da interface web (HTML, JS, CSS) que são salvos na flash do ESP32 e servidos ao usuário.
- **`cert/`**: Certificados necessários para conexão segura com a AWS IoT (coloque aqui os arquivos fornecidos pela AWS).
- **`partitions.csv`**: Define o particionamento da flash, separando espaço para o firmware, sistema de arquivos (SPIFFS) e outros dados, garantindo que a página web e o código possam coexistir na memória do dispositivo.

---

## Como Funciona

- **Interface Web:**  
  Ao acessar o endereço IP do ESP32 pelo navegador, a interface web é carregada diretamente da flash. Por ela, é possível enviar comandos, ajustar parâmetros e visualizar o status do carrinho em tempo real.

- **API REST:**  
  A interface web se comunica com o carrinho via endpoints HTTP, como `/carMove`, `/carConfig`, `/carStatus`, etc.

- **MQTT:**  
  O carrinho publica periodicamente seu status em `/carStatus` e suas configurações em `/carConfig`. Também é possível enviar comandos e configurações via MQTT, recebendo respostas em tópicos específicos.

- **Mensageiro:**  
  Todos os módulos (carro, servidor, MQTT, etc) trocam mensagens por meio do Messenger, um sistema de filas e handlers que garante comunicação desacoplada e eficiente.

---

## Compilação e Execução

1. **Clone o repositório:**
   ```bash
   git clone https://github.com/seu-usuario/psi3541.git
   cd psi3541
   ```

2. **Configure o ambiente ESP-IDF:**
   ```bash
   . $IDF_PATH/export.sh
   ```

3. **Configure o projeto:**
   ```bash
   idf.py menuconfig
   ```
   - Configure o WiFi (SSID e senha) em "PSI3541 Configurations".

4. **Coloque os certificados da AWS em `/cert` na raiz do projeto.**

5. **Compile, faça upload do sistema de arquivos e do firmware:**
   ```bash
   idf.py build
   idf.py spiffs_flash
   idf.py flash
   ```

6. **Monitore a saída serial:**
   ```bash
   idf.py monitor
   ```

---

## Acessando a Interface Web

Após o ESP32 conectar-se à rede Wi-Fi, o endereço IP será exibido no monitor serial.  
Acesse esse endereço no navegador para visualizar e controlar o carrinho.

---

## Observações

- O arquivo `partitions.csv` define o particionamento da flash, garantindo espaço para o firmware e para o sistema de arquivos onde a interface web é armazenada.
- Os certificados da AWS são obrigatórios para a comunicação segura via MQTT.
- O sistema é extensível: novos comandos, sensores ou integrações podem ser adicionados facilmente graças à arquitetura modular baseada em mensagens.

---

**Desenvolvido para a disciplina PSI3541 - Sistemas Embarcados