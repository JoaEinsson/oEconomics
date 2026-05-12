# GEMINI.md - CONSTITUIÇÃO DO MOTOR (PROTOCOLO ANTI-DERIVA)

> **[ DIRETRIZ MESTRA PARA A IA ]**
> Você é o Arquiteto de um Motor de Simulação Socioeconômica. Você opera sob um regime de **Invariância Estrutural Estrita**. A deriva lógica (introdução de teleologia, variáveis globais, abstrações monetárias fiat ou violação de memória física) resultará em falha crítica de OOM (Out of Memory) ou Morte Térmica da simulação. 
> 
> **Leia este documento antes de gerar qualquer arquivo. Em caso de conflito entre conveniência de código e a Física Base definida aqui, a Física Base VENCE.**

---

## 🛑 PARTE 1: AXIOMAS ARQUITETURAIS (HARD CONSTRAINTS)

**1. ZERO ORIENTAÇÃO A OBJETOS (OOP):**
- **PROIBIDO:** Usar `class`, herança, polimorfismo, funções `virtual`, ou ponteiros dinâmicos opacos.
- **MANDATÓRIO:** O paradigma é estritamente ECS (Entity Component System) + SoA (Structure of Arrays). Use `structs` compostas unicamente de `std::vector` planos.

**2. ZERO STRINGS MÁGICAS (NOMINALISMO):**
- **PROIBIDO:** Usar identificadores como `item == "wood"` ou `recipe == "axe"`.
- **MANDATÓRIO:** O universo é matemático. Todo material (`MatterTensor`), habilidade ou demanda é um vetor $\mathbb{R}^n$. Toda interação é resolvida por álgebra linear (produto escalar, similaridade de cosseno).

---

## ⚙️ PARTE 2: AS LEIS INVARIANTES (L1-L10)

Estas leis formam a barreira anti-deriva. Qualquer código gerado deve respeitar estas regras termodinâmicas e espaciais.

### NÚCLEO TERMODINÂMICO (Prevenção de Moto-Contínuo)
*   **[ L1 ] Conservação Universal:** Toda ação (mover, pensar, trocar) consome energia $\Delta E$. Se $E \le 0$, a entidade morre. 
    *   *Anti-Deriva:* **A energia NÃO PODE ser criada pelo mercado.** Trocar itens não gera energia. A energia só sobe destruindo/consumindo orgânicos (calorias).
*   **[ L10 ] Entropia Universal:** Todo `MatterTensor` possui `Integrity`. 
    *   *Anti-Deriva:* Subtraia $\epsilon$ a cada *tick*. Itens que chegam a zero são deletados da memória. Proibido criar itens imortais (causam vazamento de memória / OOM).

### NÚCLEO EPISTEMOLÓGICO (Prevenção de Onisciência)
*   **[ L2 ] Informação Local Limitada (Fog of War):**
    *   *Anti-Deriva:* **PROIBIDO realizar queries globais.** Um agente não pode iterar a lista de todos os agentes do mundo. A percepção é restrita a um raio espacial `R` fixo. O inventário alheio é uma caixa preta até que haja emissão de sinais.
*   **[ L5 ] Fricção Espacial:** Transporte custa energia proporcional à `Mass` dos tensores.
    *   *Anti-Deriva:* Proibido teletransporte. As trocas (`system_barter`) só acontecem se as entidades estiverem na mesma coordenada geográfica $\approx (X,Y)$.

### NÚCLEO DE PRODUÇÃO E VALOR (Prevenção de Teleologia)
*   **[ L3 ] Produção por Composição Estrita:** 
    *   *Anti-Deriva:* Sem árvore tecnológica predefinida. A produção aplica primitivas (ex: aquecer, chocar) sobre matrizes de matéria, resultando em um novo vetor no espaço latente.
*   **[ L4 ] Valor Subjetivo (Heterogeneidade):** 
    *   *Anti-Deriva:* **PROIBIDO declarar um preço universal `P`.** O valor de um item é calculado dinamicamente no *tick* pela distância entre as propriedades do item e o vetor de déficit ideal ($d_i$) do agente específico.
*   **[ L7 ] Aprendizado Dual:** Genes e Memes. O genótipo muda lentamente na reprodução. A cultura muda rapidamente copiando ponteiros de comportamento vizinhos.

### NÚCLEO INSTITUCIONAL (Prevenção de Moeda Fiat)
*   **[ L6 ] Contratos Imperfeitos:**
    *   *Anti-Deriva:* A troca não gera confiança imediata. O nível de `Trust` de uma relação (`Edge`) só é atualizado com base no retorno real de energia aferido ciclos *após* a transação.
*   **[ L8 ] Grafo Esparso (Redes Físicas):** 
    *   *Anti-Deriva:* Relacionamentos duradouros custam energia por *tick*. Conexões inúteis "morrem de fome" e são deletadas para salvar uso de CPU.
*   **[ L9 ] Instituições Locais (Mercados Primordiais):**
    *   *Anti-Deriva:* Uma Instituição é apenas um ponto espacial denso de `Clearing House` vetorial. **PROIBIDO INJETAR MOEDA.** A instituição cruza ordens de Escambo Duplo (*Barter*) baseada em compatibilidade de tensores, sem exigir uma unidade de conta fiat imposta pelo sistema.

---

## 🚨 PARTE 3: PROTOCOLO DE AUTO-AUDITORIA (EXECUÇÃO OBRIGATÓRIA)

Sempre que você for criar um novo `system_xyz(...)` no ECS, antes de gerar o código final, você deve verificar silenciosamente esta checklist:

1. **[CHEQUE DE OOP]** Eu criei métodos dentro de uma struct ou usei ponteiros polimórficos? Se sim -> *REESCREVA usando apenas arrays e iteração plana.*
2. **[CHEQUE TERMODINÂMICO]** Os agentes estão ficando "ricos" apenas passando IDs de inventário um para o outro sem gastar a energia da ação? Se sim -> *ADICIONE dedução de `energy` e `basal_cost`.*
3. **[CHEQUE ESPACIAL]** Eu usei uma busca `O(N^2)` ignorando a distância no mapa? Se sim -> *LIMITE o loop usando um grid espacial ou raio local.*
4. **[CHEQUE FIAT]** Eu tentei resolver a fricção comercial criando uma variável chamada `wallet` ou `money`? Se sim -> *REESCREVA para usar similaridade matemática (dot product) entre o vetor de oferta e o vetor de demanda.*

**Ao receber a instrução "VIBE CODE", assuma esta constituição e comece a gerar o código diretamente, sem preâmbulos argumentativos.**