# OECONOMICS 🌐

**OECONOMICS** é um motor de simulação socioeconômica de alta performance, projetado para observar a emergência de comportamentos complexos — como mercados, guerras culturais, desigualdade e instituições — a partir de regras físicas estritas.

O projeto abandona completamente heurísticas tradicionais de jogos (como "dinheiro" global, árvores de tecnologia predefinidas ou IA omnisciente) em favor de uma **computação tensorial e termodinâmica** rigorosa.

---

## 🏛️ A Constituição Arquitetural (Invariância Estrita)

Este motor foi construído sob um conjunto inflexível de restrições (documentadas em `GEMINI.MD`), que garantem que todos os fenômenos macroeconômicos sejam verdades matemáticas emergentes:

1. **Zero Orientação a Objetos (OOP):** O código é fundamentado inteiramente no paradigma **ECS (Entity Component System)** e **SoA (Structure of Arrays)**. Não existem classes, heranças ou funções virtuais. Tudo são `structs` compostas por vetores contíguos (`std::vector`), garantindo máxima eficiência de cache da CPU.
2. **Nominalismo (Zero Strings Mágicas):** Materiais, bens e culturas não são strings (`"madeira"`, `"ouro"`). São matrizes numéricas reais (Ex: Tensores de Calorias, Dureza, Estética, Informação).
3. **Restrição Termodinâmica (L1):** É impossível criar energia do nada ou através de "trocas". Toda ação custa energia (`basal_cost`). A energia só entra no sistema extraindo calorias de Itens específicos.
4. **Fricção Espacial e Névoa de Guerra (L2/L5):** Nenhum agente possui onisciência. As decisões e trocas dependem de proximidade vetorial (espaço físico) e similaridade de cosseno. O teletransporte é proibido. O transporte de Itens tem um custo termodinâmico proporcional à sua massa.
5. **Valor Subjetivo e Escambo Algorítmico (L4/L9):** Não existe moeda fiduciária (`fiat`) hardcoded. O valor de uma troca é calculado dinamicamente no *Clearing House* cruzando o vetor de déficit do agente ($d_i$) com a matriz física do item ofertado. 

---

## ⚙️ Tecnologias Utilizadas

- **C++20:** Núcleo de simulação lógica puramente orientada a dados.
- **Raylib:** Motor gráfico responsável por ler o estado descarregado pelo ECS e renderizar a geografia bidimensional, entidades e interações a 60 FPS (Desacoplado da thread da simulação).
- **Multithreading Nativo:** Separação estrita em *Double Buffering* entre o cálculo do mundo (`sim_thread`) e a renderização (`ui_thread`).
- **Python (Telemetry):** Script de análise autônoma (`analyze.py`) capaz de extrair logs brutos da simulação (`telemetry.csv`) e traduzi-los em relatórios gerenciais, medindo o Índice de Gini, mortalidade, evolução genética e comportamental.

---

## 🚀 Como Compilar e Rodar

O projeto utiliza **CMake** para gerenciar o processo de build e resolver dependências automaticamente (como o Raylib).

### Pré-requisitos
- C++20 Compiler (MinGW/GCC, Clang ou MSVC)
- CMake (>= 3.20)

### Passos
1. Clone este repositório.
2. Na raiz do projeto, gere os arquivos de build:
   ```bash
   cmake -B build
   ```
3. Compile o executável:
   ```bash
   cmake --build build --config Release
   ```
4. Execute a simulação:
   - No Windows: `.\build\Release\oeconomics.exe` (ou apenas `.\build\oeconomics.exe` no MinGW)
   - No Linux/Mac: `./build/oeconomics`

### Analisando os Dados
Após rodar a simulação por alguns *ticks*, ela gerará um arquivo de telemetria. Para visualizar as métricas macroeconômicas:
```bash
python analyze.py
```

---

## 🗺️ Roadmap de Evolução

A simulação é uma tela em branco pronta para complexidade adicional. Consulte o arquivo `futuras_implementações.md` para visualizar as próximas mecânicas focadas na emergência de **Bens de Capital, Clima, Doenças e Tribalismo Estigmergico**.

---
*Construído com regras frias para observar comportamentos quentes.*
